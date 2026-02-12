#include "handler/handler.hpp"
#include "mocks/fake_moderation_repository.hpp"
#include "mocks/mock_kafka_client.hpp"
#include "mocks/test_utils.hpp"
#include "config/config.hpp"
#include "moderation.pb.h"
#include "service/service.hpp"
#include <grpcpp/grpcpp.h>
#include <gtest/gtest.h>
#include <memory>

TEST(ModerationHandlerTest, ModerateObject_EmptyText_InvalidArgument) {
    auto repo = std::make_shared<FakeModerationRepository>();
    KafkaConfig config{};
    config.brokers = "test";
    config.request_topic = "request";
    config.result_topic = "result";
    config.consumer_group_id = "cg";
    config.max_retries = 1;
    config.retry_backoff_ms = 100;
    config.enable_ssl = false;
    auto kafka = std::make_shared<MockKafkaClient>(config);
    auto service = std::make_shared<ModerationService>(repo, kafka);
    ModerationHandler handler(service);

    auto request = test_utils::MakeRequest(1, "");
    moderation::ModerateObjectResponse response;
    grpc::ServerContext context;

    grpc::Status status = handler.ModerateObject(&context, &request, &response);
    EXPECT_FALSE(status.ok());
    EXPECT_EQ(grpc::StatusCode::INVALID_ARGUMENT, status.error_code());
    EXPECT_FALSE(response.success());
}

TEST(ModerationHandlerTest, ModerateObject_ZeroId_InvalidArgument) {
    auto repo = std::make_shared<FakeModerationRepository>();
    KafkaConfig config{};
    config.brokers = "test";
    config.request_topic = "r";
    config.result_topic = "res";
    config.consumer_group_id = "cg";
    config.max_retries = 1;
    config.retry_backoff_ms = 100;
    config.enable_ssl = false;
    auto kafka = std::make_shared<MockKafkaClient>(config);
    auto service = std::make_shared<ModerationService>(repo, kafka);
    ModerationHandler handler(service);

    auto request = test_utils::MakeRequest(0, "hello");
    moderation::ModerateObjectResponse response;
    grpc::ServerContext ctx;

    grpc::Status status = handler.ModerateObject(&ctx, &request, &response);
    EXPECT_FALSE(status.ok());
    EXPECT_FALSE(response.success());
}
