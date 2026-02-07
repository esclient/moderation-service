#pragma once

#include "moderation.pb.h" // NOLINT(build/include_subdir)
#include "moderationservice/kafka/kafkaclient.hpp"
#include "moderationservice/model/constants.hpp"
#include "moderationservice/repository/repository.hpp"
#include "moderationservice/service/text_processor.hpp"
#include <memory>
#include <mutex>
#include <string>
#include <time.h>
#include <unordered_map>
#include <vector>

namespace moderation {
class ModerateObjectResponse;
class ModerateObjectRequest;
} // namespace moderation

struct PendingRequest {
    std::chrono::system_clock::time_point timestamp;
};

class ModerationService final : public std::enable_shared_from_this<ModerationService> {
  public:
    explicit ModerationService(std::shared_ptr<ModerationRepository> repository,
                               std::shared_ptr<KafkaClient> kafkaClient);
    void InitializeKafkaCallback();
    bool ProcessModerationRequest(int64_t requestId, const std::string& text);
    void HandleModerationResult(const moderation::ModerateObjectResponse& response,
                                int64_t requestId, const std::string& originalText,
                                moderation::ObjectType objectType);

  private:
    void SaveResultToDatabase(
        int64_t object_id, const std::string& text, bool is_flagged, const std::string& reason = "",
        moderation::ObjectType object_type = moderation::ObjectType::OBJECT_TYPE_UNSPECIFIED);
    std::shared_ptr<ModerationRepository> repository_;
    std::shared_ptr<KafkaClient> kafkaClient_;
    std::mutex pendingMutex_;
    std::mutex mutex_;
};
