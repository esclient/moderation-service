#pragma once


#include <string>
#include <cstdlib>


struct Config {
    std::string host;
    std::string port;
    
    static Config New() {
        Config config;
        
        const char* host_env = std::getenv("HOST");
        config.host = host_env ? host_env : "0.0.0.0";
        
        const char* port_env = std::getenv("PORT");
        config.port = port_env ? port_env : "7006";
        
        return config;
    }
};