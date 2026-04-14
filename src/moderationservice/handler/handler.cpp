#include "handler/handler.hpp"
#include "interceptors/logger.hpp"
#include <string>
#include <utility>

ModerationHandler::ModerationHandler(std::shared_ptr<ModerationService> service)
    : service_(std::move(service)) {}

grpc::Status ModerationHandler::ModerateObject(grpc::ServerContext* context,
                                               const moderation::ModerateObjectRequest* request,
                                               moderation::ModerateObjectResponse* response) {
    if (request->text().empty() || request->id() == 0) {
        response->set_success(false);
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kGrpc,
                          grpc::StatusCode::INVALID_ARGUMENT,
                          "invalid moderation request: text empty or request id is zero");
        return {grpc::INVALID_ARGUMENT, "Invalid moderation request"};
    }

    int64_t requestId = request->id();
    const std::string& text = request->text();

    bool success = service_->ProcessModerationRequest(requestId, text);

    if (!success) {
        response->set_success(false);
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kGrpc, grpc::StatusCode::INTERNAL,
                          absl::StrCat("failed to process moderation request request_id=", requestId));
        return {grpc::INTERNAL, "Failed to process moderation request"};
    }

    response->set_success(true);
    return grpc::Status::OK;
}
