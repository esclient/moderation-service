#include "constants.hpp"

namespace TextProcessingConstants
{
    namespace LeetspeakMaps {
        const std::unordered_map<UChar32, UChar32> leetMapLatin = {
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
                    { U'>', U'd' }
        };
        const std::unordered_map<UChar32, UChar32> leetMapCyrillic = {
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
        const std::unordered_map<UChar32, UChar32> homoglyphMap = {

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
    }
    namespace HashTrieMaps {
        tsl::htrie_set<char> forbiddenWords;

        void InitializeForbiddenWords() {
            forbiddenWords.insert("gay");
            forbiddenWords.insert("еблан");
            forbiddenWords.insert("шутка");
        }
    }
} //namespace TextProcessingConstants
