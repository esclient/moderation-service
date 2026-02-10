#pragma once

#include "moderaitonservice/repository/repository.hpp"
#include <vector>
#include "include/repository/imoderation_repository.hpp"


class FakeModerationRepository : public IModerationRepository
{
    public:
    bool SaveModerationResult(const ModerationRecord& result);
    const std::vector<ModerationRecord>& GetStoredRecords() const {return stored_; }
    void Clear() {stored_.clear(); }
    void SetSaveResult(bool value) { save_result_ = value; }

    private:
    std::vector<ModerationRecord> stored_;
    bool save_result_ = true;
};