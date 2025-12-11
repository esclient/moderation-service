#include "handler.hpp"
#include <iostream>


ModerationHandler::ModerationHandler(std::shared_ptr<ModerationService> service)
    : service_(std::move(service)) {}


grpc::Status ModerationHandler::ModerateObject(
    grpc::ServerContext* context, 
    const moderation::ModerateObjectRequest* request, 
    moderation::ModerateObjectResponse* response)
{
    if(request->text().empty() || request->id() == 0)
    {
        response->set_success(false);
        std::cerr << "[Handler] ERROR: Invalid moderation request. Text is empty or request ID is zero." << std::endl;
        return grpc::Status(grpc::INVALID_ARGUMENT, "Invalid moderation request");
    }

    int64_t requestId = request->id();
    std::string text = request->text();

    bool success = service_->ProcessModerationRequest(requestId, text);

    if(!success)
    {
        response->set_success(false);
        std::cerr << "[Handler] ERROR: Failed to process moderation request." << requestId << std::endl;
        return grpc::Status(grpc::INTERNAL, "Failed to process moderation request");
    } 
    
    response->set_success(true);
    return grpc::Status::OK;
    

}