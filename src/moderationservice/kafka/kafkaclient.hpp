#pragma once

#include <string>
#include <memory>
#include <functional>
#include <mutex>
#include <atomic>
#include <thread>
#include <iostream>
#include <vector>
#include <librdkafka/rdkafkacpp.h>
#include "config/config.hpp"
#include "moderation.pb.h"

class KafkaProducer;
class KafkaConsumer;

class ProducerDeliveryReportCb : public RdKafka::DeliveryReportCb {
    public:
        void dr_cb(RdKafka::Message &message) override;
};

class ConsumerEventCb : public RdKafka::EventCb {
    public:
        void event_cb(RdKafka::Event &event) override;
};

class KafkaClient {
    
    public:
        explicit KafkaClient(const KafkaConfig& config);
        ~KafkaClient();

        void Initialize(std::function<void(const moderation::ModerateObjectResponse&, int64_t)> result_callback);
        bool SendRequestAsync(const moderation::ModerateObjectRequest& request);
        void StartConsumer();
        void StopConsumer();
        void Flush();
        bool isHealthy();
        void Shutdown();

    private:
        KafkaConfig config_;
        std::unique_ptr<KafkaProducer> producer_;
        std::unique_ptr<KafkaConsumer> consumer_;
        bool initialized_;
        std::mutex mutex_;
};

class KafkaProducer {
    public:
        explicit KafkaProducer(const KafkaConfig& config);
        ~KafkaProducer();

        bool SendRequestAsync(const moderation::ModerateObjectRequest& request);
        bool Flush(int timeoutMs = 10000);
        bool isHealthy() const { return producer_ != nullptr; }
    
    private:
        KafkaConfig config_;
        std::unique_ptr<RdKafka::Producer> producer_;
        std::string topic_name_;
        std::unique_ptr<ProducerDeliveryReportCb> dr_cb_;
        std::mutex mutex_;
};

class KafkaConsumer {
    public:
        using MessageCallback = std::function<void(const moderation::ModerateObjectResponse&, int64_t)>;

        explicit KafkaConsumer(const KafkaConfig& config, MessageCallback callback);
        ~KafkaConsumer();

        void Start();
        void Stop();
        bool isRunning() const { return running_.load(); }
    
    private:
        void ConsumeLoop();
        void ProcessMessage(RdKafka::Message* message);

        KafkaConfig config_;
        MessageCallback callback_;
        std::unique_ptr<RdKafka::KafkaConsumer> consumer_;
        std::unique_ptr<ConsumerEventCb> event_cb_;
        std::thread consumer_thread_;
        std::atomic<bool> running_;
        std::mutex mutex_;
};

