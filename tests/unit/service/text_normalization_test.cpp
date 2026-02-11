#include "moderationservice/service/text_normalization.hpp"
#include <gtest/gtest.h>

TEST(TextNormalization, EmptyString) {
    EXPECT_TRUE(TextNormalization::TextNormalization("").empty());
}

TEST(TextNormalization, Lowercase){
    EXPECT_EQ(TextNormalization::TextNormalization("HELLO"));
}

TEST(WhitespaceNormalization, CollapseSpaces) {
    EXPECT_EQ(TextNormalization::WhitespaceNormalization("a  b"), "a b");
}

TEST(WhitespaceNormalization, SingleSpaceUnchanged) {
    EXPECT_EQ(TextNormalization::WhitespaceNormalization("a b"), "a b");
}

TEST(RepetitionNormalization, ReduceRepeats) {
    EXPECT_EQ(TextNormalization::RepetitionNormalization("aaa"), "a");
}

TEST(RepetitionNormalization, TwoDifferentChars) {
    EXPECT_EQ(TextNormalization::RepetitionNormalization("aab"), "ab");
}

TEST(InvisibleCharacterNormalization, StripZeroWidthSpace) {
    std::string with_zwsp = "a";
    with_zwsp += static_cast<char>(0xE2);
    with_zwsp += static_cast<char>(0x80);
    with_zwsp += static_cast<char>(0x8B);
    with_zwsp += "b";
    std::string out = TextNormalization::InvisibleCharacterNormalization(with_zwsp);
    EXPECT_EQ(out, "ab");
}