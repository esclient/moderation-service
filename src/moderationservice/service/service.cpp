#include "service.hpp"

ModerationService::ModerationService() {
    // Initialize mock data
    moderation::Moderation mod1;
    mod1.set_id("1");
    mod1.set_user_id("1");
    mod1.set_object_id("1");
    mod1.set_reason("Inappropriate content");

    moderation::Moderation mod2;
    mod2.set_id("2");
    mod2.set_user_id("2");
    mod2.set_object_id("2");
    mod2.set_reason("Spam");

    moderations_[mod1.id()] = mod1;
    moderations_[mod2.id()] = mod2;

    user_moderations_[mod1.user_id()].push_back(mod1);
    user_moderations_[mod2.user_id()].push_back(mod2);}

}

Status ModerationService::ModerateObject(grpc::ServerContext* context, const moderation::ModerateObjectRequest* request, moderation::ModerateObjectResponse* response)
{
    std::lock_guard<std::mutex> Lock(this->mutex_);

    std::string object_id = request->object_id();
    if(this->moderations_.find(object_id) == this->moderations_.end()) {
        return Status(grpc::StatusCode::NOT_FOUND, "Not found");
    }

    moderation::Moderation moderation = this->moderations_[object_id];
    response->mutable_moderation().CopyFrom(moderation);

    return Status::OK;
}
