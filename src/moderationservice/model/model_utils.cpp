#include "model/model_utils.hpp"
#include "moderation.pb.h"
#include <string>

namespace moderation::utils {

std::string ObjectTypeToString(moderation::ObjectType type) {
    switch (type) {
    case moderation::OBJECT_TYPE_COMMENT_TEXT:
        return "COMMENT_TEXT";
    case moderation::OBJECT_TYPE_MOD_DESCRIPTION:
        return "MOD_DESCRIPTION";
    case moderation::OBJECT_TYPE_USER_NAME:
        return "USER_NAME";
    default:
        return "UNSPECIFIED";
    }
}
} // namespace moderation::utils
