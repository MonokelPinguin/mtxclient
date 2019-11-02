#pragma once

#include <nlohmann/json_fwd.hpp>

namespace mtx {
namespace responses {

struct JoinedGroups
{
        std::vector<std::string> groups;
};

void
from_json(const nlohmann::json &obj, JoinedGroups &res);

struct GroupProfile
{
        std::string name;
        std::string avatar_url;
};

void
from_json(const nlohmann::json &obj, GroupProfile &res);
}
}
