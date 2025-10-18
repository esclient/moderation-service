#pragma once

// Minimal fallback tinylog.hpp to avoid hard dependency during development.
// If you have a proper tinylog library, set TINYLOG_ROOT or install it and
// remove this file.

#include <iostream>
#include <string>

namespace tinylog {

enum level { TRACE, DEBUG, INFO, WARN, ERROR };

inline void log(level lvl, const std::string &msg) {
    switch (lvl) {
        case TRACE: std::cout << "TRACE: "; break;
        case DEBUG: std::cout << "DEBUG: "; break;
        case INFO:  std::cout << "INFO: ";  break;
        case WARN:  std::cout << "WARN: ";  break;
        case ERROR: std::cout << "ERROR: "; break;
    }
    std::cout << msg << std::endl;
}

} // namespace tinylog
