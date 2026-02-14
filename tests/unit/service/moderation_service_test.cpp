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
        KafkaConfig config{};
        config.brokers = "test";
        config.request_topic = "request";
        config.result_topic = "result";
        config.consumer_group_id = "cg";
        config.max_retries = 1;
        config.retry_backoff_ms = 100;
        config.enable_ssl = false;

        repo_ = std::make_shared<FakeModerationRepository>();
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

TEST_F(ModerationServiceTest, ProcessModerationRequest_ValidRequest_SendsToKafka) {
    EXPECT_TRUE(service_->ProcessModerationRequest(123, "hello world"));
    
    ASSERT_EQ(1u, kafka_->GetSentRequests().size());
    
    const auto& sent = kafka_->GetSentRequests()[0];
    EXPECT_EQ(123, sent.id());
    EXPECT_EQ("hello world", sent.text());
}

TEST_F(ModerationServiceTest, HandleModerationResult_NotFlagged_DoesNotSave) {
    auto response = test_utils::MakeResponse(false);  
    
    service_->HandleModerationResult(response, 55, "clean text",
                                     moderation::OBJECT_TYPE_COMMENT_TEXT);
    
    EXPECT_TRUE(repo_->GetStoredRecords().empty());
}

TEST_F(ModerationServiceTest, ProcessModerationRequest_SendFails_ReturnsFalse) {
    kafka_->SetSendResult(false);  
    
    EXPECT_FALSE(service_->ProcessModerationRequest(1, "test text"));

    EXPECT_TRUE(repo_->GetStoredRecords().empty());
}

TEST_F(ModerationServiceTest, HandleModerationResult_SavesFlaggedContent) {
    auto response = test_utils::MakeResponse(true);
    
    service_->HandleModerationResult(response, 777, "bad content",
                                     moderation::OBJECT_TYPE_COMMENT_TEXT);
    
    ASSERT_EQ(1u, repo_->GetStoredRecords().size());
    
    const auto& record = repo_->GetStoredRecords()[0];
    EXPECT_EQ(777, record.object_id);
    EXPECT_EQ("bad content", record.text);
    EXPECT_EQ(moderation::OBJECT_TYPE_COMMENT_TEXT, record.object_type);
    EXPECT_TRUE(record.is_flagged);
}

TEST_F(ModerationServiceTest, ProcessModerationRequest_MultipleRequests) {
    EXPECT_TRUE(service_->ProcessModerationRequest(1, "first"));
    EXPECT_TRUE(service_->ProcessModerationRequest(2, "second"));
    EXPECT_TRUE(service_->ProcessModerationRequest(3, "third"));
    
    ASSERT_EQ(3u, kafka_->GetSentRequests().size());
    EXPECT_EQ(1, kafka_->GetSentRequests()[0].id());
    EXPECT_EQ(2, kafka_->GetSentRequests()[1].id());
    EXPECT_EQ(3, kafka_->GetSentRequests()[2].id());
}

TEST_F(ModerationServiceTest, HandleModerationResult_RepositoryFailure_LogsError) {
    repo_->SetSaveResult(false);  
    
    auto response = test_utils::MakeResponse(true);
    
    service_->HandleModerationResult(response, 999, "content",
                                     moderation::OBJECT_TYPE_COMMENT_TEXT);
    
    EXPECT_TRUE(repo_->GetStoredRecords().empty());
}
