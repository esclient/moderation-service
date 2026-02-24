#include "service/word_checker.hpp"
#include "model/constants.hpp"
#include <string>
#include <vector>

namespace WordChecker {

const auto& forbiddenWords = TextProcessingConstants::HashTrieMaps::forbiddenWords;

std::vector<std::string> Tokenize(const std::string& textN) {
    std::vector<std::string> tokens;
    icu::UnicodeString text = icu::UnicodeString::fromUTF8(textN);
    if (text.length() == 0) {
        const auto length = static_cast<int32_t>(textN.length());
        text = icu::UnicodeString(textN.c_str(), length, "UTF-8");
    }
    std::string currentWord;

    for (int32_t i = 0; i < text.length(); i++) {
        UChar32 character = text.char32At(i);

        if (u_isalnum(character)) {
            icu::UnicodeString charStr(character);
            std::string utf8char;
            charStr.toUTF8String(utf8char);
            currentWord += utf8char;
        } else {
            if (!currentWord.empty()) {
                tokens.push_back(currentWord);
                currentWord.clear();
            }
        }

        if (U16_IS_LEAD(text[i]) && i + 1 < text.length()) {
            i++;
        }
    }

    if (!currentWord.empty()) {
        tokens.push_back(currentWord);
    }

    return tokens;
}
bool WordChecking(const std::vector<std::string>& textN) {
    unsigned int forbiddenWordCount = 0;

    for (const auto& token : textN) {
        if (forbiddenWords.find(token) != forbiddenWords.end()) {
            forbiddenWordCount++;
        }
    }

    return (forbiddenWordCount > TextProcessingConstants::Thresholds::FORBIDDEN_WORD_THRESHOLD);
}

} // namespace WordChecker
