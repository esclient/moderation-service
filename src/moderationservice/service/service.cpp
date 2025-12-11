#include "service.hpp"


ModerationService::ModerationService(std::shared_ptr<ModerationRepository> repository,
    std::shared_ptr<KafkaClient> kafkaClient) : repository_(std::move(repository)),
    kafkaClient_(std::move(kafkaClient)) {
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

        {
            std::lock_guard<std::mutex> lock(pendingMutex_);
            pending_text_[request_id] = text;
            std::cout << "[Service] Stored text for request ID " << request_id 
                      << " (text length: " << text.length() << ")" << std::endl;
        }

        return true;
    } catch(const std::exception& e)
    {
        std::cerr << "[Service] ERROR: " << e.what() << std::endl;
        return false;
    }
}

void ModerationService::HandleModerationResult(const moderation::ModerateObjectResponse& response, int64_t requestId)
{

    try {
        bool isFlagged = response.success();
        std::string reason = "";

        std::string original_text;
        {
            std::lock_guard<std::mutex> lock(pendingMutex_);
            std::cout << "[Service] DEBUG: Looking for request ID " << requestId 
                      << " in map. Map size: " << pending_text_.size() << std::endl;

            std::cout << "[Service] DEBUG: Map contains request IDs: ";
            for(const auto& pair : pending_text_) {
                std::cout << pair.first << " ";
            }
            std::cout << std::endl;

            auto it = pending_text_.find(requestId);
            if(it != pending_text_.end())
            {
                original_text = it->second;
                std::cout << "[Service] Retrieved text for request ID " << requestId 
                          << " (length: " << original_text.length() << ")" << std::endl;
                pending_text_.erase(it);
            }
            else
            {
               std::cerr << "[Service] ERROR: Original text not found for request ID: " << requestId 
                          << ". Map size: " << pending_text_.size() << std::endl;
            }
        }

        if(isFlagged)
        {
            std::cout << "Text flagged for moderation, request id: " << requestId << std::endl;
        }
        else {
            std::cout << "Text passed moderation: " << requestId << std::endl;
        }

        
        SaveResultToDatabase(requestId, original_text, isFlagged, reason);
    } catch(const std::exception& e) {
        std::cerr << "Error handling moderation result: " << e.what() << std::endl;
    }
}

void ModerationService::SaveResultToDatabase(int64_t request_id, const std::string& text, bool is_flagged, const std::string& reason)
{
    ModerationRecord record;
    record.request_id = request_id;
    record.text = text;
    record.is_flagged = is_flagged; 
    record.moderated_at = std::chrono::system_clock::now();
    record.reason = reason;

    if(!repository_->SaveModerationResult(record))
    {
        std::cerr << "Failed to save moderation result to database for request ID: " << request_id << std::endl;
    }
    else{
        std::cout << "Moderation result saved to database for request ID: " << request_id << std::endl;
    }
}