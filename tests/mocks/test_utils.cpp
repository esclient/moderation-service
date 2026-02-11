#include "tests/mocks/test_utils.hpp"

namespace test_utils {

ModerationRecord MakeRecord(int64_t object_id, const std::string& text, bool is_flagged,
                        const std::string& reason, moderation::ObjectType type){

    ModerationRecord record;
    record.object_id = object_id;
    record.object_type = type;
    record.text = text;
    record.is_flagged = is_flagged;
    record.moderated_at = std::chrono::system_clock::now();
    record.reason = reason;

    return record;
}

moderation::ModerateObjectRequest MakeRequest(int64_t id, const std::string& text,
                                            moderation::ObjectType type){
    moderation::ModerateObjectRequest request;
    request.set_id(id);
    request.set_text(text);
    request.set_type(type);

    return request;
}

moderation::ModerateObjectResponse MakeResponse(bool success){
    moderation::ModerateObjectResponse response;
    response.set_success(success);
    
    return success;
}

} //namespace test_utils