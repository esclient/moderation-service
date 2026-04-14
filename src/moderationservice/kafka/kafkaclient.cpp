#include "kafka/kafkaclient.hpp"
#include "config/timeouts.hpp"
#include "model/constants.hpp"
#include "model/model_utils.hpp"
#include "service/text_processor.hpp"
#include "interceptors/logger.hpp"
#include <memory>
#include <string>
#include <utility>
#include <vector>

KafkaClient::KafkaClient(KafkaConfig config) : config_(std::move(config)) {}
void KafkaClient::Initialize(std::function<void(const moderation::ModerateObjectResponse&, int64_t,
                                                const std::string&, moderation::ObjectType)>
                                 result_callback) {
    if (initialized_) {
        return;
    }

    try {
        producer_ = std::make_unique<KafkaProducer>(config_);
        consumer_ =
            std::make_unique<KafkaConsumer>(config_, std::move(result_callback), producer_.get());
        initialized_ = true;
        SERVICE_LOG_INFO("kafka producer and consumer initialized");
    } catch (const std::exception& e) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "INITIALIZATION_FAIL", e.what());
        throw;
    }
}

bool KafkaClient::SendRequestAsync(const moderation::ModerateObjectRequest& request) {
    if (!initialized_ || !producer_) {
        SERVICE_LOG_ERROR(
            moderation::logging::Subsystem::kKafka,
            "NOT_INITIALIZED",
            "KafkaClient not initialized at requesting time"
        );
        return false;
    }
    return producer_->SendRequestAsync(request, config_.request_topic);
}

void ProducerDeliveryReportCb::dr_cb(RdKafka::Message& message) {
    if (message.err() == RdKafka::ERR_NO_ERROR) {
        SERVICE_VLOG1(absl::StrCat(
            "kafka message delivered",
            " topic=", message.topic_name(),
            " partition=", message.partition(),
            " offset=", message.offset()
        ));
    } else {
        SERVICE_LOG_ERROR(
            moderation::logging::Subsystem::kKafka,
            message.err(),                          
            absl::StrCat("delivery failed"
                        " topic=", message.topic_name(),
                        " partition=", message.partition(),
                        " error=", message.errstr())  
        );
    }
}

KafkaProducer::KafkaProducer(const KafkaConfig& config) : config_(config) {
    std::string errorString;

    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

    dr_cb_ = std::make_unique<ProducerDeliveryReportCb>();

    if (conf->set("bootstrap.servers", config.brokers, errorString) != RdKafka::Conf::CONF_OK) {
        throw std::runtime_error("Failed to set Kafka brokers: " + errorString);
    }

    conf->set("client.id", "moderation_service_producer", errorString);
    conf->set("acks", "all", errorString);
    conf->set("retries", std::to_string(config.max_retries), errorString);
    conf->set("retry.backoff.ms", std::to_string(config.retry_backoff_ms), errorString);
    conf->set("dr_cb", dr_cb_.get(), errorString);
    conf->set("compression.codec", "none", errorString);

    producer_.reset(RdKafka::Producer::create(conf, errorString));
    if (!producer_) {
        delete conf;
        throw std::runtime_error("Failed to create Kafka producer: " + errorString);
    }

    delete conf;

    topic_name_ = config.request_topic;
}

bool KafkaProducer::SendRequestAsync(const moderation::ModerateObjectRequest& request,
                                     const std::string& topic) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!producer_ || topic_name_.empty()) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "PRODUCER_NOT_INITIALIZED",
                          "producer not properly initialized");
        return false;
    }

    std::string target_topic = topic.empty() ? topic_name_ : topic;
    if (target_topic.empty()) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "TOPIC_EMPTY",
                          "no request topic specified");
        return false;
    }

    std::string serializedRequest;
    if (!request.SerializeToString(&serializedRequest)) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "SERIALIZE_REQUEST_FAIL",
                          "failed to serialize ModerateObjectRequest");
        return false;
    }

    std::string key = std::to_string(request.id());

    RdKafka::ErrorCode error =
        producer_->produce(target_topic, -1, RdKafka::Producer::RK_MSG_COPY,
                           const_cast<char*>(serializedRequest.c_str()), serializedRequest.size(),
                           key.c_str(), key.size(), 0, nullptr);

    if (error != RdKafka::ERR_NO_ERROR) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, error,
                          absl::StrCat("failed to produce request message topic=", target_topic));
        return false;
    }

    producer_->poll(0);
    SERVICE_VLOG1(absl::StrCat("kafka request produced topic=", target_topic, " key=", key));

    return true;
}

bool KafkaProducer::SendResponseAsync(const moderation::ModerateObjectResponse& response,
                                      const std::string& topic, const std::string& key) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (!producer_) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "PRODUCER_NOT_INITIALIZED",
                          "producer not properly initialized");
        return false;
    }

    if (topic.empty()) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "TOPIC_EMPTY",
                          "response topic cannot be empty");
        return false;
    }

    std::string serializedResponse;
    if (!response.SerializeToString(&serializedResponse)) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "SERIALIZE_RESPONSE_FAIL",
                          "failed to serialize ModerateObjectResponse");
        return false;
    }

    RdKafka::ErrorCode error = producer_->produce(
        topic, -1, RdKafka::Producer::RK_MSG_COPY, const_cast<char*>(serializedResponse.c_str()),
        serializedResponse.size(), key.c_str(), key.size(), 0, nullptr);

    if (error != RdKafka::ERR_NO_ERROR) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, error,
                          absl::StrCat("failed to produce response topic=", topic, " key=", key));
        return false;
    }

    producer_->poll(0);
    SERVICE_VLOG1(absl::StrCat("kafka response produced topic=", topic, " key=", key));

    return true;
}

bool KafkaProducer::Flush(int timeoutMs) {
    if (!producer_) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "PRODUCER_NOT_INITIALIZED",
                          "producer not properly initialized");
        return false;
    }

    RdKafka::ErrorCode error = producer_->flush(timeoutMs);

    if (error != RdKafka::ERR_NO_ERROR) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, error,
                          "failed to flush kafka producer");
        return false;
    }

    return true;
}

KafkaProducer::~KafkaProducer() {
    if (producer_) {
        producer_->flush(moderation::config::KAFKA_FLUSH_TIMEOUT_MS);
    }
}

void ConsumerEventCb::event_cb(RdKafka::Event& event) {
    switch (event.type()) {
    case RdKafka::Event::EVENT_ERROR:
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, event.err(),
                          absl::StrCat(event.fatal() ? "fatal kafka event: " : "kafka event error: ",
                                       event.str()));
        break;
    case RdKafka::Event::EVENT_LOG:
        SERVICE_VLOG1(absl::StrCat("kafka event log: ", event.str()));
        break;
    default:
        break;
    }
}

void KafkaConsumer::ProcessMessage(RdKafka::Message* message) {
    int64_t request_id = 0;
    if (message->key() != nullptr && message->key_len() > 0) {
        const void* key_void = message->key();
        const char* key_ptr = static_cast<const char*>(key_void);

        std::string key_str(key_ptr, message->key_len());

        // Validate that key_str contains only digits (and optional leading +/-)
        if (key_str.empty() ||
            (!std::isdigit(key_str[0]) && key_str[0] != '-' && key_str[0] != '+')) {
            SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "INVALID_KEY_FORMAT",
                              absl::StrCat("message key is not numeric key=", key_str));
            return;
        }

        try {
            request_id = std::stoll(key_str);
        } catch (const std::exception& e) {
            SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "PARSE_KEY_FAIL",
                              absl::StrCat("failed to parse key=", key_str, " error=", e.what()));
            return;
        }
    }

    const char* payload = static_cast<const char*>(message->payload());
    size_t payload_size = message->len();

    moderation::ModerateObjectRequest request;
    if (!request.ParseFromArray(payload, static_cast<int>(payload_size))) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "PARSE_REQUEST_FAIL",
                          "failed to parse ModerateObjectRequest from payload");
        return;
    }

    moderation::ObjectType object_type = request.type();

    if (!moderation::ObjectType_IsValid(static_cast<int>(object_type))) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "INVALID_OBJECT_TYPE",
                          absl::StrCat("invalid object type request_id=", request_id));
        return;
    }

    if (object_type == moderation::OBJECT_TYPE_UNSPECIFIED) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "UNSPECIFIED_OBJECT_TYPE",
                          absl::StrCat("unspecified object type request_id=", request_id));
        return;
    }

    int type_value = static_cast<int>(object_type);

    SERVICE_VLOG1(absl::StrCat("kafka request consumed request_id=", request_id,
                               " object_type=", type_value, " (",
                               moderation::utils::ObjectTypeToString(object_type), ")"));

    bool isFlagged = false;

    try {
        isFlagged = TextProcessor::TextProcessing(request.text());
    } catch (const std::exception& e) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kAlgorithm, "TEXT_PROCESSING_FAIL",
                          e.what());
        return;
    }

    moderation::ModerateObjectResponse response;
    response.set_success(isFlagged);

    SERVICE_VLOG1(absl::StrCat("text processing request_id=", request_id,
                               " result=", (isFlagged ? "FLAGGED" : "PASSED")));

    if (producer_ != nullptr) {
        std::string response_key = std::to_string(request_id);
        if (!producer_->SendResponseAsync(response, config_.result_topic, response_key)) {
            SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "SEND_RESPONSE_FAIL",
                              absl::StrCat("failed to send moderation response request_id=",
                                           request_id));
        } else {
            SERVICE_VLOG1(absl::StrCat("sent moderation response topic=", config_.result_topic,
                                       " request_id=", request_id));
        }
    } else {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "PRODUCER_NOT_AVAILABLE",
                          absl::StrCat("producer unavailable request_id=", request_id));
    }

    if (callback_) {
        callback_(response, request_id, request.text(), object_type);
    }
}

KafkaConsumer::KafkaConsumer(const KafkaConfig& config, MessageCallback callback,
                             KafkaProducer* producer)
    : config_(config), callback_(std::move(callback)), producer_(producer), running_(false) {
    std::string errorString;
    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    event_cb_ = std::make_unique<ConsumerEventCb>();

    conf->set("bootstrap.servers", config.brokers, errorString);
    conf->set("group.id", config.consumer_group_id, errorString);
    conf->set("auto.offset.reset", "earliest", errorString);
    conf->set("enable.auto.commit", "true", errorString);
    conf->set("auto.commit.interval.ms", "1000", errorString);
    conf->set("session.timeout.ms", "30000", errorString);
    conf->set("event_cb", event_cb_.get(), errorString);
    conf->set("heartbeat.interval.ms", "10000", errorString);
    conf->set("max.poll.interval.ms", "300000", errorString);
    conf->set("socket.timeout.ms", "60000", errorString);
    conf->set("request.timeout.ms", "30000", errorString);

    consumer_.reset(RdKafka::KafkaConsumer::create(conf, errorString));
    if (!consumer_) {
        delete conf;
        throw std::runtime_error("Failed to create Kafka consumer: " + errorString);
    }
    delete conf;

    std::vector<std::string> topics;
    topics.push_back(config.request_topic);

    RdKafka::ErrorCode error = consumer_->subscribe(topics);
    if (error != RdKafka::ERR_NO_ERROR) {
        throw std::runtime_error("Failed to subscribe to Kafka topics: " + RdKafka::err2str(error));
    }

    SERVICE_LOG_INFO(absl::StrCat("kafka consumer subscribed topic=", config.request_topic));

    running_ = false;
}

void KafkaConsumer::Start() {
    if (running_.load()) {
        return;
    }

    running_.store(true);
    consumer_thread_ = std::thread(&KafkaConsumer::ConsumeLoop, this);
}

void KafkaConsumer::ConsumeLoop() {
    while (running_.load()) {
        RdKafka::Message* message =
            consumer_->consume(moderation::config::KAFKA_CONSUME_TIMEOUT_MS);

        if (message == nullptr) {
            continue;
        }

        if (message->err() == RdKafka::ERR_NO_ERROR) {
            ProcessMessage(message);
        } else if (message->err() == RdKafka::ERR__PARTITION_EOF) {
            SERVICE_VLOG1(
                absl::StrCat("kafka partition end of topic=", message->topic_name()));
        } else if (message->err() == RdKafka::ERR__TIMED_OUT) {
        } else {
            SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, message->err(),
                              absl::StrCat("kafka consumer error: ", message->errstr()));
        }

        delete message;
    }
    SERVICE_LOG_INFO("kafka consumer stopped");
}

KafkaConsumer::~KafkaConsumer() {
    Stop();
    if (consumer_) {
        consumer_->close();
    }
}

void KafkaConsumer::Stop() {
    if (!running_.load()) {
        return;
    }
    running_.store(false);

    if (consumer_thread_.joinable()) {
        consumer_thread_.join();
    }
}

void KafkaClient::StartConsumer() {
    if (!initialized_ || !consumer_) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "NOT_INITIALIZED",
                          "kafka client not initialized");
        return;
    }
    consumer_->Start();
}

void KafkaClient::StopConsumer() {
    if (!initialized_ || !consumer_) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "NOT_INITIALIZED",
                          "kafka client not initialized");
        return;
    }
    consumer_->Stop();
}

void KafkaClient::Shutdown() {
    StopConsumer();

    if (producer_) {
        producer_->Flush(moderation::config::KAFKA_FLUSH_TIMEOUT_MS);
    }

    consumer_.reset();
    producer_.reset();

    initialized_ = false;
    SERVICE_LOG_INFO("kafka client shutdown completed");
}

void KafkaClient::Flush() {
    if (!initialized_ || !producer_) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "NOT_INITIALIZED",
                          "kafka client not initialized");
        return;
    }
    producer_->Flush(moderation::config::KAFKA_FLUSH_TIMEOUT_MS);
}

bool KafkaClient::isHealthy() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        return false;
    }

    bool producerHealthy = producer_ ? producer_->isHealthy() : false;
    bool consumerHealthy = consumer_ ? consumer_->isRunning() : false;

    return producerHealthy && consumerHealthy;
}

KafkaClient::~KafkaClient() { Shutdown(); }
