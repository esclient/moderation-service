#pragma once

#include "moderation.pb.h"
#include <chrono>
#include <string>
#include "repository/repository.hpp"

class IModerationRepository {
    public:
    virtual ~IModerationRepository() = default;
    virtual bool SaveModerationResult(const ModerationRecord& result) = 0;
};