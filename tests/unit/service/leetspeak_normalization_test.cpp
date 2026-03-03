#include "service/leetspeak_normalization.hpp"
#include <gtest/gtest.h>
#include <string>

TEST(LeetspeakNormalization, LatinDigitToLetter) {
    std::string out = LeetspeakNormalization::LeetspeakNormalization("0", false);
    EXPECT_EQ(out, "o");
}

TEST(LeetspeakNormalization, IsPrimarilyCyrillic_CyrillicWin) {
    EXPECT_TRUE(LeetspeakNormalization::IsPrimarilyCyrillic("привет привет hello"));
}

TEST(LeetspeakNormalization, IsPrimarilyCyrillic_LatinWin) {
    EXPECT_FALSE(LeetspeakNormalization::IsPrimarilyCyrillic("hello hello привет"));
}

TEST(LeetspeakNormalization, EmptyInputReturnsEmpty) {
    std::string outLatin = LeetspeakNormalization::LeetspeakNormalization("", false);
    std::string outCyrillic = LeetspeakNormalization::LeetspeakNormalization("", true);

    EXPECT_EQ(outLatin, "");
    EXPECT_EQ(outCyrillic, "");
}

TEST(LeetspeakNormalization, CyrillicHomoglyphMapping) {
    std::string out = LeetspeakNormalization::LeetspeakNormalization("a", true);
    // Latin 'a' (U+0061) mapped to Cyrillic 'а' (U+0430)
    EXPECT_EQ(out, "а");
}

TEST(LeetspeakNormalization, CyrillicLeetDigitMapping) {
    std::string out = LeetspeakNormalization::LeetspeakNormalization("0", true);
    // '0' mapped to Cyrillic 'о'
    EXPECT_EQ(out, "о");
}

TEST(LeetspeakNormalization, CyrillicNoMappingKeepsCharacter) {
    std::string out = LeetspeakNormalization::LeetspeakNormalization("?", true);
    EXPECT_EQ(out, "?");
}

TEST(LeetspeakNormalization, SurrogatePairIsHandledCorrectly) {
    std::string input = "a😊b";
    std::string out = LeetspeakNormalization::LeetspeakNormalization(input, false);
    EXPECT_EQ(out, input);
}

TEST(LeetspeakNormalization, IsPrimarilyCyrillic_EqualCountsReturnsFalse) {
    EXPECT_FALSE(LeetspeakNormalization::IsPrimarilyCyrillic("aа"));
}

TEST(LeetspeakNormalization, IsPrimarilyCyrillic_NoLettersReturnsFalse) {
    EXPECT_FALSE(LeetspeakNormalization::IsPrimarilyCyrillic("1234!@#$"));
}

TEST(LeetspeakNormalization, IsPrimarilyCyrillic_SurrogatePairIgnoredForCounts) {
    std::string text = "привет😊hello";
    EXPECT_TRUE(LeetspeakNormalization::IsPrimarilyCyrillic(text));
}
