#include "mocks/fake_moderation_repository.hpp"

bool FakeModerationRepository::SaveModerationResult(const ModerationRecord& result) {
    if (save_result_) {
        stored_.push_back(result);
    }
    return save_result_;
}
