#include "data/RoleDatabase.h"

#include "data/ConfigLoader.h"

namespace bbae {

void RoleDatabase::loadFromFile(const std::filesystem::path& path)
{
    const auto json = ConfigLoader::loadJson(path);
    roles_.clear();

    for (const auto& entry : json) {
        Role role;
        role.id = entry.value("id", "");
        role.name = entry.value("name", "");
        role.description = entry.value("description", "");
        role.inputProfile = entry.value("input_profile", "");
        role.enabled = entry.value("enabled", true);
        roles_.push_back(std::move(role));
    }
}

const std::vector<Role>& RoleDatabase::roles() const
{
    return roles_;
}

} // namespace bbae

