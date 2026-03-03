#include "service/leetspeak_normalization.hpp"
#include "model/constants.hpp"
#include "model/unicode_constants.hpp"
#include <string>

namespace LeetspeakNormalization {

const auto& latinMap = TextProcessingConstants::LeetspeakMaps::leetMapLatin;
const auto& cyrillicMap = TextProcessingConstants::LeetspeakMaps::leetMapCyrillic;
const auto& homoglyphMap = TextProcessingConstants::LeetspeakMaps::homoglyphMap;

std::string LeetspeakNormalization(const std::string& textN, bool isCyrillic) {
    icu::UnicodeString text = icu::UnicodeString::fromUTF8(textN);

    icu::UnicodeString result;

    for (int32_t i = 0; i < text.length(); i++) {
        UChar32 character = text.char32At(i);
        UChar32 normalized = character;

        if (isCyrillic) {
            auto homoglyphText = homoglyphMap.find(character);
            if (homoglyphText != homoglyphMap.end()) {
                normalized = homoglyphText->second;
            } else {
                auto leetedTextCyrillic = cyrillicMap.find(character);
                if (leetedTextCyrillic != cyrillicMap.end()) {
                    normalized = leetedTextCyrillic->second;
                }
            }
        } else {
            auto leetedText = latinMap.find(character);
            if (leetedText != latinMap.end()) {
                normalized = leetedText->second;
            }
        }

        result.append(normalized);
        if (U16_IS_LEAD(text.charAt(i))) {
            i++;
        }
    }

    std::string output;
    result.toUTF8String(output);
    return output;
}

bool IsPrimarilyCyrillic(const std::string& textN) {
    icu::UnicodeString text = icu::UnicodeString::fromUTF8(textN);

    int cyrillicCount = 0;
    int latinCount = 0;

    for (int32_t i = 0; i < text.length(); i++) {

        if (UChar32 character = text.char32At(i);
            character >= unicode_constants::CYRILLIC_BLOCK_START &&
            character <= unicode_constants::CYRILLIC_BLOCK_END) {
            cyrillicCount++;
        } else if ((character >= unicode_constants::LATIN_UPPERCASE_START &&
                    character <= unicode_constants::LATIN_UPPERCASE_END) ||
                   (character >= unicode_constants::LATIN_LOWERCASE_START &&
                    character <= unicode_constants::LATIN_LOWERCASE_END)) {
            latinCount++;
        }

        if (U16_IS_LEAD(text.charAt(i))) {
            i++;
        }
    }

    return cyrillicCount > latinCount;
}

} // namespace LeetspeakNormalization
