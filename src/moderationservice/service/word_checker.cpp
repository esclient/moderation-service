#include "service/word_checker.hpp"
#include "model/constants.hpp"
#include <string>
#include <vector>

namespace WordChecker {

const auto& forbiddenWords = TextProcessingConstants::HashTrieMaps::forbiddenWords;

std::vector<std::string> Tokenize(const std::string& textN) {
    std::vector<std::string> tokens;
    icu::UnicodeString text = icu::UnicodeString::fromUTF8(textN);

    std::string currentWord;

    for (int32_t i = 0; i < text.length(); i++) {
        if (UChar32 character = text.char32At(i); u_isalnum(character)) {
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
    unsigned int forbiddenWordCount = std::count_if(textN.begin(), textN.end(), [](const std::string& word) {
        return forbiddenWords.find(word) != forbiddenWords.end();
    });

    return (forbiddenWordCount > TextProcessingConstants::Thresholds::FORBIDDEN_WORD_THRESHOLD);
}

} // namespace WordChecker
