#pragma once

#include "core/ColorInput.h"
#include "core/CombinationSolver.h"
#include "core/RoleConfig.h"

#include <optional>
#include <map>
#include <string>
#include <vector>

namespace bbae {

class ItemDatabase;

struct CombinationPriceRow {
    std::map<std::string, int> counts;
    double lowestPrice = 0.0;
    double guaranteedPrice = 0.0;
    double referencePrice = 0.0;
    double aggressivePrice = 0.0;
    double highestPrice = 0.0;
};

struct PriceEstimate {
    bool valid = false;
    double lowestPrice = 0.0;
    double guaranteedPrice = 0.0;
    double referencePrice = 0.0;
    double aggressivePrice = 0.0;
    double highestPrice = 0.0;
    std::vector<CombinationPriceRow> rows;
    std::vector<std::string> risks;
};

class PriceCalculator {
public:
    explicit PriceCalculator(const ItemDatabase& itemDatabase);

    PriceEstimate calculate(const RoleConfig& role,
                            const std::vector<ColorInputValues>& inputs,
                            const CombinationResult& combinations) const;

private:
    struct Distribution {
        std::vector<double> totalPrices;
    };

    const ItemDatabase& itemDatabase_;

    Distribution distributionForGroup(const ColorInputValues& input, int count) const;
    static std::vector<double> combineSamples(const std::vector<double>& lhs,
                                              const std::vector<double>& rhs,
                                              std::size_t cap);
    std::vector<std::string> qualitiesForKey(const std::string& key) const;
    double percentile(std::vector<double> values, double q) const;
};

} // namespace bbae
