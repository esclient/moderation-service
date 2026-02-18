#include "service/service.hpp"
#include "model/model_utils.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <utility>

ModerationService::ModerationService(std::shared_ptr<IModerationRepository> repository,
                                     std::shared_ptr<KafkaClient> kafkaClient)
    : repository_(std::move(repository)), kafkaClient_(std::move(kafkaClient)) {
    TextProcessingConstants::HashTrieMaps::InitializeForbiddenWords();
}
void ModerationService::InitializeKafkaCallback() {
    auto callback = [weakThis = std::weak_ptr<ModerationService>(shared_from_this())](
                        const moderation::ModerateObjectResponse& response, int64_t request_id,
                        const std::string& originalText, moderation::ObjectType object_type) {
        try {
            if (auto self = weakThis.lock()) {
                self->HandleModerationResult(response, request_id, originalText, object_type);
            } else {
                std::cerr << "ModerationService expired, cannot handle result" << "\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Error in Kafka callback: " << e.what() << "\n";
        }
    };
    kafkaClient_->Initialize(callback);
}
bool ModerationService::ProcessModerationRequest(int64_t request_id, const std::string& text) {
    try {
        if (text.empty() || request_id == 0) {
            std::cerr << "[Service] ERROR: Invalid moderation request. Text is empty or request ID "
                         "is zero."
                      << "\n";
            return false;
        }

        moderation::ModerateObjectRequest kafka_request;
        kafka_request.set_id(request_id);
        kafka_request.set_text(text);

        if (!kafkaClient_->SendRequestAsync(kafka_request)) {
            std::cerr << "[Service] ERROR: Failed to send moderation request to Kafka."
                      << request_id << "\n";
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Service] ERROR: " << e.what() << "\n";
        return false;
    }
}
void ModerationService::HandleModerationResult(const moderation::ModerateObjectResponse& response,
                                               int64_t request_id, const std::string& originalText,
                                               moderation::ObjectType objectType) {
    try {
        bool isFlagged = response.success();

        if (isFlagged) {
            std::string reason =
                "Violates content policy"; // This can be enhanced to provide specific
                                           // reasons based on the response
            std::cout << "Text flagged for moderation, request id: " << request_id << "\n";
            SaveResultToDatabase(request_id, originalText, isFlagged, reason, objectType);

        } else {
            std::cout << "Text passed moderation: " << request_id << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling moderation result: " << e.what() << "\n";
    }
}

void ModerationService::SaveResultToDatabase(int64_t object_id, const std::string& text,
                                             bool is_flagged, const std::string& reason,
                                             moderation::ObjectType object_type) {
    ModerationRecord record;
    record.object_id = object_id;
    record.object_type = object_type;
    record.text = text;
    record.is_flagged = is_flagged;
    record.moderated_at = std::chrono::system_clock::now();
    record.reason = reason;

    if (!repository_->SaveModerationResult(record)) {
        std::cerr << "Failed to save moderation result to database for object ID: " << object_id
                  << "\n";
    } else {
        std::string type_name = moderation::utils::ObjectTypeToString(object_type);

        std::cout << "Moderation result saved to database for object ID: " << object_id
                  << ", Type: " << type_name << "\n";
    }
}
