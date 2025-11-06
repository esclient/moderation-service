#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include "unicode/unistr.h"
#include <tsl/htrie_set.h>
#include <tsl/htrie_map.h>


namespace TextProcessingConstants{
    namespace LeetspeakMaps {

        extern const std::unordered_map<UChar32, UChar32> leetMapLatin;
        extern const std::unordered_map<UChar32, UChar32> leetMapCyrillic;
        extern const std::unordered_map<UChar32, UChar32> homoglyphMap;

    } //namespace LeetspeakMaps
    namespace HashTrieMaps {

        extern tsl::htrie_set<char> forbiddenWords;
        void InitializeForbiddenWords();

    } //namespace HashTrieMaps
} //namespace TextProcessingConstants
