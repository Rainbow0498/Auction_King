#include "data/RoleConfigLoader.h"

#include "data/ConfigLoader.h"

namespace bbae {

std::vector<RoleConfig> RoleConfigLoader::loadFromFile(const std::filesystem::path& path) const
{
    const auto json = ConfigLoader::loadJson(path);
    std::vector<RoleConfig> roles;
    roles.reserve(json.size());

    for (const auto& entry : json) {
        RoleConfig role;
        role.id = entry.value("id", "");
        role.name = entry.value("name", "");
        role.description = entry.value("description", "");
        role.mode = entry.value("mode", "");
        role.enableCombinationSolver = entry.value("enable_combination_solver", true);

        try {
            for (const auto& fieldEntry : entry.at("global_fields")) {
                GlobalFieldConfig field;
                field.key = fieldEntry.value("key", "");
                field.label = fieldEntry.value("label", "");
                field.placeholder = fieldEntry.value("placeholder", "");
                field.description = fieldEntry.value("description", "");
                role.globalFields.push_back(std::move(field));
            }
        } catch (...) {
        }

        for (const auto& groupEntry : entry.at("input_groups")) {
            ColorGroupConfig group;
            group.key = groupEntry.value("key", "");
            group.label = groupEntry.value("label", "");
            group.colorHex = groupEntry.value("color_hex", "#888888");

            for (const auto& fieldEntry : groupEntry.at("fields")) {
                group.fields.push_back(fieldEntry.get<std::string>(""));
            }

            role.inputGroups.push_back(std::move(group));
        }

        roles.push_back(std::move(role));
    }

    return roles;
}

} // namespace bbae
