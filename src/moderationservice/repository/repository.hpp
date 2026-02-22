#pragma once

#include "config/config.hpp"
#include "moderation.pb.h"
#include "repository/imoderation_repository.hpp"
#include "repository/moderation_record.hpp"
#include <chrono>
#include <memory>
#include <optional>
#include <pqxx/pqxx>
#include <string>
#include <vector>

class ModerationRepository : public IModerationRepository {
  public:
    explicit ModerationRepository(const std::string& database_url);
    ~ModerationRepository() override;
    bool SaveModerationResult(const ModerationRecord& result) override;

  private:
    std::string database_url_;
    std::unique_ptr<pqxx::connection> db_connection_;
};
