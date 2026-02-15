#include "config/config.hpp"
#include "handler/handler.hpp"
#include "mocks/fake_moderation_repository.hpp"
#include "mocks/mock_kafka_client.hpp"
#include "mocks/test_utils.hpp"
#include "moderation.pb.h"
#include "service/service.hpp"
#include <grpcpp/grpcpp.h>
#include <gtest/gtest.h>
#include <memory>

TEST(ModerationHandlerTest, ModerateObject_EmptyText_InvalidArgument) {
    KafkaConfig config{};
    config.brokers = "test";
    config.request_topic = "request";
    config.result_topic = "result";
    config.consumer_group_id = "cg";
    config.max_retries = 1;
    config.retry_backoff_ms = 100;
    config.enable_ssl = false;

    std::shared_ptr<IModerationRepository> repo = std::make_shared<FakeModerationRepository>();
    std::shared_ptr<KafkaClient> kafka = std::make_shared<MockKafkaClient>(config);
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
    KafkaConfig config{};
    config.brokers = "test";
    config.request_topic = "r";
    config.result_topic = "res";
    config.consumer_group_id = "cg";
    config.max_retries = 1;
    config.retry_backoff_ms = 100;
    config.enable_ssl = false;

    std::shared_ptr<IModerationRepository> repo = std::make_shared<FakeModerationRepository>();
    std::shared_ptr<KafkaClient> kafka = std::make_shared<MockKafkaClient>(config);
    auto service = std::make_shared<ModerationService>(repo, kafka);
    ModerationHandler handler(service);

    auto request = test_utils::MakeRequest(0, "hello");
    moderation::ModerateObjectResponse response;
    grpc::ServerContext ctx;

    grpc::Status status = handler.ModerateObject(&ctx, &request, &response);
    EXPECT_FALSE(status.ok());
    EXPECT_FALSE(response.success());
}

TEST(ModerationHandlerTest, ModerateObject_ValidRequest_ReturnsOk) {
    KafkaConfig config{};
    config.brokers = "test";
    config.request_topic = "request";
    config.result_topic = "result";
    config.consumer_group_id = "cg";
    config.max_retries = 1;
    config.retry_backoff_ms = 100;
    config.enable_ssl = false;

    auto repo = std::make_shared<FakeModerationRepository>();
    auto kafka = std::make_shared<MockKafkaClient>(config);
    kafka->SetSendResult(true);

    auto service = std::make_shared<ModerationService>(repo, kafka);
    ModerationHandler handler(service);

    auto request = test_utils::MakeRequest(123, "hello world");
    moderation::ModerateObjectResponse response;
    grpc::ServerContext context;

    grpc::Status status = handler.ModerateObject(&context, &request, &response);

    EXPECT_TRUE(status.ok());
    EXPECT_TRUE(response.success());
}

TEST(ModerationHandlerTest, ModerateObject_LongText_ReturnsOk) {
    KafkaConfig config{};
    config.brokers = "test";
    config.request_topic = "request";
    config.result_topic = "result";
    config.consumer_group_id = "cg";
    config.max_retries = 1;
    config.retry_backoff_ms = 100;
    config.enable_ssl = false;

    auto repo = std::make_shared<FakeModerationRepository>();
    auto kafka = std::make_shared<MockKafkaClient>(config);
    kafka->SetSendResult(true);

    auto service = std::make_shared<ModerationService>(repo, kafka);
    ModerationHandler handler(service);

    std::string long_text(1000, 'a');
    auto request = test_utils::MakeRequest(456, long_text);
    moderation::ModerateObjectResponse response;
    grpc::ServerContext context;

    grpc::Status status = handler.ModerateObject(&context, &request, &response);

    EXPECT_TRUE(status.ok());
    EXPECT_TRUE(response.success());
}

TEST(ModerationHandlerTest, ModerateObject_KafkaFailure_ReturnsError) {
    KafkaConfig config{};
    config.brokers = "test";
    config.request_topic = "request";
    config.result_topic = "result";
    config.consumer_group_id = "cg";
    config.max_retries = 1;
    config.retry_backoff_ms = 100;
    config.enable_ssl = false;

    auto repo = std::make_shared<FakeModerationRepository>();
    auto kafka = std::make_shared<MockKafkaClient>(config);
    kafka->SetSendResult(false);

    auto service = std::make_shared<ModerationService>(repo, kafka);
    ModerationHandler handler(service);

    auto request = test_utils::MakeRequest(789, "test text");
    moderation::ModerateObjectResponse response;
    grpc::ServerContext context;

    grpc::Status status = handler.ModerateObject(&context, &request, &response);

    EXPECT_FALSE(status.ok());
    EXPECT_FALSE(response.success());
}

TEST(ModerationHandlerTest, ModerateObject_SpecialCharacters_ReturnsOk) {
    KafkaConfig config{};
    config.brokers = "test";
    config.request_topic = "request";
    config.result_topic = "result";
    config.consumer_group_id = "cg";
    config.max_retries = 1;
    config.retry_backoff_ms = 100;
    config.enable_ssl = false;

    auto repo = std::make_shared<FakeModerationRepository>();
    auto kafka = std::make_shared<MockKafkaClient>(config);
    kafka->SetSendResult(true);

    auto service = std::make_shared<ModerationService>(repo, kafka);
    ModerationHandler handler(service);

    auto request = test_utils::MakeRequest(999, "Hello 👋 世界 @#$%");
    moderation::ModerateObjectResponse response;
    grpc::ServerContext context;

    grpc::Status status = handler.ModerateObject(&context, &request, &response);

    EXPECT_TRUE(status.ok());
    EXPECT_TRUE(response.success());
}
