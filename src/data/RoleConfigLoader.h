#pragma once

#include "core/RoleConfig.h"

#include <filesystem>
#include <vector>

namespace bbae {

class RoleConfigLoader {
public:
    std::vector<RoleConfig> loadFromFile(const std::filesystem::path& path) const;
};

} // namespace bbae

