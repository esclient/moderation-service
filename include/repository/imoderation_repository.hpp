#pragma once

#include "repository/moderation_record.hpp"
#include <chrono>
#include <string>

class IModerationRepository {
  public:
    virtual ~IModerationRepository() = default;
    virtual bool SaveModerationResult(const ModerationRecord& result) = 0;
};
