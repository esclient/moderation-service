#include "service.hpp"

ModerationService::ModerationService() {
    // Initialize mock data
    moderation::ModerateObjectRequest mod1;
    mod1.set_id(1);
    mod1.set_text("Inappropriate content");

    moderation::ModerateObjectResponse mod2;
    mod2.set_success(true);

    moderations_[mod1.id()] = mod1;

}

Status ModerationService::ModerateObject(grpc::ServerContext* context, const moderation::ModerateObjectRequest* request, moderation::ModerateObjectResponse* response)
{
    std::lock_guard<std::mutex> Lock(this->mutex_);

    int64_t id = request->id();
    if(this->moderations_.find(id) == this->moderations_.end()) {
        return Status(grpc::StatusCode::NOT_FOUND, "Not found");
    }

    moderation::ModerateObjectRequest moderation = this->moderations_[id];
    response->set_success(true);

    return Status::OK;
}
