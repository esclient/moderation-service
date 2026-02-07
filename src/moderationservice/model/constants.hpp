#pragma once

#include "unicode/unistr.h"
#include <iostream>
#include <string>
#include <tsl/htrie_map.h>
#include <tsl/htrie_set.h>
#include <unordered_map>

namespace TextProcessingConstants {
namespace LeetspeakMaps {

extern const std::unordered_map<UChar32, UChar32> leetMapLatin;
extern const std::unordered_map<UChar32, UChar32> leetMapCyrillic;
extern const std::unordered_map<UChar32, UChar32> homoglyphMap;

} // namespace LeetspeakMaps
namespace HashTrieMaps {

extern tsl::htrie_set<char> forbiddenWords;
void InitializeForbiddenWords();

} // namespace HashTrieMaps
} // namespace TextProcessingConstants
