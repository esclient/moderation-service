#pragma once

#include <chrono>
#include <string>
#include "repository/moderation_record.hpp"

class IModerationRepository {
    public:
    virtual ~IModerationRepository() = default;
    virtual bool SaveModerationResult(const ModerationRecord& result) = 0;
};