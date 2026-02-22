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
