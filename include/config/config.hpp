#pragma once


#include <string>
#include <cstdlib>


struct Config {
    std::string host;
    std::string port;
    std::string kafka_brokers;
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

        const char* kafka_brokers = std::getenv("KAFKA_BROKERS");
        if(kafka_brokers == nullptr)
        {
            throw std::runtime_error("KAFKA_BROKERS environment variable not set");
        }
        else
        {
            config.kafka_brokers = kafka_brokers;
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
