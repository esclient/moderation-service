#include "mocks/fake_moderation_repository.hpp"
#include "mocks/mock_kafka_client.hpp"
#include "mocks/test_utils.hpp"
#include "config/config.hpp"
#include "service/service.hpp"
#include <gtest/gtest.h>
#include <memory>

class ModerationServiceTest : public ::testing::Test {
    protected:
    void SetUp() override {
        repo_ = std::make_shared<FakeModerationRepository>();
        KafkaConfig config{};
        config.brokers = "test";
        config.request_topic = "request";
        config.result_topic = "result";
        config.consumer_group_id = "cg";
        config.max_retries = 1;
        config.retry_backoff_ms = 100;
        config.enable_ssl = false;
        kafka_ = std::make_shared<MockKafkaClient>(config);
        service_ = std::make_shared<ModerationService>(repo_, kafka_);
        service_->InitializeKafkaCallback();
    }

    std::shared_ptr<FakeModerationRepository> repo_;
    std::shared_ptr<MockKafkaClient> kafka_;
    std::shared_ptr<ModerationService> service_;
};

TEST_F(ModerationServiceTest, ProcessModerationRequest_InvalidIdFails) {
    EXPECT_FALSE(service_->ProcessModerationRequest(0, "some text"));
}

TEST_F(ModerationServiceTest, ProcessModerationRequest_EmptyTextFails) {
    EXPECT_FALSE(service_->ProcessModerationRequest(1, ""));
}

TEST_F(ModerationServiceTest, HandleModerationResult_FlaggedSavesToRepo) {    
    auto response = test_utils::MakeResponse(true);    
    service_->HandleModerationResult(response, 99, "original text",         
                                    moderation::OBJECT_TYPE_COMMENT_TEXT);    
    ASSERT_EQ(1u, repo_->GetStoredRecords().size());    
    EXPECT_EQ(99, repo_->GetStoredRecords()[0].object_id);    
    EXPECT_TRUE(repo_->GetStoredRecords()[0].is_flagged);
}