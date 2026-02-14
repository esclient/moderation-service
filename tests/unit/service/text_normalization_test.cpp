#include "service/text_normalization.hpp"
#include <gtest/gtest.h>

TEST(TextNormalization, EmptyString) {
    EXPECT_TRUE(TextNormalization::TextNormalization("").empty());
}

TEST(TextNormalization, Lowercase){
    EXPECT_EQ(TextNormalization::TextNormalization("HELLO"), "hello");
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


TEST(RepetitionNormalization, ReduceSpamPatterns) {
    EXPECT_EQ(TextNormalization::RepetitionNormalization("loooook"), "lok");
    EXPECT_EQ(TextNormalization::RepetitionNormalization("coooool"), "col");
    EXPECT_EQ(TextNormalization::RepetitionNormalization("heeeeey"), "hey");
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

TEST(TextNormalization, Trim) {
    EXPECT_EQ(TextNormalization::TextNormalization("  hello  "), "hello");
    EXPECT_EQ(TextNormalization::TextNormalization("\thello\t"), "hello");
    EXPECT_EQ(TextNormalization::TextNormalization("  hello world  "), "hello world");
}

TEST(TextNormalization, Unicode) {
    EXPECT_EQ(TextNormalization::TextNormalization("HELLO"), "hello");
    EXPECT_EQ(TextNormalization::TextNormalization("Café"), "café");
    EXPECT_EQ(TextNormalization::TextNormalization("МОСКВА"), "москва");
}

TEST(RepetitionNormalization, SingleChar) {
    EXPECT_EQ(TextNormalization::RepetitionNormalization("a"), "a");
    EXPECT_EQ(TextNormalization::RepetitionNormalization("x"), "x");
}


TEST(RepetitionNormalization, NoRepeatsStaysUnchanged) {
    EXPECT_EQ(TextNormalization::RepetitionNormalization("abc"), "abc");
    EXPECT_EQ(TextNormalization::RepetitionNormalization("123"), "123");
    EXPECT_EQ(TextNormalization::RepetitionNormalization("abcdef"), "abcdef");
}

TEST(WhitespaceNormalization, MultipleSpaceTypes) {
    EXPECT_EQ(TextNormalization::WhitespaceNormalization("hello\t\tworld"), "hello world");
    EXPECT_EQ(TextNormalization::WhitespaceNormalization("hello\n\nworld"), "hello world");
    EXPECT_EQ(TextNormalization::WhitespaceNormalization("hello \t \n world"), "hello world");
}

TEST(InvisibleCharacterNormalization, MultipleInvisibleChars) {
    // Zero-width space (U+200B)
    std::string text1 = "hello\u200Bworld";
    EXPECT_EQ(TextNormalization::InvisibleCharacterNormalization(text1), "helloworld");
    
    // Zero-width non-joiner (U+200C)
    std::string text2 = "test\u200Ctext";
    EXPECT_EQ(TextNormalization::InvisibleCharacterNormalization(text2), "testtext");
    
    // Soft hyphen (U+00AD)
    std::string text3 = "soft\u00ADhyphen";
    EXPECT_EQ(TextNormalization::InvisibleCharacterNormalization(text3), "softhyphen");
}

TEST(InvisibleCharacterNormalization, NoInvisibleChars) {
    EXPECT_EQ(TextNormalization::InvisibleCharacterNormalization("hello"), "hello");
    EXPECT_EQ(TextNormalization::InvisibleCharacterNormalization("test 123"), "test 123");
}