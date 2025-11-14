#include "service.hpp"

ModerationService::ModerationService(std::shared_ptr<KafkaClient> kafkaClient) : kafkaClient_(std::move(kafkaClient)) {
    TextProcessingConstants::HashTrieMaps::InitializeForbiddenWords();
}

void ModerationService::InitializeKafkaCallback()
{
    auto callback = [weakThis = std::weak_ptr<ModerationService>(shared_from_this())]
                    (const moderation::ModerateObjectResponse& response, int64_t requestId)
                    {
                        try {
                            if(auto self = weakThis.lock())
                            {
                                self->HandleModerationResult(response, requestId);
                            }
                            else
                            {
                                std::cerr << "ModerationService expired, cannot handle result" << std::endl;
                            }
                        } catch(const std::exception& e)
                        {
                            std::cerr << "Error in Kafka callback: " << e.what() << std::endl;
                        }
                    };
    kafkaClient_->Initialize(callback);
}

Status ModerationService::ModerateObject(grpc::ServerContext* context, const moderation::ModerateObjectRequest* request, moderation::ModerateObjectResponse* response)
{
    try {
        if(request->text().empty() || request->id() == 0)
        {
            response->set_success(false);
            return Status(grpc::StatusCode::INVALID_ARGUMENT, "Request must contain non-empty text and ID.");
        }
        
        moderation::ModerateObjectRequest copy = *request;

        if(!kafkaClient_->SendRequestAsync(copy))
        {
            std::cerr << "[Service] ERROR: Failed to send moderation request to Kafka." << request->id() << std::endl;
            return Status(grpc::StatusCode::INTERNAL, "Failed to enqueue moderation request.");
        }

        response->set_success(true);
        return Status::OK;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Service] ERROR: " << e.what() << std::endl;
        return Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

void ModerationService::HandleModerationResult(const moderation::ModerateObjectResponse& response, int64_t requestId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    try {
        bool isFlagged = response.success();

        if(isFlagged)
        {
            std::cout << "Text flagged for moderation, request id: " << requestId << std::endl;
            user_moderations_[requestId].push_back(response);
        }
        else {
            std::cout << "Text passed moderation: " << requestId << std::endl;
        }
    } catch(const std::exception& e) {
        std::cerr << "Error handling moderation result: " << e.what() << std::endl;
    }
}
