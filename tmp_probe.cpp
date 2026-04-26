#include "core/CombinationSolver.h"
#include "data/ItemDatabase.h"
#include <filesystem>
#include <iostream>
int main(){
  bbae::ItemDatabase db; db.loadFromFile(std::filesystem::current_path()/"data"/"items.json");
  bbae::RoleConfig role; role.id="victor"; role.mode="high_value_only"; role.name="维克托";
  bbae::ColorInputValues purple; purple.key="purple"; purple.label="紫色"; purple.unitGridPrice=2312.0; purple.totalGrid=2.0;
  bbae::ColorInputValues gold; gold.key="gold"; gold.label="橙色"; gold.totalCount=2;
  std::vector<bbae::ColorInputValues> inputs{purple,gold};
  bbae::CombinationSolver solver(db);
  auto result = solver.solve(role, inputs, 8);
  std::cout << "valid=" << result.valid << " count=" << result.legalCombinationCount << " message=" << result.message << "\n";
  for (const auto& opt : result.countOptions){ std::cout << opt.key << ':'; for(int c : opt.counts) std::cout << c << ','; std::cout << "\n"; }
}
