#pragma once

#include "core/ColorInput.h"
#include "core/RoleConfig.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace bbae {

class ItemDatabase;

struct CountOptions {
    std::string key;
    std::string label;
    std::vector<int> counts;
};

struct LegalCombination {
    std::map<std::string, int> counts;
};

struct CombinationResult {
    bool valid = false;
    std::string message;
    std::vector<CountOptions> countOptions;
    std::vector<std::string> combinationKeys;
    std::vector<LegalCombination> combinations;
    unsigned long long legalCombinationCount = 0;
    std::optional<int> totalCollectionCount;
    std::vector<std::string> risks;
};

class CombinationSolver {
public:
    explicit CombinationSolver(const ItemDatabase& itemDatabase);

    CombinationResult solve(const RoleConfig& role,
                            const std::vector<ColorInputValues>& inputs,
                            std::optional<int> totalCollectionCount = std::nullopt) const;

private:
    struct Distribution {
        std::vector<double> totalPrices;
    };

    const ItemDatabase& itemDatabase_;

    CombinationResult solveVictor(const std::vector<ColorInputValues>& inputs,
                                  std::optional<int> totalCollectionCount) const;
    std::vector<int> deriveCounts(const ColorInputValues& input,
                                  std::vector<std::string>& risks,
                                  bool allowZero = false) const;
    bool hasSupportingItems(const ColorInputValues& input, int count) const;
    Distribution distributionFor(const ColorInputValues& input, int count) const;
    std::vector<std::string> qualitiesForKey(const std::string& key) const;
    void enumerate(const std::vector<CountOptions>& options,
                   std::size_t index,
                   LegalCombination& current,
                   CombinationResult& result) const;
};

} // namespace bbae

