#pragma once

#include <grpcpp/grpcpp.h>
#include "moderation.pb.h"
#include "moderation.grpc.pb.h"
#include <grpcpp/server_context.h>
#include <grpcpp/support/status.h>
#include "kafkaclient.hpp"
#include "text_processor.hpp"
#include "constants.hpp"
#include <unordered_map>
#include <vector>
#include <mutex>
#include <time.h>

using grpc::Status;
using grpc::ServerContext;

struct PendingRequest
{
    moderation::ModerateObjectRequest request;
    std::chrono::system_clock::time_point timestamp;
};

class ModerationService final : public moderation::ModerationService::Service,
                                public std::enable_shared_from_this<ModerationService> {

    public: 
        explicit ModerationService(std::shared_ptr<KafkaClient> kafkaClient);
        void InitializeKafkaCallback();
        grpc::Status ModerateObject(grpc::ServerContext* context, const moderation::ModerateObjectRequest* request, moderation::ModerateObjectResponse* response) override;
    private:
        void HandleModerationResult(const moderation::ModerateObjectResponse& response, int64_t requestId);
        std::shared_ptr<KafkaClient> kafkaClient_;
        std::unordered_map<int64_t, PendingRequest> pendingRequests_;
        std::unordered_map<int64_t, moderation::ModerateObjectRequest> moderations_;
        std::unordered_map<int64_t, std::vector<moderation::ModerateObjectResponse>> user_moderations_;
        std::mutex pendingMutex_;
        std::mutex mutex_;
};
