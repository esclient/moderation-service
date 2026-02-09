#pragma once

#include "unicode/normalizer2.h"
#include <iostream>
#include <string>
#include <unicode/unistr.h>

namespace TextNormalization {

std::string TextNormalization(const std::string& textN);
std::string WhitespaceNormalization(const std::string& textN);
std::string RepetitionNormalization(const std::string& textN);
std::string InvisibleCharacterNormalization(const std::string& textN);
} // namespace TextNormalization
