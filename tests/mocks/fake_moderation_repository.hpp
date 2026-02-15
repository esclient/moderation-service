#pragma once

#include "repository/imoderation_repository.hpp"
#include "repository/moderation_record.hpp"
#include <vector>

class FakeModerationRepository : public IModerationRepository {
  public:
    bool SaveModerationResult(const ModerationRecord& result);
    const std::vector<ModerationRecord>& GetStoredRecords() const { return stored_; }
    void Clear() { stored_.clear(); }
    void SetSaveResult(bool value) { save_result_ = value; }

  private:
    std::vector<ModerationRecord> stored_;
    bool save_result_ = true;
};