#include "server.hpp"

Server::Server(std::string server_address, std::shared_ptr<grpc::Service> service, std::string server_name)
    : server_address_(server_address), service_(service), server_name_(server_name) {

        // register interceptors 

        this->interceptor_creators_.emplace_back(std::make_unique<LoggerInterceptorFactory>());

    }

void Server::Start()
{
    // create server builder
    grpc::ServerBuilder builder;

    builder.AddListeningPort(server_address_, grpc::InsecureServerCredentials());

    builder.experimental().SetInterceptorCreators(this->interceptor_creators_);

    builder.RegisterService(this->service_.get());

    this->server_ = builder.BuildAndStart();

    std::cout << this->server_name_ << " listening on " << this->server_address_ << std::endl;

    this->server_->Wait();
}

void Server::Stop()
{
    std::cout << "Shutting down " << this->server_name_ << "..." << std::endl;
    this->server_->Shutdown();
    std::cout << this->server_name_ << " shut down." << std::endl;
}