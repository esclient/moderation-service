#include "model/constants.hpp"
#include "service/text_processor.hpp"
#include <gtest/gtest.h>
#include <string>

class TextProcessorTest : public ::testing::Test {
    protected:
    void SetUp() override { TextProcessingConstants::HashTrieMaps::InitializeForbiddenWords(); }
};

TEST_F(TextProcessorTest, CleanTextReturnsFalse) {
    EXPECT_FALSE(TextProcessor::TextProcessing("clean normal text"));
}

TEST_F(TextProcessorTest, ManyForbiddenWordsReturnsTrue) {
    std::string bad;
    for (int i = 0; i < 5; ++i) 
    {
        bad += "gay ";
    }

    EXPECT_TRUE(TextProcessor::TextProcessing(bad));
}

TEST_F(TextProcessorTest, NumbersOnly) {
    EXPECT_FALSE(TextProcessor::TextProcessing("1234567890"));
}

TEST_F(TextProcessorTest, NormalizedSpamPattern) {
    EXPECT_TRUE(TextProcessor::TextProcessing("gayyyyyy gayyyyyy gayyyyyy gayyyyyy"));
}

TEST_F(TextProcessorTest, CaseSensitivity) {
    EXPECT_TRUE(TextProcessor::TextProcessing("gay gay gay gay"));
    EXPECT_TRUE(TextProcessor::TextProcessing("GAY GAY GAY GAY"));
    EXPECT_TRUE(TextProcessor::TextProcessing("Gay Gay Gay Gay"));
}

TEST_F(TextProcessorTest, SingleCharacter) {
    EXPECT_FALSE(TextProcessor::TextProcessing("a"));
    EXPECT_FALSE(TextProcessor::TextProcessing("x"));
}
