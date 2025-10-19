#pragma once

#include <string>
#include <memory>
#include <vector>
#include <grpcpp/server_builder.h>
#include <grpcpp/security/server_credentials.h>
#include <grpcpp/impl/service_type.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/support/interceptor.h>
#include "interceptors/logger.hpp"

class Server {
    public: 
        explicit Server(std::string server_address, std::shared_ptr<grpc::Service> service = nullptr, std::string server_name = "Server");

        void Start();
        void Stop();

    private:
        std::unique_ptr<grpc::Server> server_;
        std::shared_ptr<grpc::Service> service_;
        std::string server_address_;
        std::string server_name_;

        // interceptors (middlewares in grpc)
        std::vector<std::unique_ptr<grpc::experimental::ServerInterceptorFactoryInterface>> interceptor_creators_;
};