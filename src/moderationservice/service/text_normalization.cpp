#include "service/text_normalization.hpp"
#include "model/unicode_constants.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace TextNormalization {

std::string TextNormalization(const std::string& textN) {
    icu::UnicodeString text = icu::UnicodeString::fromUTF8(textN);

    UErrorCode status = U_ZERO_ERROR;
    const icu::Normalizer2* normalizer = icu::Normalizer2::getNFCInstance(status);

    if (U_FAILURE(status) != 0) {
        std::cerr << "Error getting normalizer" << "\n";
        return "";
    }

    icu::UnicodeString normalized = normalizer->normalize(text, status);

    if (U_FAILURE(status) != 0) {
        std::cerr << "Error normalizing" << "\n";
        return "";
    }

    icu::UnicodeString result_ = normalized.toLower();
    result_.trim();

    std::string result;

    result_.toUTF8String(result);

    return result;
}

std::string WhitespaceNormalization(const std::string& textN) {
    icu::UnicodeString text = icu::UnicodeString::fromUTF8(textN);
    icu::UnicodeString result;

    bool prev_space = false;

    for (int32_t i = 0; i < text.length(); i++) {
        UChar32 character = text.char32At(i);

        if (u_isWhitespace(character)) {
            if (!prev_space && result.length() > 0) {
                result.append((UChar32)' ');
                prev_space = true;
            }
        } else {
            result.append(character);
            prev_space = false;
        }
        if (U16_IS_LEAD(text.charAt(i))) {
            i++;
        }
    }
    std::string output;
    result.toUTF8String(output);
    return output;
}

std::string RepetitionNormalization(const std::string& textN) {
    icu::UnicodeString text = icu::UnicodeString::fromUTF8(textN);
    icu::UnicodeString result;

    UChar32 prevChar = 0;

    for (int32_t i = 0; i < text.length(); i++) {
        UChar32 character = text.char32At(i);

        if (character != prevChar) {
            result.append(character);
            prevChar = character;
        }

        if (U16_IS_LEAD(text.charAt(i))) {
            i++;
        }
    }

    std::string output;
    result.toUTF8String(output);
    return output;
}

std::string InvisibleCharacterNormalization(const std::string& textN) {
    icu::UnicodeString text = icu::UnicodeString::fromUTF8(textN);
   
    icu::UnicodeString result;

    for (int32_t i = 0; i < text.length(); i++) {
        UChar32 character = text.char32At(i);

        // Skip zero-width and format characters
        if (character == unicode_constants::ZERO_WIDTH_SPACE ||
            character == unicode_constants::ZERO_WIDTH_NON_JOINER ||
            character == unicode_constants::ZERO_WIDTH_JOINER ||
            character == unicode_constants::LEFT_TO_RIGHT_MARK ||
            character == unicode_constants::RIGHT_TO_LEFT_MARK ||
            character == unicode_constants::ZERO_WIDTH_NO_BREAK_SPACE ||
            character == unicode_constants::WORD_JOINER ||
            character == unicode_constants::FUNCTION_APPLICATION ||
            character == unicode_constants::INVISIBLE_TIMES ||
            character == unicode_constants::INVISIBLE_SEPARATOR ||
            character == unicode_constants::INVISIBLE_PLUS ||
            character == unicode_constants::SOFT_HYPHEN ||
            character == unicode_constants::COMBINING_GRAPHEME_JOINER ||
            character == unicode_constants::ARABIC_LETTER_MARK ||
            character == unicode_constants::MONGOLIAN_VOWEL_SEPARATOR ||
            character == unicode_constants::LINE_SEPARATOR ||
            character == unicode_constants::PARAGRAPH_SEPARATOR ||
            (character >= unicode_constants::BIDI_MARKERS_START &&
             character <= unicode_constants::BIDI_MARKERS_END)) {
            continue;
        }

        result.append(character);

        if (U16_IS_LEAD(text.charAt(i))) {
            i++;
        }
    }

    std::string output;
    result.toUTF8String(output);
    return output;
}

} // namespace TextNormalization
