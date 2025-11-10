#include "kafkaclient.hpp"

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
        consumer_ = std::make_unique<KafkaConsumer>(config_, result_callback);
        initialized_ = true;
        std::cout << "KafkaClient initialized successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize KafkaClient: " << e.what() << std::endl;
        throw;
    }
}

bool KafkaClient::SendRequest(const moderation::ModerateObjectRequest& request)
{
    if(!initialized_ || !producer_)
    {
        std::cerr << "KafkaClient not initialized." << std::endl;
        return false;
    }
    return producer_->SendRequest(request);
}

class ProducerDeliveryReportCb : public RdKafka::DeliveryReportCb {
    public:
        void dr_cb(RdKafka::Message &message) override {
            if(message.err() == RdKafka::ERR_NO_ERROR) {
                std::cout << "Message delivered to topic " << message.topic_name()
                << ", partition " << message.partition() 
                << ", offset " << message.offset() << std::endl;
            }
            else {
                std::cerr << "Message delivery failed: " << message.errstr() << std::endl;
            }
        }
};

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

    RdKafka::Producer* producer = RdKafka::Producer::create(conf, errorString);
    
    if(!producer)
    {
        delete conf;
        throw std::runtime_error("Failed to create Kafka producer: " + errorString);
    }
    
    producer_ = producer;
    delete conf;

    RdKafka::TopicConf* topicConf = RdKafka::TopicConf::create(RdKafka::TopicConf::CONF_TOPIC);

    RdKafka::Topic* topic = RdKafka::Topic::create(
        producer,
        config.request_topic,
        topicConf,
        errorString
    );
    if(!topic)
    {
        delete topicConf;
        throw std::runtime_error("Failed to create Kafka topic: " + errorString);
    }

    topic_ = topic;
    delete topicConf;
}

bool KafkaProducer::SendRequestAsync(const moderation::ModerateObjectRequest& request) {
    
    std::string serializedRequest;
    if(!request.SerializeToString(&serializedRequest))
    {
        std::cerr << "Failed to serialize ModerateObjectRequest" << std::endl;
        return false;
    }

    std::string key = std::to_string(request.id());

    RdKafka::ErrorCode error = producer_->produce(
        topic_.get(),
        RdKafka::Topic::PARTITION_UA,
        RdKafka::Producer::RK_MSG_COPY,
        const_cast<char*>(serializedRequest.c_str()),
        serializedRequest.size(),
        &key[0],
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

bool KafkaProducer::Flush(int timeoutMs) {
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

class ConsumerEventCb : public RdKafka::EventCb {
    public:
        void event_cb(RdKafka::Event &event) override {
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
};

void KafkaConsumer::ProcessMessage(RdKafka::Message* message)
{
    int64_t request_id = 0;
    if(message->key() && message->key_len() > 0)
    {
        std::string key_str(
            static_cast<const char*>(message->key()),
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

    moderation::ModerateObjectResponse response;
    if(!response.ParseFromArray(payload, static_cast<int>(payload_size)))
    {
        std::cerr << "Failed to parse ModerateObjectResponse from message payload" << std::endl;
        return;
    }
    
    std::cout << "Received ModerateObjectResponse for request ID " << request_id
              << ", success: " << response.success() << std::endl;

    if (callback_)
    {
        callback_(response, request_id);
    }
    
}

KafkaConsumer::KafkaConsumer(const KafkaConfig& config, MessageCallback callback) {
    std::string errorString;
    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
    ConsumerEventCb* eventCallback = new ConsumerEventCb();

    conf->set("bootstrap.servers", config.brokers, errorString);
    conf->set("group.id", config.consumer_group_id, errorString);
    conf->set("auto.offset.reset", "earliest", errorString);
    conf->set("enable.auto.commit", "true", errorString);
    conf->set("auto.commit.interval.ms", "1000", errorString);
    conf->set("session.timeout.ms", "30000", errorString);
    conf->set("event_cb", eventCallback, errorString);

    consumer_ = RdKafka::KafkaConsumer::create(conf, errorString);
    if(!consumer_)
    {
        delete conf;
        throw std::runtime_error("Failed to create Kafka consumer: " + errorString);
    }
    delete conf;

    std::vector<std::string> topics;
    topics.push_back(config.result_topic);

    RdKafka::ErrorCode error = consumer_->subscribe(topics);
    if(error != RdKafka::ERR_NO_ERROR)
    {
        throw std::runtime_error("Failed to subscribe to Kafka topics: " + RdKafka::err2str(error));
    }

    std::cout << "Kafka consumer subscribed to topic: " << config.result_topic << std::endl;
    
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

KafkaClient::~KafkaClient() {
    Shutdown();
}