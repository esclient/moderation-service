#pragma once

#include "repository/moderation_record.hpp"
#include "config/config.hpp"
#include "repository/imoderation_repository.hpp"
#include "moderation.pb.h"
#include <chrono>
#include <memory>
#include <optional>
#include <pqxx/pqxx>
#include <string>
#include <vector>


class ModerationRepository : public IModerationRepository{
  public:
    explicit ModerationRepository(const std::string& database_url);
    ~ModerationRepository();
    bool SaveModerationResult(const ModerationRecord& result) override;

  private:
    std::string database_url_;
    std::unique_ptr<pqxx::connection> db_connection_;
};
