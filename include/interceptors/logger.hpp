#pragma once 

#include <grpcpp/grpcpp.h>
#include <grpcpp/support/interceptor.h>
#include <grpcpp/support/server_interceptor.h>

class LoggerInterceptor final : public grpc::experimental::ServerInterceptor {
    public: 
        explicit LoggerInterceptor(grpc::experimental::ServerRpcInfo* info) {
            std::string method = info->method();
            if(method == "unknown")
            {
                std::cout << "[LoggerInterceptor] Unknown method called." << std::endl
            }
            else
            {
                std::cout << "[LoggerInterceptor] Method called: " << method << std::endl;
            }
        }
        void Intercept(grpc::experimental::InterceptorBatchMethods *methods)
        {
            methods->Proceed();
        }
};

class LoggerInterceptorFactory : public grpc::experimental::ServerInterceptorFactoryInterface {
    public:
        grpc::experimental::Interceptor* CreateServerInterceptor(grpc::experimental::ServerRpcInfo* info) {
            return new LoggerInterceptor(info);
        }
};