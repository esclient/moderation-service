#include "mocks/fake_moderation_repository.hpp"

bool FakeModerationRepository::SaveModerationResult(const ModerationRecord& result)
{
    stored_.push_back(result);
    return save_result_;
}