#pragma once

#include "unicode/unistr.h"
#include <iostream>
#include <string>
#include <unordered_map>

namespace LeetspeakNormalization {

std::string LeetspeakNormalization(const std::string& textN, bool isCyrillic);
bool IsPrimarilyCyrillic(const std::string& textN);

} // namespace LeetspeakNormalization
