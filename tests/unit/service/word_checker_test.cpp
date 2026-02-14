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
    ASSERT_EQ(2u, tokens.size());
    EXPECT_EQ(tokens[0], "hello");
    EXPECT_EQ(tokens[1], "world");
}

TEST_F(WordCheckerTest, TokenizeEmpty) {
    auto tokens = WordChecker::Tokenize("");
    EXPECT_TRUE(tokens.empty());
}

TEST_F(WordCheckerTest, WordCheckingUnderThreshold) {
    std::vector<std::string> few_bad = { "hello", "gay", "world" };
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
