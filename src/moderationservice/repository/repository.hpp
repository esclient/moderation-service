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
    
    std::vector<ModerationRecord> GetModerationRecordsByType(moderation::ObjectType object_type, int limit = 100);
    std::optional<ModerationRecord> GetModerationRecord(int64_t object_id);
    std::vector<ModerationRecord> GetAllModerationRecords(int limit = 100);

    private:
    std::string database_url_;
    std::unique_ptr<pqxx::connection> db_connection_;

    void ParseDatabaseUrl(const std::string& database_url);
};