#pragma once

#include <string>
#include <vector>

namespace bbae {

struct ColorGroupConfig {
    std::string key;
    std::string label;
    std::string colorHex;
    std::vector<std::string> fields;
};

struct GlobalFieldConfig {
    std::string key;
    std::string label;
    std::string placeholder;
    std::string description;
};

struct RoleConfig {
    std::string id;
    std::string name;
    std::string description;
    std::string mode;
    bool enableCombinationSolver = true;
    std::vector<GlobalFieldConfig> globalFields;
    std::vector<ColorGroupConfig> inputGroups;
};

} // namespace bbae
