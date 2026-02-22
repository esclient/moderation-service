#include "config/config.hpp"
#include "config/test_constants.hpp"
#include "handler/handler.hpp"
#include "mocks/fake_moderation_repository.hpp"
#include "mocks/mock_kafka_client.hpp"
#include "mocks/test_utils.hpp"
#include "moderation.pb.h"
#include "service/service.hpp"
#include <grpcpp/grpcpp.h>
#include <gtest/gtest.h>
#include <memory>
#include <string>

TEST(ModerationHandlerTest, ModerateObject_EmptyText_InvalidArgument) {
    KafkaConfig config{};
    config.brokers = "test";
    config.request_topic = "request";
    config.result_topic = "result";
    config.consumer_group_id = "cg";
    config.max_retries = 1;
    config.retry_backoff_ms = test_constants::TEST_RETRY_BACKOFF_MS;
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
    config.request_topic = "request";
    config.result_topic = "result";
    config.consumer_group_id = "cg";
    config.max_retries = 1;
    config.retry_backoff_ms = test_constants::TEST_RETRY_BACKOFF_MS;
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
    config.retry_backoff_ms = test_constants::TEST_RETRY_BACKOFF_MS;
    config.enable_ssl = false;

    auto repo = std::make_shared<FakeModerationRepository>();
    auto kafka = std::make_shared<MockKafkaClient>(config);
    kafka->SetSendResult(true);

    auto service = std::make_shared<ModerationService>(repo, kafka);
    ModerationHandler handler(service);

    auto request = test_utils::MakeRequest(test_constants::TEST_ID_HANDLER, "hello world");
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
    config.retry_backoff_ms = test_constants::TEST_RETRY_BACKOFF_MS;
    config.enable_ssl = false;

    auto repo = std::make_shared<FakeModerationRepository>();
    auto kafka = std::make_shared<MockKafkaClient>(config);
    kafka->SetSendResult(true);

    auto service = std::make_shared<ModerationService>(repo, kafka);
    ModerationHandler handler(service);

    std::string long_text(test_constants::TEST_LONG_TEXT_LENGTH, 'a');
    auto request = test_utils::MakeRequest(test_constants::TEST_ID_LONG_TEXT, long_text);
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
    config.retry_backoff_ms = test_constants::TEST_RETRY_BACKOFF_MS;
    config.enable_ssl = false;

    auto repo = std::make_shared<FakeModerationRepository>();
    auto kafka = std::make_shared<MockKafkaClient>(config);
    kafka->SetSendResult(false);

    auto service = std::make_shared<ModerationService>(repo, kafka);
    ModerationHandler handler(service);

    auto request = test_utils::MakeRequest(test_constants::TEST_ID_MAX, "test text");
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
    config.retry_backoff_ms = test_constants::TEST_RETRY_BACKOFF_MS;
    config.enable_ssl = false;

    auto repo = std::make_shared<FakeModerationRepository>();
    auto kafka = std::make_shared<MockKafkaClient>(config);
    kafka->SetSendResult(true);

    auto service = std::make_shared<ModerationService>(repo, kafka);
    ModerationHandler handler(service);

    auto request = test_utils::MakeRequest(test_constants::TEST_ID_SPECIAL, "Hello 👋 世界 @#$%");
    moderation::ModerateObjectResponse response;
    grpc::ServerContext context;

    grpc::Status status = handler.ModerateObject(&context, &request, &response);

    EXPECT_TRUE(status.ok());
    EXPECT_TRUE(response.success());
}
