#include "core/CombinationSolver.h"

#include "data/ItemDatabase.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>

namespace bbae {
namespace {

constexpr int kDefaultMaxCount = 12;
constexpr int kHardMaxCount = 80;
constexpr std::size_t kCombinationSampleCap = 500;
constexpr std::size_t kPriceSampleCap = 256;

bool closeEnough(double actual, double expected, double toleranceRatio, double absoluteTolerance)
{
    return std::abs(actual - expected) <= std::max(absoluteTolerance, std::abs(expected) * toleranceRatio);
}

unsigned long long saturatedMultiply(unsigned long long lhs, unsigned long long rhs)
{
    if (lhs == 0 || rhs == 0) {
        return 0;
    }
    const auto maxValue = std::numeric_limits<unsigned long long>::max();
    if (lhs > maxValue / rhs) {
        return maxValue;
    }
    return lhs * rhs;
}

std::vector<double> reduceSamples(std::vector<double> values, std::size_t cap)
{
    if (values.size() <= cap) {
        return values;
    }
    std::sort(values.begin(), values.end());
    std::vector<double> reduced;
    reduced.reserve(cap);
    for (std::size_t i = 0; i < cap; ++i) {
        const double pos = static_cast<double>(i) * static_cast<double>(values.size() - 1) /
                           static_cast<double>(cap - 1);
        reduced.push_back(values[static_cast<std::size_t>(std::round(pos))]);
    }
    return reduced;
}

} // namespace

CombinationSolver::CombinationSolver(const ItemDatabase& itemDatabase)
    : itemDatabase_(itemDatabase)
{
}

CombinationResult CombinationSolver::solve(const RoleConfig& role,
                                           const std::vector<ColorInputValues>& inputs,
                                           std::optional<int> totalCollectionCount) const
{
    CombinationResult result;
    if (inputs.empty()) {
        result.message = "No color input groups are configured.";
        return result;
    }

    result.totalCollectionCount = totalCollectionCount;

    if (role.id == "victor" || role.mode == "high_value_only") {
        return solveVictor(inputs, totalCollectionCount);
    }

    if (!role.enableCombinationSolver) {
        LegalCombination direct;
        for (const auto& input : inputs) {
            int count = 0;
            if (input.totalCount) {
                count = *input.totalCount;
            } else {
                auto derived = deriveCounts(input, result.risks);
                count = derived.empty() ? 0 : derived.front();
            }

            CountOptions options;
            options.key = input.key;
            options.label = input.label;
            options.counts = {count};
            result.countOptions.push_back(options);
            result.combinationKeys.push_back(input.key);
            direct.counts[input.key] = count;
        }

        result.valid = true;
        result.message = "OK";
        result.legalCombinationCount = 1;
        result.combinations.push_back(std::move(direct));
        result.risks.push_back(role.name + " is in direct estimate mode; no complex combinations were expanded.");
        return result;
    }

    for (const auto& input : inputs) {
        CountOptions options;
        options.key = input.key;
        options.label = input.label;
        options.counts = deriveCounts(input, result.risks);
        if (options.counts.empty()) {
            result.message = input.label + " has no legal item count.";
            return result;
        }
        result.countOptions.push_back(std::move(options));
        result.combinationKeys.push_back(input.key);
    }

    result.legalCombinationCount = 1;
    for (const auto& options : result.countOptions) {
        result.legalCombinationCount = saturatedMultiply(result.legalCombinationCount,
                                                         static_cast<unsigned long long>(options.counts.size()));
    }

    LegalCombination current;
    enumerate(result.countOptions, 0, current, result);
    result.valid = result.legalCombinationCount > 0;
    result.message = result.valid ? "OK" : "No legal combination matched current inputs.";

    if (result.legalCombinationCount > kCombinationSampleCap) {
        result.risks.push_back("Too many legal combinations; the detail table shows representative samples only.");
    }

    return result;
}

CombinationResult CombinationSolver::solveVictor(const std::vector<ColorInputValues>& inputs,
                                                 std::optional<int> totalCollectionCount) const
{
    CombinationResult result;
    result.totalCollectionCount = totalCollectionCount;
    result.combinationKeys = {"purple", "gold", "red"};

    if (!totalCollectionCount || *totalCollectionCount <= 0) {
        result.message = "Victor mode requires total collection count for purple + gold + red.";
        return result;
    }

    std::optional<ColorInputValues> purpleInput;
    std::optional<ColorInputValues> goldInput;
    for (const auto& input : inputs) {
        if (input.key == "purple") {
            purpleInput = input;
        } else if (input.key == "gold") {
            goldInput = input;
        }
    }

    if (!purpleInput || !goldInput) {
        result.message = "Victor mode requires purple and gold input cards.";
        return result;
    }

    CountOptions purpleOptions;
    purpleOptions.key = "purple";
    purpleOptions.label = purpleInput->label;
    purpleOptions.counts = deriveCounts(*purpleInput, result.risks, true);

    CountOptions goldOptions;
    goldOptions.key = "gold";
    goldOptions.label = goldInput->label;
    goldOptions.counts = deriveCounts(*goldInput, result.risks, true);

    if (purpleOptions.counts.empty() || goldOptions.counts.empty()) {
        result.message = "Purple or gold has no legal count under current clues.";
        return result;
    }

    std::vector<int> redCounts;
    unsigned long long legalCount = 0;
    for (int purpleCount : purpleOptions.counts) {
        for (int goldCount : goldOptions.counts) {
            const int redCount = *totalCollectionCount - purpleCount - goldCount;
            if (redCount < 0) {
                continue;
            }
            ++legalCount;

            redCounts.push_back(redCount);
            if (result.combinations.size() < kCombinationSampleCap) {
                LegalCombination combination;
                combination.counts["purple"] = purpleCount;
                combination.counts["gold"] = goldCount;
                combination.counts["red"] = redCount;
                result.combinations.push_back(std::move(combination));
            }
        }
    }

    if (redCounts.empty()) {
        result.message = "No legal red count can be inferred from total collection count.";
        return result;
    }

    std::sort(redCounts.begin(), redCounts.end());
    redCounts.erase(std::unique(redCounts.begin(), redCounts.end()), redCounts.end());

    CountOptions redOptions;
    redOptions.key = "red";
    redOptions.label = "红色";
    redOptions.counts = std::move(redCounts);

    result.countOptions.push_back(std::move(purpleOptions));
    result.countOptions.push_back(std::move(goldOptions));
    result.countOptions.push_back(std::move(redOptions));
    result.legalCombinationCount = legalCount;
    result.valid = true;
    result.message = "OK";

    if (result.countOptions[2].counts.size() > 6) {
        result.risks.push_back("Red count still spans a wide range; purple or gold needs stronger clues.");
    }
    if (result.legalCombinationCount > kCombinationSampleCap) {
        result.risks.push_back("Combination detail table shows representative rows only.");
    }
    return result;
}

std::vector<int> CombinationSolver::deriveCounts(const ColorInputValues& input,
                                                 std::vector<std::string>& risks,
                                                 bool allowZero) const
{
    if (input.totalCount) {
        if (*input.totalCount < 0 || !hasSupportingItems(input, *input.totalCount)) {
            return {};
        }
        return {*input.totalCount};
    }

    int maxCount = kDefaultMaxCount;
    if (input.totalGrid) {
        maxCount = std::max(maxCount, static_cast<int>(std::ceil(*input.totalGrid)));
    }
    maxCount = std::clamp(maxCount, 1, kHardMaxCount);

    std::vector<int> counts;
    const int startCount = allowZero ? 0 : 1;
    for (int count = startCount; count <= maxCount; ++count) {
        if (hasSupportingItems(input, count)) {
            counts.push_back(count);
        }
    }

    if (!input.totalCount && !input.totalGrid && !input.unitGridPrice && !input.totalPrice) {
        risks.push_back(input.label + " has no strong clue; count falls back to a broad search range.");
    } else if (!input.totalCount && counts.size() > 8) {
        risks.push_back(input.label + " still matches many counts; current clues are not tight yet.");
    }

    return counts;
}

bool CombinationSolver::hasSupportingItems(const ColorInputValues& input, int count) const
{
    return !distributionFor(input, count).totalPrices.empty();
}

CombinationSolver::Distribution CombinationSolver::distributionFor(const ColorInputValues& input, int count) const
{
    Distribution distribution;
    if (count < 0) {
        return distribution;
    }
    if (count == 0) {
        if (!input.totalGrid && !input.unitGridPrice && !input.totalPrice) {
            distribution.totalPrices.push_back(0.0);
        }
        return distribution;
    }

    std::vector<const Item*> items;
    for (const auto& quality : qualitiesForKey(input.key)) {
        const auto& group = itemDatabase_.itemsByQuality(quality);
        items.insert(items.end(), group.begin(), group.end());
    }
    if (items.empty()) {
        return distribution;
    }

    int maxItemSize = 0;
    for (const auto* item : items) {
        maxItemSize = std::max(maxItemSize, item->size);
    }

    const int targetSize = input.totalGrid ? static_cast<int>(std::round(*input.totalGrid)) : -1;
    const int maxTotalSize = targetSize >= 0 ? targetSize : count * std::max(1, maxItemSize);

    std::map<std::pair<int, int>, std::vector<double>> states;
    states[{0, 0}] = {0.0};

    for (const auto* item : items) {
        auto snapshot = states;
        for (const auto& entry : snapshot) {
            const int nextCount = entry.first.first + 1;
            const int nextSize = entry.first.second + item->size;
            if (nextCount > count || nextSize > maxTotalSize) {
                continue;
            }

            auto& target = states[{nextCount, nextSize}];
            for (double basePrice : entry.second) {
                target.push_back(basePrice + item->price);
            }
            target = reduceSamples(std::move(target), kPriceSampleCap);
        }
    }

    for (const auto& entry : states) {
        const int currentCount = entry.first.first;
        const int currentSize = entry.first.second;
        if (currentCount != count) {
            continue;
        }
        if (targetSize >= 0 && currentSize != targetSize) {
            continue;
        }

        for (double totalPrice : entry.second) {
            bool ok = true;
            if (input.totalPrice) {
                ok = ok && closeEnough(totalPrice, *input.totalPrice, 0.06, 1000.0);
            }
            if (input.unitGridPrice && currentSize > 0) {
                ok = ok && closeEnough(totalPrice / currentSize, *input.unitGridPrice, 0.08, 120.0);
            }
            if (ok) {
                distribution.totalPrices.push_back(totalPrice);
            }
        }
    }

    distribution.totalPrices = reduceSamples(std::move(distribution.totalPrices), kPriceSampleCap);
    return distribution;
}

std::vector<std::string> CombinationSolver::qualitiesForKey(const std::string& key) const
{
    if (key == "white_green") {
        return {"white", "green"};
    }
    if (key == "gold") {
        return {"gold"};
    }
    return {key};
}

void CombinationSolver::enumerate(const std::vector<CountOptions>& options,
                                  std::size_t index,
                                  LegalCombination& current,
                                  CombinationResult& result) const
{
    if (result.combinations.size() >= kCombinationSampleCap) {
        return;
    }
    if (index >= options.size()) {
        result.combinations.push_back(current);
        return;
    }

    const auto& option = options[index];
    for (int count : option.counts) {
        current.counts[option.key] = count;
        enumerate(options, index + 1, current, result);
        if (result.combinations.size() >= kCombinationSampleCap) {
            return;
        }
    }
}

} // namespace bbae

