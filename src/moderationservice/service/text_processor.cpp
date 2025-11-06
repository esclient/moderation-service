#include "text_processor.hpp"

namespace TextProcessor {

    bool TextProcessing(const std::string& textN)
    {
        std::string text = TextNormalization::TextNormalization(textN);
        text = TextNormalization::InvisibleCharacterNormalization(text);
        text = TextNormalization::WhitespaceNormalization(text);
        text = TextNormalization::RepetitionNormalization(text);

        bool AlphabetOrientation = LeetspeakNormalization::IsPrimarilyCyrillic(text);
        text = LeetspeakNormalization::LeetspeakNormalization(text, AlphabetOrientation);

        std::vector<std::string> wordTokens = WordChecker::Tokenize(text);
        bool bannedWordsFound = WordChecker::WordChecking(wordTokens);

        return bannedWordsFound;  
    }

} //namespace TextProcessor 