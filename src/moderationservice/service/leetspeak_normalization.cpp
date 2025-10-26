#include "leetspeak_normalization.hpp"
#include "constants.hpp"

namespace LeetspeakNormalization {

    const auto& latinMap = TextProcessingConstants::LeetspeakMaps::leetMapLatin;
    const auto& cyrillicMap = TextProcessingConstants::LeetspeakMaps::leetMapCyrillic;
    const auto& homoglyphMap = TextProcessingConstants::LeetspeakMaps::homoglyphMap;

    std::string LeetspeakNormalization(const std::string& textN, bool isCyrillic)
    {
        icu::UnicodeString text = icu::UnicodeString::fromUTF8(textN);
        if (text.length() == 0) {
            text = icu::UnicodeString(textN.c_str(), textN.length(), "UTF-8");
        }

        icu::UnicodeString result;

        for (int32_t i = 0; i < text.length(); i++)
        {
            UChar32 c = text.char32At(i);
            UChar32 normalized = c;
            
            if (isCyrillic)
            {
                auto homoglyphText = homoglyphMap.find(c);
                if (homoglyphText != homoglyphMap.end())
                {
                    normalized = homoglyphText->second;
                }

                else {
                    auto leetedTextCyrillic = cyrillicMap.find(c);
                    if (leetedTextCyrillic != cyrillicMap.end())
                    {
                        normalized = leetedTextCyrillic->second;
                    }
                }
            }
            else {
                auto leetedText = latinMap.find(c);
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

    bool IsPrimarilyCyrillic(const std::string& textN)
    {
        icu::UnicodeString text = icu::UnicodeString::fromUTF8(textN);

        if (text.length() == 0) {
            text = icu::UnicodeString(textN.c_str(), textN.length(), "UTF-8");
        }

        int cyrillicCount = 0;
        int latinCount = 0;

        for (int32_t i = 0; i < text.length(); i++) {
            UChar32 c = text.char32At(i);

            // Cyrillic range: U+0400 to U+04FF
            if (c >= 0x0400 && c <= 0x04FF) {
                cyrillicCount++;
            }
            // Latin range
            else if ((c >= 0x0041 && c <= 0x005A) ||
                (c >= 0x0061 && c <= 0x007A)) {
                latinCount++;
            }

            if (U16_IS_LEAD(text.charAt(i))) i++;
        }

        return cyrillicCount > latinCount;
    }

} //namespace LeetspeakNormalization
