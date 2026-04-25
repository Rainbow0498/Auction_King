#pragma once

#include <string>

namespace bbae {

struct Role {
    std::string id;
    std::string name;
    std::string description;
    std::string inputProfile;
    bool enabled = true;
};

} // namespace bbae

