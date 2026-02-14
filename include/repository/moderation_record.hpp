#pragma once

#include "moderation.pb.h"
#include <chrono>
#include <string>

struct ModerationRecord {
    int64_t object_id;
    moderation::ObjectType object_type;
    std::string text;
    bool is_flagged;
    std::chrono::system_clock::time_point moderated_at;
    std::string reason;
};