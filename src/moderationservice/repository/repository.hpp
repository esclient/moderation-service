#pragma once

#include <memory>
#include <vector>
#include <chrono>
#include <string>
#include <optional>
#include "config/config.hpp"
#include <pqxx/pqxx>
#include "moderation.pb.h"


struct ModerationRecord {
    int64_t object_id;
    moderation::ObjectType object_type;
    std::string text;
    bool is_flagged;
    std::chrono::system_clock::time_point moderated_at;
    std::string reason;
};


class ModerationRepository {
    public:
    explicit ModerationRepository(const std::string& database_url);
    ~ModerationRepository();
    bool SaveModerationResult(const ModerationRecord& result); 

    private:
    std::string database_url_;
    std::unique_ptr<pqxx::connection> db_connection_;

};