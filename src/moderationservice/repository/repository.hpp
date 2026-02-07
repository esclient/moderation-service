#pragma once

#include "config/config.hpp"
#include "moderation.pb.h" // NOLINT(build/include_subdir)
#include <chrono>
#include <memory>
#include <optional>
#include <pqxx/pqxx>
#include <string>
#include <vector>

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
