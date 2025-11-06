#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unicode/unistr.h>
#include <unicode/uchar.h> 
#include <tsl/htrie_set.h>
#include <tsl/htrie_map.h>

namespace WordChecker {

    std::vector<std::string> Tokenize(const std::string& textN);
    bool WordChecking(const std::vector < std::string> &textN);

} //namespace WordChecker
