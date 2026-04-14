#include "server/server.hpp"
#include "interceptors/logger.hpp"
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <memory>
#include <string>
#include <utility>

Server::Server(std::string server_address, std::shared_ptr<grpc::Service> service,
               const std::string& server_name)
    : server_address_(server_address), service_(service), server_name_(server_name) {

    this->interceptor_creators_.emplace_back(std::make_unique<LoggerInterceptorFactory>());
}

void Server::Start() {
    grpc::ServerBuilder builder;

    builder.AddListeningPort(server_address_, grpc::InsecureServerCredentials());

    builder.RegisterService(this->service_.get());

    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    builder.experimental().SetInterceptorCreators(std::move(this->interceptor_creators_));

    this->server_ = builder.BuildAndStart();
    SERVICE_LOG_INFO(
        absl::StrCat(this->server_name_, " listening on ", this->server_address_));
    this->server_->Wait();
}

void Server::Stop() {
    SERVICE_LOG_INFO(absl::StrCat("shutting down ", this->server_name_));
    this->server_->Shutdown();
    SERVICE_LOG_INFO(absl::StrCat(this->server_name_, " shut down"));
}
