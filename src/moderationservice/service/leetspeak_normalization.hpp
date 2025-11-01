#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include "unicode/unistr.h"

namespace LeetspeakNormalization {

    std::string LeetspeakNormalization(const std::string& textN, bool isCyrillic);
    bool IsPrimarilyCyrillic(const std::string& textN);

} //namespace LeetspeakNormalization
