#include "model/constants.hpp"
#include "service/word_checker.hpp"
#include <gtest/gtest.h>
#include <string>
#include <vector>

class WordCheckerTest : public ::testing::Test {
  protected:
    void SetUp() override { TextProcessingConstants::HashTrieMaps::InitializeForbiddenWords(); }
};

TEST_F(WordCheckerTest, TokenizeSimple) {
    auto tokens = WordChecker::Tokenize("hello world");
    ASSERT_EQ(2U, tokens.size());
    EXPECT_EQ(tokens[0], "hello");
    EXPECT_EQ(tokens[1], "world");
}

TEST_F(WordCheckerTest, TokenizeEmpty) {
    auto tokens = WordChecker::Tokenize("");
    EXPECT_TRUE(tokens.empty());
}

TEST_F(WordCheckerTest, WordCheckingUnderThreshold) {
    std::vector<std::string> few_bad = {"hello", "gay", "world"};
    EXPECT_FALSE(WordChecker::WordChecking(few_bad));
}

TEST_F(WordCheckerTest, WordCheckingOverThreshold) {
    std::vector<std::string> many_bad = {"gay", "gay", "gay", "gay", "gay"};
    EXPECT_TRUE(WordChecker::WordChecking(many_bad));
}

TEST_F(WordCheckerTest, Tokenize_SpecialCharacters) {
    auto tokens = WordChecker::Tokenize("@#$%");

    EXPECT_TRUE(tokens.empty());
}

TEST_F(WordCheckerTest, TokenizeUnicodeAndEmoji) {
    std::string text = "привет😀мир";
    auto tokens = WordChecker::Tokenize(text);

    ASSERT_EQ(tokens.size(), 2U);
    EXPECT_EQ(tokens[0], "привет");
    EXPECT_EQ(tokens[1], "мир");
}

TEST_F(WordCheckerTest, TokenizeEmojiBetweenAsciiWords) {
    std::string text = "hello😀world";
    auto tokens = WordChecker::Tokenize(text);

    ASSERT_EQ(tokens.size(), 2U);
    EXPECT_EQ(tokens[0], "hello");
    EXPECT_EQ(tokens[1], "world");
}

TEST_F(WordCheckerTest, WordCheckingExactlyAtThresholdIsFalse) {
    std::vector<std::string> at_threshold = {"gay", "gay", "gay"};
    EXPECT_FALSE(WordChecker::WordChecking(at_threshold));
}

TEST_F(WordCheckerTest, WordCheckingNoForbiddenWords) {
    std::vector<std::string> safe = {"hello", "world", "test"};
    EXPECT_FALSE(WordChecker::WordChecking(safe));
}
