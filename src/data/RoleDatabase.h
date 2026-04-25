#pragma once

#include "core/Role.h"

#include <filesystem>
#include <vector>

namespace bbae {

class RoleDatabase {
public:
    void loadFromFile(const std::filesystem::path& path);
    const std::vector<Role>& roles() const;

private:
    std::vector<Role> roles_;
};

} // namespace bbae

