#include "core/CombinationSolver.h"
#include "core/PriceCalculator.h"
#include "data/ItemDatabase.h"

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

    std::vector<bbae::ColorInputValues> inputs;
    bbae::ColorInputValues purple;
    purple.key = "purple";
    purple.label = "紫色";
    purple.totalCount = 2;
    inputs.push_back(purple);

    bbae::ColorInputValues gold;
    gold.key = "gold";
    gold.label = "橙色";
    gold.totalCount = 1;
    inputs.push_back(gold);

    bbae::CombinationSolver solver(database);
    const auto combinations = solver.solve(role, inputs, 4);
    bbae::PriceCalculator calculator(database);
    const auto estimate = calculator.calculate(role, inputs, combinations);

    assert(estimate.valid);
    assert(estimate.referencePrice > 0.0);
    assert(estimate.highestPrice >= estimate.lowestPrice);
    return 0;
}
