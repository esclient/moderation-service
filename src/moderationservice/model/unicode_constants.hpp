#pragma once

#include <unicode/unistr.h>

namespace unicode_constants {

// Zero-width and invisible format characters
constexpr UChar32 ZERO_WIDTH_SPACE = 0x200B;
constexpr UChar32 ZERO_WIDTH_NON_JOINER = 0x200C;
constexpr UChar32 ZERO_WIDTH_JOINER = 0x200D;
constexpr UChar32 LEFT_TO_RIGHT_MARK = 0x200E;
constexpr UChar32 RIGHT_TO_LEFT_MARK = 0x200F;
constexpr UChar32 ZERO_WIDTH_NO_BREAK_SPACE = 0xFEFF;
constexpr UChar32 WORD_JOINER = 0x2060;
constexpr UChar32 FUNCTION_APPLICATION = 0x2061;
constexpr UChar32 INVISIBLE_TIMES = 0x2062;
constexpr UChar32 INVISIBLE_SEPARATOR = 0x2063;
constexpr UChar32 INVISIBLE_PLUS = 0x2064;

// Typographic format characters
constexpr UChar32 SOFT_HYPHEN = 0x00AD;
constexpr UChar32 COMBINING_GRAPHEME_JOINER = 0x034F;
constexpr UChar32 ARABIC_LETTER_MARK = 0x061C;
constexpr UChar32 MONGOLIAN_VOWEL_SEPARATOR = 0x180E;
constexpr UChar32 LINE_SEPARATOR = 0x2028;
constexpr UChar32 PARAGRAPH_SEPARATOR = 0x2029;

// Bidirectional control characters [0x202A–0x202E]
constexpr UChar32 BIDI_MARKERS_START = 0x202A;
constexpr UChar32 BIDI_MARKERS_END = 0x202E;

} // namespace unicode_constants
