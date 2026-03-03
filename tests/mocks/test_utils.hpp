#pragma once

#include "moderation.pb.h"
#include "repository/moderation_record.hpp"
#include "repository/repository.hpp"
#include <chrono>
#include <string>

namespace test_utils {
ModerationRecord MakeRecord(int64_t object_id, const std::string& text, bool is_flagged,
                            const std::string& reason = "",
                            moderation::ObjectType type = moderation::OBJECT_TYPE_UNSPECIFIED);

moderation::ModerateObjectRequest
MakeRequest(int64_t request_id, const std::string& text,
            moderation::ObjectType type = moderation::OBJECT_TYPE_UNSPECIFIED);

moderation::ModerateObjectResponse MakeResponse(bool success);
} // namespace test_utils
