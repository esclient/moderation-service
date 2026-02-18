#include "config/test_constants.hpp"
#include "mocks/fake_moderation_repository.hpp"
#include "repository/repository.hpp"
#include <chrono>
#include <gtest/gtest.h>
#include <string>

TEST(FakeModerationRepository, SaveStoresRecord) {
    FakeModerationRepository repo;
    ModerationRecord record{};

    record.object_id = test_constants::TEST_ID_BASIC;
    record.object_type = moderation::OBJECT_TYPE_COMMENT_TEXT;
    record.text = "hello";
    record.is_flagged = true;
    record.reason = "bad";
    record.moderated_at = std::chrono::system_clock::now();

    bool save_result = repo.SaveModerationResult(record);
    ASSERT_TRUE(save_result);
    ASSERT_EQ(1u, repo.GetStoredRecords().size());
    EXPECT_EQ(moderation::config::TEST_ID_BASIC, repo.GetStoredRecords()[0].object_id);
    EXPECT_EQ(record.text, repo.GetStoredRecords()[0].text);
    EXPECT_TRUE(repo.GetStoredRecords()[0].is_flagged);
}

TEST(FakeModerationRepository, SaveMultipleRecords) {
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

TEST(FakeModerationRepository, SetSaveResultFalse) {
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

TEST(FakeModerationRepository, SaveStoresReasonAndType) {
    FakeModerationRepository repo;

    ModerationRecord record;
    record.object_id = test_constants::TEST_ID_BASIC;
    record.object_type = moderation::OBJECT_TYPE_COMMENT_TEXT;
    record.text = "bad content";
    record.is_flagged = true;
    record.moderated_at = std::chrono::system_clock::now();
    record.reason = "contains forbidden words";

    EXPECT_TRUE(repo.SaveModerationResult(record));
    ASSERT_EQ(1u, repo.GetStoredRecords().size());

    const auto& stored = repo.GetStoredRecords()[0];
    EXPECT_EQ(moderation::config::TEST_ID_BASIC, stored.object_id);
    EXPECT_EQ(moderation::OBJECT_TYPE_COMMENT_TEXT, stored.object_type);
    EXPECT_EQ("bad content", stored.text);
    EXPECT_TRUE(stored.is_flagged);
    EXPECT_EQ("contains forbidden words", stored.reason);
}

TEST(FakeModerationRepository, SaveWhenSetSaveResultFalse_DoesNotStore) {
    FakeModerationRepository repo;

    repo.SetSaveResult(false);

    ModerationRecord record;
    record.object_id = test_constants::TEST_ID_SECONDARY;
    record.text = "test";
    record.is_flagged = true;

    EXPECT_FALSE(repo.SaveModerationResult(record));
    EXPECT_TRUE(repo.GetStoredRecords().empty());
}

TEST(FakeModerationRepository, MultipleSavesWithMixedResults) {
    FakeModerationRepository repo;

    ModerationRecord record1;
    record1.object_id = 1;
    record1.text = "first";

    ModerationRecord record2;
    record2.object_id = 2;
    record2.text = "second";

    ModerationRecord record3;
    record3.object_id = 3;
    record3.text = "third";

    EXPECT_TRUE(repo.SaveModerationResult(record1));
    EXPECT_EQ(1u, repo.GetStoredRecords().size());

    repo.SetSaveResult(false);
    EXPECT_FALSE(repo.SaveModerationResult(record2));
    EXPECT_EQ(1u, repo.GetStoredRecords().size());

    repo.SetSaveResult(true);
    EXPECT_TRUE(repo.SaveModerationResult(record3));
    EXPECT_EQ(2u, repo.GetStoredRecords().size());

    EXPECT_EQ(1, repo.GetStoredRecords()[0].object_id);
    EXPECT_EQ(3, repo.GetStoredRecords()[1].object_id);
}
