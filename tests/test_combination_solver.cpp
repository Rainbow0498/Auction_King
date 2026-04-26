#include "core/CombinationSolver.h"
#include "data/ItemDatabase.h"

#include <algorithm>
#include <cassert>
#include <filesystem>

int main()
{
    bbae::ItemDatabase database;
    database.loadFromFile(std::filesystem::current_path() / "data" / "items.json");

    bbae::RoleConfig role;
    role.id = "victor";
    role.name = "维克托";
    role.mode = "high_value_only";
    role.enableCombinationSolver = true;

    std::vector<bbae::ColorInputValues> inputs;
    bbae::ColorInputValues purple;
    purple.key = "purple";
    purple.label = "紫色";
    purple.unitGridPrice = 2312.0;
    purple.totalGrid = 2.0;
    inputs.push_back(purple);

    bbae::ColorInputValues gold;
    gold.key = "gold";
    gold.label = "橙色";
    gold.totalCount = 2;
    inputs.push_back(gold);

    bbae::CombinationSolver solver(database);
    const auto result = solver.solve(role, inputs, 8);
    assert(result.valid);
    assert(!result.countOptions[0].counts.empty());
    assert(std::find(result.countOptions[0].counts.begin(), result.countOptions[0].counts.end(), 1) !=
           result.countOptions[0].counts.end());
    assert(std::find(result.countOptions[2].counts.begin(), result.countOptions[2].counts.end(), 5) !=
           result.countOptions[2].counts.end());
    return 0;
}
