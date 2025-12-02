#include "kafkaclient.hpp"
#include "text_processor.hpp"
#include "constants.hpp"

KafkaClient::KafkaClient(const KafkaConfig& config) : config_(config), initialized_(false) {
    
}

void KafkaClient::Initialize(std::function<void(const moderation::ModerateObjectResponse&, int64_t)> result_callback)
{
    if (initialized_)
    {
        return; 
    }

    try{
        producer_ = std::make_unique<KafkaProducer>(config_);
        consumer_ = std::make_unique<KafkaConsumer>(config_, result_callback, producer_.get());
        initialized_ = true;
        std::cout << "KafkaClient initialized successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize KafkaClient: " << e.what() << std::endl;
        throw;
    }
}

bool KafkaClient::SendRequestAsync(const moderation::ModerateObjectRequest& request)
{
    if(!initialized_ || !producer_)
    {
        std::cerr << "KafkaClient not initialized." << std::endl;
        return false;
    }
    return producer_->SendRequestAsync(request, config_.request_topic);
}

bool KafkaClient::SendResponseAsync(const moderation::ModerateObjectResponse& response, int64_t requestId)
{
    if(!initialized_ || !producer_)
    {
        std::cerr << "KafkaClient not initialized." << std::endl;
        return false;
    }

    std::string key = std::to_string(requestId);
    return producer_->SendResponseAsync(response, config_.result_topic, key);
}

void ProducerDeliveryReportCb::dr_cb(RdKafka::Message &message) {
    if(message.err() == RdKafka::ERR_NO_ERROR) {
        std::cout << "Message delivered to topic " << message.topic_name()
        << ", partition " << message.partition() 
        << ", offset " << message.offset() << std::endl;
    }
    else {
        std::cerr << "Message delivery failed: " << message.errstr() << std::endl;
    }
}

KafkaProducer::KafkaProducer(const KafkaConfig& config) {
    std::string errorString;

    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

    dr_cb_ = std::make_unique<ProducerDeliveryReportCb>();

    if(conf->set("bootstrap.servers", config.brokers, errorString) != RdKafka::Conf::CONF_OK)
    {
        throw std::runtime_error("Failed to set Kafka brokers: " + errorString);
    }

    conf->set("client.id", "moderation_service_producer", errorString);
    conf->set("acks", "all", errorString);
    conf->set("retries", std::to_string(config.max_retries), errorString);
    conf->set("retry.backoff.ms", std::to_string(config.retry_backoff_ms), errorString);
    conf->set("dr_cb", dr_cb_.get(), errorString);
    conf->set("compression.codec", "none", errorString);

    producer_.reset(RdKafka::Producer::create(conf, errorString));
    if(!producer_)
    {
        delete conf;
        throw std::runtime_error("Failed to create Kafka producer: " + errorString);
    }

    delete conf;

   topic_name_ = config.request_topic;
}

bool KafkaProducer::SendRequestAsync(const moderation::ModerateObjectRequest& request, const std::string& topic) {
    std::lock_guard<std::mutex> lock(mutex_);
    if(!producer_ || topic_name_.empty())
    {
        std::cerr << "KafkaProducer not properly initialized." << std::endl;
        return false;
    }

    std::string target_topic = topic.empty() ? topic_name_ : topic;
    if(target_topic.empty())
    {
        std::cerr << "KafkaProducer: No topic specified." << std::endl;
        return false;
    }

    std::string serializedRequest;
    if(!request.SerializeToString(&serializedRequest))
    {
        std::cerr << "Failed to serialize ModerateObjectRequest" << std::endl;
        return false;
    }

    std::string key = std::to_string(request.id());

    RdKafka::ErrorCode error = producer_->produce(
        target_topic,
        -1,
        RdKafka::Producer::RK_MSG_COPY,
        const_cast<char*>(serializedRequest.c_str()),
        serializedRequest.size(),
        key.c_str(),
        key.size(),
        0,
        nullptr
    );

    if(error != RdKafka::ERR_NO_ERROR)
    {
        std::cerr << "Failed to produce message: " << RdKafka::err2str(error) << std::endl;
        return false;
    }

    producer_->poll(0);

    return true;
}

bool KafkaProducer::SendResponseAsync(const moderation::ModerateObjectResponse& response, const std::string& topic, const std::string& key)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if(!producer_)
    {
        std::cerr << "KafkaProducer not properly initialized." << std::endl;
        return false;
    }

    if(topic.empty())
    {
        std::cerr << "KafkaProducer: Topic cannot be empty for SendResponseAsync" << std::endl;
        return false;
    }

    std::string serializedResponse;
    if(!response.SerializeToString(&serializedResponse))
    {
        std::cerr << "Failed to serialize ModerateObjectResponse" << std::endl;
        return false;
    }

    RdKafka::ErrorCode error = producer_->produce(
        topic,
        -1,
        RdKafka::Producer::RK_MSG_COPY,
        const_cast<char*>(serializedResponse.c_str()),
        serializedResponse.size(),
        key.c_str(),
        key.size(),
        0,
        nullptr
    );

    if(error != RdKafka::ERR_NO_ERROR)
    {
        std::cerr << "Failed to produce response message: " << RdKafka::err2str(error) << std::endl;
        return false;
    }

    producer_->poll(0);

    return true;

}

bool KafkaProducer::Flush(int timeoutMs) {

    if(!producer_)
    {
        std::cerr << "KafkaProducer not properly initialized." << std::endl;
        return false;
    }

    RdKafka::ErrorCode error = producer_->flush(timeoutMs);

    if(error != RdKafka::ERR_NO_ERROR)
    {
        std::cerr << "Failed to flush Kafka producer: " << RdKafka::err2str(error) << std::endl;
        return false;
    }

    return true;
}

KafkaProducer::~KafkaProducer() {
    if(producer_)
    {
        producer_->flush(10000);
    }
}

void ConsumerEventCb::event_cb(RdKafka::Event &event) {
    switch(event.type()) {
        case RdKafka::Event::EVENT_ERROR:
            if(event.fatal()) {
                std::cerr << "Fatal Kafka error: " << event.str() << std::endl;
                // Fatal errors mean consumer must be recreated
            }
            else {
                std::cerr << "Kafka error: " << event.str() << std::endl;
                // Non-fatal errors can be recovered
            }
            break;
        case RdKafka::Event::EVENT_LOG:
            std::cout << "Kafka log: " << event.str() << std::endl;
            break;
        default:
            break;
    }
}

void KafkaConsumer::ProcessMessage(RdKafka::Message* message)
{
    int64_t request_id = 0;
    if(message->key() && message->key_len() > 0)
    {
        const void* key_void = message->key();
        const char* key_ptr = static_cast<const char*>(key_void);

        std::string key_str(
            key_ptr,
            message->key_len()
        );

        try {
            request_id = std::stoll(key_str);
        }
        catch(const std::exception& e) {
            std::cerr << "Failed to parse message key to request ID: " << e.what() << std::endl;
            return;
        }
    }

    const char* payload = static_cast<const char*>(message->payload());
    size_t payload_size = message->len();

    moderation::ModerateObjectRequest request;
    if(!request.ParseFromArray(payload, static_cast<int>(payload_size)))
    {
        std::cerr << "Failed to parse ModerateObjectRequest from message payload" << std::endl;
        return;
    }
    
    std::cout << "Received ModerateObjectRequest for request ID " << request_id <<", text: " << request.text() << std::endl;

    bool isFlagged = false;
    
    try{
        isFlagged = TextProcessor::TextProcessing(request.text());
    } catch(const std::exception& e)
    {
        std::cerr << "Error processing text: " << e.what() << std::endl;
        return;
    }

    moderation::ModerateObjectResponse response;
    response.set_success(isFlagged);

    std::cout << "Text processing result for request ID " << request_id 
              << ": " << (isFlagged ? "FLAGGED" : "PASSED") << std::endl;


    if(producer_)
    {
        std::string response_key = std::to_string(request_id);
        if(!producer_->SendResponseAsync(response, config_.result_topic, response_key))
        {
           std::cerr << "Failed to send moderation response to Kafka for request ID: " << request_id << std::endl;
        }
        else
        {
            std::cout << "Sent moderation response to topic " << config_.result_topic 
                      << " for request ID: " << request_id << std::endl;
        }
    }
    else{
        std::cerr << "Producer not available, cannot send response for request ID: " << request_id << std::endl;
    }
    

    if (callback_)
    {
        callback_(response, request_id);
    }
    
}

KafkaConsumer::KafkaConsumer(const KafkaConfig& config, MessageCallback callback, KafkaProducer* producer) : config_(config), callback_(callback), producer_(producer), running_(false) {
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
    if(!consumer_)
    {
        delete conf;
        throw std::runtime_error("Failed to create Kafka consumer: " + errorString);
    }
    delete conf;

    std::vector<std::string> topics;
    topics.push_back(config.request_topic);

    RdKafka::ErrorCode error = consumer_->subscribe(topics);
    if(error != RdKafka::ERR_NO_ERROR)
    {
        throw std::runtime_error("Failed to subscribe to Kafka topics: " + RdKafka::err2str(error));
    }

    std::cout << "Kafka consumer subscribed to topic: " << config.request_topic << std::endl;

    
    
    running_ = false;
}

void KafkaConsumer::Start()
{
    if(running_.load())
    {
        return; 
    }

    running_.store(true);
    consumer_thread_ = std::thread(&KafkaConsumer::ConsumeLoop, this);
}

void KafkaConsumer::ConsumeLoop()
{
    while(running_.load())
    {
        RdKafka::Message* message = consumer_->consume(1000);
    
        if(message == nullptr)
        {
            continue; 
        }

        if(message->err() == RdKafka::ERR_NO_ERROR)
        {
            ProcessMessage(message);
        }
        else if(message->err() == RdKafka::ERR__PARTITION_EOF)
        {
            std::cout << "Reached end of partition for topic " << message->topic_name() << std::endl;
        }
        else if(message->err() == RdKafka::ERR__TIMED_OUT){}
        else
        {
            std::cerr << "Kafka consumer error: " << message->errstr() << std::endl;
        }
            
        delete message;
    }
    std::cout << "Kafka consumer ended." << std::endl;
}

KafkaConsumer::~KafkaConsumer() {
    Stop();
    if(consumer_)
    {
        consumer_->close();
    }
}

void KafkaConsumer::Stop() {
    if(!running_.load())
    {
        return;
    }
    running_.store(false);

    if(consumer_thread_.joinable())
    {
        consumer_thread_.join();
    }
}

void KafkaClient::StartConsumer()
{
    if(!initialized_ || !consumer_)
    {
        std::cerr << "KafkaClient not initialized." << std::endl;
        return;
    }
    consumer_->Start();
}

void KafkaClient::StopConsumer()
{
    if(!initialized_ || !consumer_)
    {
        std::cerr << "KafkaClient not initialized." << std::endl;
        return;
    }
    consumer_->Stop();
}

void KafkaClient::Shutdown()
{
    StopConsumer();

    if(producer_)
    {
        producer_->Flush(10000);
    }

    consumer_.reset();
    producer_.reset();

    initialized_ = false;
    std::cout << "KafkaClient shutdown completed." << std::endl;
}

void KafkaClient::Flush()
{
    if(!initialized_ || !producer_)
    {
        std::cerr << "KafkaClient not initialized." << std::endl;
        return;
    }
    producer_->Flush(10000);
}

bool KafkaClient::isHealthy() {
    std::lock_guard<std::mutex> lock(mutex_);
    if(!initialized_)
    {
        return false;
    }

    bool producerHealthy = producer_ ? producer_->isHealthy() : false;
    bool consumerHealthy = consumer_ ? consumer_->isRunning() : false;

    return producerHealthy && consumerHealthy;
}

KafkaClient::~KafkaClient() {
    Shutdown();
}