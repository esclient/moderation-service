#include "service.hpp"
#include "text_processor.hpp"
#include "constants.hpp"

ModerationService::ModerationService() {
    TextProcessingConstants::HashTrieMaps::InitializeForbiddenWords();
}

Status ModerationService::ModerateObject(grpc::ServerContext* context, const moderation::ModerateObjectRequest* request, moderation::ModerateObjectResponse* response)
{
    std::lock_guard<std::mutex> Lock(this->mutex_);

    try {
        bool testingWordModeration = TextProcessor::TextProcessing(request->text());
        if(!testingWordModeration)
        {
            response->set_success(false);
            std::cout << "Moderation check passed for ID: " << request->id() << std::endl;
        }
        else{
            response->set_success(true);
            std::cout << "Text flagged for moderation: " << request->text() << std::endl;
            std::cout << "Moderation check failed for ID: " << request->id() << std::endl;
        }
        return Status::OK;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Service] ERROR: " << e.what() << std::endl;
        return Status(grpc::StatusCode::INTERNAL, e.what());
    }
}
