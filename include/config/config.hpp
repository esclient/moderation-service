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
        
        const char* host_env = std::getenv("HOST");
        if(host_env == nullptr)
        {
            throw std::runtime_error("HOST environment variable not set");
        }
        else{
        config.host = host_env;
        }

        const char* port_env = std::getenv("PORT");
        if(port_env == nullptr)
        {
            throw std::runtime_error("PORT environment variable not set");
        }
        else{
        config.port = port_env;
        }

        const char* database_url_env = std::getenv("DATABASE_URL");
        if(database_url_env == nullptr)
        {
            throw std::runtime_error("DATABASE_URL environment variable not set");
        }
        else{
            config.database_url = database_url_env;
        }

        const char* log_level_env = std::getenv("LOG_LEVEL");
        if(log_level_env == nullptr)
        {
            throw std::runtime_error("LOG_LEVEL environment variable not set");
        }
        else{
            config.log_level = log_level_env;
        }

        const char* log_format_env = std::getenv("LOG_FORMAT");
        if(log_format_env == nullptr)
        {
            throw std::runtime_error("LOG_FORMAT environment variable not set");
        }
        else{
            config.log_format = log_format_env;
        }

        const char* log_datefmt_env = std::getenv("LOG_DATEFMT");
        if(log_datefmt_env == nullptr)
        {
            throw std::runtime_error("LOG_DATEFMT environment variable not set");
        }
        else{
            config.log_datefmt = log_datefmt_env;
        }

        return config;
    }
};