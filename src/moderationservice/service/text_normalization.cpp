#include "text_normalization.hpp"

namespace TextNormalization {

    std::string TextNormalization(const std::string& textN)
    {
        icu::UnicodeString text = icu::UnicodeString::fromUTF8(textN);

        UErrorCode status = U_ZERO_ERROR;
        const icu::Normalizer2* normalizer = icu::Normalizer2::getNFCInstance(status);

        if (U_FAILURE(status))
        {
            std::cerr << "Error getting normalizer" << std::endl;
            return "";
        }

        if (text.length() == 0) {
            text = icu::UnicodeString(textN.c_str(), textN.length(), "UTF-8");
        }

        icu::UnicodeString normalized = normalizer->normalize(text, status);

        if (U_FAILURE(status))
        {
            std::cerr << "Error normalizing" << std::endl;
            return "";
        }

        icu::UnicodeString result_ = normalized.toLower();
        result_.trim();

        std::string result;
        
        result_.toUTF8String(result);

        return result;
    }

    std::string WhitespaceNormalization(const std::string& textN)
    {
        icu::UnicodeString text = icu::UnicodeString::fromUTF8(textN);
        icu::UnicodeString result;

        if (text.length() == 0) {
            text = icu::UnicodeString(textN.c_str(), textN.length(), "UTF-8");
        }

        bool prev_space = false;

        for (int32_t i = 0; i < text.length(); i++)
        {
            UChar32 c = text.char32At(i);
            
                    if (u_isWhitespace(c)) {
                        if (!prev_space && result.length() > 0) {
                            result.append((UChar32)' ');
                            prev_space = true;
                            
                        }
                    }
                    else {
                        result.append(c);
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

    std::string RepetitionNormalization(const std::string& textN)
    {
        icu::UnicodeString text = icu::UnicodeString::fromUTF8(textN);
        icu::UnicodeString result;

        UChar32 prevChar = 0;
        int repeatCount = 0;

        if (text.length() == 0) {
            text = icu::UnicodeString(textN.c_str(), textN.length(), "UTF-8");
        }

        for (int32_t i = 0; i < text.length(); i++)
        {
            UChar32 c = text.char32At(i);

            if (c == prevChar)
            {
                repeatCount++;

                if (repeatCount < 2)
                {
                    result.append(c);
                }
            }
            else
            {
                result.append(c);
                prevChar = c;
                repeatCount = 0;
            }

            if (U16_IS_LEAD(text.charAt(i))) i++;

        }

        std::string output;
        result.toUTF8String(output);
        return output;
    }

    std::string InvisibleCharacterNormalization(const std::string& textN)
    {
        icu::UnicodeString text = icu::UnicodeString::fromUTF8(textN);
        if (text.length() == 0) {
            text = icu::UnicodeString(textN.c_str(), textN.length(), "UTF-8");
        }
        icu::UnicodeString result;

        for (int32_t i = 0; i < text.length(); i++) {
            UChar32 c = text.char32At(i);

            // Skip zero-width and format characters
            if (c == 0x200B || // Zero Width Space
                c == 0x200C || // Zero Width Non-Joiner
                c == 0x200D || // Zero Width Joiner
                c == 0x200E || // Left-to-Right Mark
                c == 0x200F || // Right-to-Left Mark
                c == 0xFEFF || // Zero Width No-Break Space (BOM)
                c == 0x2060 || // Word Joiner
                c == 0x2061 || // Function Application
                c == 0x2062 || // Invisible Times
                c == 0x2063 || // Invisible Separator
                c == 0x2064 || // Invisible Plus
                c == 0x00AD || // Soft Hyphen
                c == 0x034F || // Combining Grapheme Joiner
                c == 0x061C || // Arabic Letter Mark
                c == 0x180E || // Mongolian Vowel Separator
                c == 0x2028 || // Line Separator
                c == 0x2029 || // Paragraph Separator
                (c >= 0x202A && c <= 0x202E)) { // Bidirectional markers
                continue;
            }

            result.append(c);

            if (U16_IS_LEAD(text.charAt(i))) i++;
        }

        std::string output;
        result.toUTF8String(output);
        return output;
    }

} //namespace TextNormalization
