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