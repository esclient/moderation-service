#pragma once

#include "repository/repository.hpp"
#include "moderation.pb.h"
#include "kafkaclient.hpp"
#include "text_processor.hpp"
#include "constants.hpp"
#include <unordered_map>
#include <vector>
#include <mutex>
#include <time.h>

namespace moderation {
    class ModerateObjectResponse;
    class ModerateObjectRequest;
}

struct PendingRequest
{
    std::chrono::system_clock::time_point timestamp;
};

class ModerationService final : public std::enable_shared_from_this<ModerationService> {

    public: 
        explicit ModerationService(std::shared_ptr<ModerationRepository> repository, std::shared_ptr<KafkaClient> kafkaClient);
        void InitializeKafkaCallback();
        bool ProcessModerationRequest(int64_t requestId, const std::string& text);
        void HandleModerationResult(const moderation::ModerateObjectResponse& response, int64_t requestId, const std::string& originalText);

    private:
        void SaveResultToDatabase(int64_t request_id, const std::string& text, bool is_flagged, const std::string& reason = "");
        std::shared_ptr<ModerationRepository> repository_;
        std::shared_ptr<KafkaClient> kafkaClient_;
        std::mutex pendingMutex_;
        std::mutex mutex_;
};
