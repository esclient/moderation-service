#pragma once


#include <string>
#include <cstdlib>


struct Config {
    std::string host;
    std::string port;
    std::string database_url;
    std::string log_level;
    std::string log_format;
    std::string log_datefmt;
    
    static Config New() {
        Config config;
        
        const char* host = std::getenv("HOST");
        if(host == nullptr)
        {
            throw std::runtime_error("HOST environment variable not set");
        }
        else
        {
            config.host = host;
        }

        const char* port = std::getenv("PORT");
        if(port == nullptr)
        {
            throw std::runtime_error("PORT environment variable not set");
        }
        else
        {
            config.port = port;
        }

        const char* database_url = std::getenv("DATABASE_URL");
        if(database_url == nullptr)
        {
            throw std::runtime_error("DATABASE_URL environment variable not set");
        }
        else
        {
            config.database_url = database_url;
        }

        const char* log_level = std::getenv("LOG_LEVEL");
        if(log_level == nullptr)
        {
            throw std::runtime_error("LOG_LEVEL environment variable not set");
        }
        else
        {
            config.log_level = log_level;
        }

        const char* log_format = std::getenv("LOG_FORMAT");
        if(log_format == nullptr)
        {
            throw std::runtime_error("LOG_FORMAT environment variable not set");
        }
        else
        {
            config.log_format = log_format;
        }

        const char* log_datefmt = std::getenv("LOG_DATEFMT");
        if(log_datefmt == nullptr)
        {
            throw std::runtime_error("LOG_DATEFMT environment variable not set");
        }
        else
        {
            config.log_datefmt = log_datefmt;
        }

        return config;
    }
};

struct KafkaConfig {
    std::string brokers;
    std::string request_topic;
    std::string result_topic;
    std::string consumer_group_id;
    int max_retries;
    int retry_backoff_ms;
    bool enable_ssl;
    
    static KafkaConfig New() {
        KafkaConfig kafkaConfig;
        
        const char* kafka_brokers = std::getenv("KAFKA_BROKERS");
        if(kafka_brokers == nullptr)
        {
            throw std::runtime_error("KAFKA_BROKERS environment variable not set");
        }
        else
        {
            kafkaConfig.brokers = kafka_brokers;
        }
        const char* kafka_request_topic = std::getenv("KAFKA_REQUEST_TOPIC");
        if(kafka_request_topic == nullptr)
        {
            throw std::runtime_error("KAFKA_REQUEST_TOPIC environment variable not set");
        }
        else
        {
            kafkaConfig.request_topic = kafka_request_topic;
        }
        const char* kafka_result_topic = std::getenv("KAFKA_RESULT_TOPIC");
        if(kafka_result_topic == nullptr)
        {
            throw std::runtime_error("KAFKA_RESULT_TOPIC environment variable not set");
        }
        else
        {
            kafkaConfig.result_topic = kafka_result_topic;
        }
        const char* kafka_consumer_group_id = std::getenv("KAFKA_CONSUMER_GROUP_ID");
        if(kafka_consumer_group_id == nullptr)
        {
            throw std::runtime_error("KAFKA_CONSUMER_GROUP_ID environment variable not set");
        }
        else
        {
            kafkaConfig.consumer_group_id = kafka_consumer_group_id;
        }
        const char* kafka_max_retries = std::getenv("KAFKA_MAX_RETRIES");
        if(kafka_max_retries == nullptr)
        {
            throw std::runtime_error("KAFKA_MAX_RETRIES environment variable not set");
        }
        else
        {
            kafkaConfig.max_retries = std::stoi(kafka_max_retries);
        }
        const char* kafka_retry_backoff_ms = std::getenv("KAFKA_RETRY_BACKOFF_MS");
        if(kafka_retry_backoff_ms == nullptr)
        {
            throw std::runtime_error("KAFKA_RETRY_BACKOFF_MS environment variable not set");
        }
        else
        {
            kafkaConfig.retry_backoff_ms = std::stoi(kafka_retry_backoff_ms);
        }
        const char* kafka_enable_ssl = std::getenv("KAFKA_ENABLE_SSL");
        if(kafka_enable_ssl == nullptr)
        {
            throw std::runtime_error("KAFKA_ENABLE_SSL environment variable not set");
        }
        else
        {
            if(std::string(kafka_enable_ssl) == "true" || std::string(kafka_enable_ssl) == "1")
            {
                kafkaConfig.enable_ssl = true;
            }
            else
            {
                kafkaConfig.enable_ssl = false;
            }
        }

        return kafkaConfig;
    }
};
