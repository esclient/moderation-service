#include "tests/mocks/fake_moderation_repository.hpp"
#include "moderationservice/repository/repository.hpp"
#include <chrono>
#include <gtest/gtest.h>
#include <string>

TEST(FakeModerationRepository, SaveStoresRecord){
    FakeModerationRepository repo;
    ModerationRecord record{};
    
    record.object_id = 42;
    record.object_type = moderation::OBJECT_TYPE_COMMENT_TEXT;
    record.text = "hello";
    record.is_flagged = true;
    record.reason = "bad";
    record.moderated_at = std::chrono::system_clock::now();

    bool ok = repo.SaveModerationResult(record);
    ASSERT_TRUE(ok);
    ASSERT_EQ(1u, repo.GetStoredRecords().size());
    EXPECT_EQ(42, repo.GetStoredRecords()[0].object_id);
    EXPECT_EQ(record.text, repo.GetStoredRecords()[0].text);
    EXPECT_TRUE(repo.GetStoredRecords()[0].is_flagged);
}

TEST(FakeModerationRepository, SaveMultipleRecords){
    FakeModerationRepository repo;
    ModerationRecord aRecord{};
    aRecord.object_id = 1;
    aRecord.text = "a";
    aRecord.moderated_at = std::chrono::system_clock::now();
    ModerationRecord bRecord{};
    bRecord.object_id = 2;
    bRecord.text = "b";
    bRecord.moderated_at = std::chrono::system_clock::now();

    repo.SaveModerationResult(aRecord);
    repo.SaveModerationResult(bRecord);
    ASSERT_EQ(2u, repo.GetStoredRecords().size());
    EXPECT_EQ(1, repo.GetStoredRecords()[0].object_id);
    EXPECT_EQ(2, repo.GetStoredRecords()[1].object_id);
}

TEST(FakeModerationRepository, SetSaveResultFalse){
    FakeModerationRepository repo;
    repo.SetSaveResult(false);
    ModerationRecord record{};
    record.object_id = 1;
    record.moderated_at = std::chrono::system_clock::now();
    EXPECT_FALSE(repo.SaveModerationResult(record));
    EXPECT_TRUE(repo.GetStoredRecords().empty());
}

TEST(FakeModerationRepository, ClearEmptiesStored) {
    FakeModerationRepository repo;
    ModerationRecord record{};
    record.object_id = 1;
    record.moderated_at = std::chrono::system_clock::now();
    repo.SaveModerationResult(record);
    repo.Clear();
    EXPECT_TRUE(repo.GetStoredRecords().empty());
}