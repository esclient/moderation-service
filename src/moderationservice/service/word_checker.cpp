#include "word_checker.hpp"

namespace WordChecker {
    std::vector<std::string> Tokenize(const std::string& textN)
    {
        std::vector<std::string> tokens;
        icu::UnicodeString text = icu::UnicodeString::fromUTF8(textN);
        if (text.length() == 0) {
            text = icu::UnicodeString(textN.c_str(), textN.length(), "UTF-8");
        }
        std::string currentWord;

        for (int32_t i = 0; i < text.length(); i++)
        {
            UChar32 c = text.char32At(i);

            if (u_isalnum(c)) {
                icu::UnicodeString charStr(c);
                std::string utf8char;
                charStr.toUTF8String(utf8char);
                currentWord += utf8char;
            }
            else {
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

    bool TrieBuildingAndWordChecking(const std::vector < std::string> &textN)
    {
        tsl::htrie_set<char> words;

        unsigned int forbiddenWordCount = 0;

        words.insert("Gay");
        words.insert("еблан");
        words.insert("шутка");

        for (int i = 0; i < textN.size(); i++)
        {
            if (words.find(textN[i]) != words.end())
            {
                forbiddenWordCount++;
            }

        }

        return (forbiddenWordCount > 3);
    }

} //namespace WordChecker
