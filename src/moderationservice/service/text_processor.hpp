#pragma once

#include "moderationservice/service/leetspeak_normalization.hpp"
#include "moderationservice/service/text_normalization.hpp"
#include "moderationservice/service/word_checker.hpp"
#include <string>

namespace TextProcessor {

bool TextProcessing(const std::string& textN);

} // namespace TextProcessor
