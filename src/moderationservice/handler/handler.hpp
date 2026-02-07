#pragma once

#include "moderation.grpc.pb.h"
#include "moderation.pb.h"
#include "service/service.hpp"
#include <grpcpp/grpcpp.h>
#include <memory>

class ModerationHandler final : public moderation::ModerationService::Service {
  public:
    explicit ModerationHandler(std::shared_ptr<ModerationService> service);

    grpc::Status ModerateObject(grpc::ServerContext* context,
                                const moderation::ModerateObjectRequest* request,
                                moderation::ModerateObjectResponse* response) override;

  private:
    std::shared_ptr<ModerationService> service_;
};
