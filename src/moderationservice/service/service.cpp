#include "service.hpp"
#include "text_processor.hpp"

ModerationService::ModerationService() {
    // Initialize mock data
    moderation::ModerateObjectRequest mod1;
    mod1.set_id(1);
    mod1.set_text("Very b@d w0rd! Please m0d3rate. This is a t3st. You are gay. Super gay! Super super gay!");

    moderation::ModerateObjectResponse mod2;
    mod2.set_success(true);

    moderations_[mod1.id()] = mod1;

}

Status ModerationService::ModerateObject(grpc::ServerContext* context, const moderation::ModerateObjectRequest* request, moderation::ModerateObjectResponse* response)
{
    std::lock_guard<std::mutex> Lock(this->mutex_);

    try {
        bool testingWordModeration = TextProcessor::TextProcessing(request->text());
        if(!testingWordModeration)
        {
            response->set_success(false);
            std::cout << "Moderation check passed for ID: " << id << std::endl;
        }
        else{
            response->set_success(true);
            std::cout << "Text flagged for moderation: " << request->text() << std::endl;
            std::cout << "Moderation check failed for ID: " << id << std::endl;
        }
        return Status::OK;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Service] ERROR: " << e.what() << std::endl;
        return Status(grpc::StatusCode::INTERNAL, e.what());
    }
}
