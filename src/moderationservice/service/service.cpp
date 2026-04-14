#include "service/service.hpp"
#include "interceptors/logger.hpp"
#include "model/model_utils.hpp"
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
                SERVICE_LOG_ERROR(moderation::logging::Subsystem::kService, "SERVICE_EXPIRED",
                                  "ModerationService expired, cannot handle result");
            }
        } catch (const std::exception& e) {
            SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "KAFKA_CALLBACK_FAIL",
                              e.what());
        }
    };
    kafkaClient_->Initialize(callback);
}
bool ModerationService::ProcessModerationRequest(int64_t request_id, const std::string& text) {
    try {
        if (text.empty() || request_id == 0) {
            SERVICE_LOG_ERROR(moderation::logging::Subsystem::kService, grpc::StatusCode::INVALID_ARGUMENT,
                              "invalid moderation request: text empty or request_id is zero");
            return false;
        }

        moderation::ModerateObjectRequest kafka_request;
        kafka_request.set_id(request_id);
        kafka_request.set_text(text);

        if (!kafkaClient_->SendRequestAsync(kafka_request)) {
            SERVICE_LOG_ERROR(moderation::logging::Subsystem::kKafka, "SEND_REQUEST_FAIL",
                              absl::StrCat("failed to send moderation request request_id=", request_id));
            return false;
        }

        return true;
    } catch (const std::exception& e) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kService, grpc::StatusCode::INTERNAL,
                          e.what());
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
            SERVICE_VLOG1(absl::StrCat("text flagged request_id=", request_id));
            SaveResultToDatabase(request_id, originalText, isFlagged, reason, objectType);

        } else {
            SERVICE_VLOG1(absl::StrCat("text passed request_id=", request_id));
        }
    } catch (const std::exception& e) {
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kService, "HANDLE_RESULT_FAIL", e.what());
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
        SERVICE_LOG_ERROR(moderation::logging::Subsystem::kRepository, "SAVE_RESULT_FAIL",
                          absl::StrCat("failed to save moderation result object_id=", object_id));
    } else {
        std::string type_name = moderation::utils::ObjectTypeToString(object_type);
        SERVICE_VLOG1(absl::StrCat("moderation result saved object_id=", object_id,
                                   " type=", type_name));
    }
}
