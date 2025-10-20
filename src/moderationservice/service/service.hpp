#pragma once

#include <grpcpp/grpcpp.h>
#include "moderation.pb.h"
#include "moderation.grpc.pb.h"
#include <grpcpp/server_context.h>
#include <grpcpp/support/status.h>
#include <unordered_map>
#include <vector>
#include <mutex>

using grpc::Status;
using grpc::ServerContext;

class ModerationService final : public moderation::ModerationService::Service {

    public: 
        ModerationService();
        grpc::Status ModerateObject(grpc::ServerContext* context, const moderation::ModerateObjectRequest* request, moderation::ModerateObjectResponse* response) override;
    private:
        // Mock data
        std::unordered_map<int64_t, moderation::ModerateObjectRequest> moderations_;
        std::unordered_map<int64_t, std::vector<moderation::ModerateObjectResponse>> user_moderations_;
        std::mutex mutex_;
};
