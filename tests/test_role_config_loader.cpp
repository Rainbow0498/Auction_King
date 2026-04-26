#include "data/RoleConfigLoader.h"

#include <cassert>
#include <filesystem>

int main()
{
    const auto roles = bbae::RoleConfigLoader().loadFromFile(std::filesystem::current_path() / "data" / "roles.json");
    assert(roles.size() == 3);
    assert(roles[0].id == "victor");
    assert(roles[0].globalFields.size() == 1);
    assert(roles[0].inputGroups.size() == 2);
    assert(roles[1].id == "ahmed");
    assert(roles[2].id == "raven");
    return 0;
}
