#include "leetspeak_normalization.hpp"

namespace LeetspeakNormalization {

    std::string LeetspeakNormalization(const std::string& textN, bool isCyrillic)
    {
        icu::UnicodeString text = icu::UnicodeString::fromUTF8(textN);
        if (text.length() == 0) {
            text = icu::UnicodeString(textN.c_str(), textN.length(), "UTF-8");
        }

        icu::UnicodeString result;

        static const std::unordered_map<UChar32, UChar32> leetMapLatin = {
                    { U'0', U'o' }, { U'!', U'i' },
                    { U'1', U'i' }, { U'%', U'o' },
                    { U'3', U'e' }, { U'^', U'a' },
                    { U'4', U'a' }, { U'[', U'c' },
                    { U'5', U's' }, { U'&', U'a' },
                    { U'6', U'g' }, { U'$', U's' },
                    { U'7', U't' }, { U'8', U'b' },
                    { U'9', U'g' }, { U'@', U'a' },
                    { U'*', U'a' }, { U'#', U'h' },
                    { U'+', U't' }, { U'|', U'i' },
                    { U'/', U'l' }, { U')', U'd' },
                    { U'~', U'n' }, { U'(', U'c' },
                    { U'{', U'c' }, { U'<', U'c' },
                    { U'>', U'd' },

        };
        static const std::unordered_map<UChar32, UChar32> leetMapCyrillic = {
                    { U'0', U'о' },
                    { U'1', U'і' },
                    { U'3', U'з' },
                    { U'4', U'ч' },
                    { U'5', U'ѕ' },
                    { U'6', U'б' },
                    { U'8', U'в' },
                    { U'9', U'я' },
                    { U'@', U'а' },
                    { U'$', U'з' },
                    { U'|', U'і' },
                    { U'/', U'г' }
        };
        static const std::unordered_map<UChar32, UChar32> homoglyphMap = {

                    { U'a', U'а' },
                    { U'e', U'е' },
                    { U'o', U'о' },
                    { U'p', U'р' },
                    { U'c', U'с' },
                    { U'y', U'у' },
                    { U'x', U'х' },
                    { U'k', U'к' },
                    { U'b', U'в' },
                    { U'h', U'н' },
                    { U'm', U'м' },
                    { U't', U'т' }
        };

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
                    auto leetedTextCyrillic = leetMapCyrillic.find(c);
                    if (leetedTextCyrillic != leetMapCyrillic.end())
                    {
                        normalized = leetedTextCyrillic->second;
                    }
                }
            }
            else {
                auto leetedText = leetMapLatin.find(c);
                if (leetedText != leetMapLatin.end()) {
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
