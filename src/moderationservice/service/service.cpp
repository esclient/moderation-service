#include "service.hpp"


ModerationService::ModerationService(std::shared_ptr<ModerationRepository> repository,
    std::shared_ptr<KafkaClient> kafkaClient) : repository_(std::move(repository)),
    kafkaClient_(std::move(kafkaClient)) {
    TextProcessingConstants::HashTrieMaps::InitializeForbiddenWords();
}

void ModerationService::InitializeKafkaCallback()
{
    auto callback = [weakThis = std::weak_ptr<ModerationService>(shared_from_this())]
                    (const moderation::ModerateObjectResponse& response, int64_t requestId, const std::string& originalText)
                    {
                        try {
                            if(auto self = weakThis.lock())
                            {
                                self->HandleModerationResult(response, requestId, originalText);
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


bool ModerationService::ProcessModerationRequest(int64_t request_id, const std::string& text)
{
    try{
        if(text.empty() || request_id == 0)
        {
            std::cerr << "[Service] ERROR: Invalid moderation request. Text is empty or request ID is zero." << std::endl;
            return false;
        }

        moderation::ModerateObjectRequest kafka_request;
        kafka_request.set_id(request_id);
        kafka_request.set_text(text);

        if(!kafkaClient_->SendRequestAsync(kafka_request))
        {
            std::cerr << "[Service] ERROR: Failed to send moderation request to Kafka." << request_id << std::endl;
            return false;
        }

        return true;
    } catch(const std::exception& e)
    {
        std::cerr << "[Service] ERROR: " << e.what() << std::endl;
        return false;
    }
}

void ModerationService::HandleModerationResult(const moderation::ModerateObjectResponse& response, int64_t requestId, const std::string& originalText, moderation::ObjectType objectType)
{

    try {
        bool isFlagged = response.success();
        std::string reason = "";

        if(isFlagged)
        {
            std::cout << "Text flagged for moderation, request id: " << requestId << std::endl;
            SaveResultToDatabase(requestId, originalText, isFlagged, reason, objectType);

        }
        else {
            std::cout << "Text passed moderation: " << requestId << std::endl;
        }
        
    } catch(const std::exception& e) {
        std::cerr << "Error handling moderation result: " << e.what() << std::endl;
    }
}

void ModerationService::SaveResultToDatabase(int64_t object_id, const std::string& text, bool is_flagged, const std::string& reason, moderation::ObjectType object_type)
{
    ModerationRecord record;
    record.object_id = object_id;
    record.object_type = object_type;
    record.text = text;
    record.is_flagged = is_flagged; 
    record.moderated_at = std::chrono::system_clock::now();
    record.reason = reason;

    if(!repository_->SaveModerationResult(record))
    {
        std::cerr << "Failed to save moderation result to database for object ID: " << object_id << std::endl;
    }
    else{
        std::cout << "Moderation result saved to database for object ID: " << object_id << std::endl;
    }
}