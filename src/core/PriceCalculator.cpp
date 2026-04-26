#include "core/PriceCalculator.h"

#include "data/ItemDatabase.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>

namespace bbae {
namespace {

constexpr std::size_t kPriceSampleCap = 256;

bool closeEnough(double actual, double expected, double toleranceRatio, double absoluteTolerance)
{
    return std::abs(actual - expected) <= std::max(absoluteTolerance, std::abs(expected) * toleranceRatio);
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

ColorInputValues syntheticInputForKey(const std::string& key)
{
    ColorInputValues input;
    input.key = key;
    if (key == "purple") {
        input.label = "紫色";
    } else if (key == "gold") {
        input.label = "橙色";
    } else if (key == "red") {
        input.label = "红色";
    } else if (key == "blue") {
        input.label = "蓝色";
    } else if (key == "white_green") {
        input.label = "白+绿";
    } else {
        input.label = key;
    }
    return input;
}

} // namespace

PriceCalculator::PriceCalculator(const ItemDatabase& itemDatabase)
    : itemDatabase_(itemDatabase)
{
}

PriceEstimate PriceCalculator::calculate(const RoleConfig&,
                                         const std::vector<ColorInputValues>& inputs,
                                         const CombinationResult& combinations) const
{
    PriceEstimate estimate;
    if (!combinations.valid || combinations.combinations.empty()) {
        estimate.risks.push_back("No legal combination is available for price calculation.");
        return estimate;
    }

    std::map<std::string, ColorInputValues> inputByKey;
    for (const auto& input : inputs) {
        inputByKey[input.key] = input;
    }

    std::vector<double> overallLowest;
    std::vector<double> overallGuaranteed;
    std::vector<double> overallReference;
    std::vector<double> overallAggressive;
    std::vector<double> overallHighest;

    for (const auto& combination : combinations.combinations) {
        std::vector<double> totalSamples = {0.0};

        for (const auto& entry : combination.counts) {
            auto inputIt = inputByKey.find(entry.first);
            if (inputIt == inputByKey.end()) {
                inputIt = inputByKey.emplace(entry.first, syntheticInputForKey(entry.first)).first;
            }

            auto distribution = distributionForGroup(inputIt->second, entry.second);
            if (distribution.totalPrices.empty()) {
                totalSamples.clear();
                break;
            }
            totalSamples = combineSamples(totalSamples, distribution.totalPrices, kPriceSampleCap);
        }

        if (totalSamples.empty()) {
            continue;
        }

        CombinationPriceRow row;
        row.counts = combination.counts;
        row.lowestPrice = percentile(totalSamples, 0.10);
        row.guaranteedPrice = percentile(totalSamples, 0.25);
        row.referencePrice = percentile(totalSamples, 0.50);
        row.aggressivePrice = percentile(totalSamples, 0.75);
        row.highestPrice = percentile(totalSamples, 0.90);
        estimate.rows.push_back(row);

        overallLowest.push_back(row.lowestPrice);
        overallGuaranteed.push_back(row.guaranteedPrice);
        overallReference.push_back(row.referencePrice);
        overallAggressive.push_back(row.aggressivePrice);
        overallHighest.push_back(row.highestPrice);
    }

    if (estimate.rows.empty()) {
        estimate.risks.push_back("No legal combination can be valued with current clues.");
        return estimate;
    }

    estimate.valid = true;
    estimate.lowestPrice = *std::min_element(overallLowest.begin(), overallLowest.end());
    estimate.guaranteedPrice = percentile(overallGuaranteed, 0.25);
    estimate.referencePrice = percentile(overallReference, 0.50);
    estimate.aggressivePrice = percentile(overallAggressive, 0.75);
    estimate.highestPrice = *std::max_element(overallHighest.begin(), overallHighest.end());
    estimate.risks = combinations.risks;

    if (estimate.highestPrice > 0.0 && estimate.lowestPrice > 0.0 &&
        estimate.highestPrice / estimate.lowestPrice > 3.0) {
        estimate.risks.push_back("Price range is wide; add stronger purple or gold clues to tighten the result.");
    }

    std::sort(estimate.rows.begin(), estimate.rows.end(), [](const CombinationPriceRow& lhs, const CombinationPriceRow& rhs) {
        return lhs.referencePrice < rhs.referencePrice;
    });
    return estimate;
}

PriceCalculator::Distribution PriceCalculator::distributionForGroup(const ColorInputValues& input, int count) const
{
    Distribution distribution;
    if (count < 0) {
        return distribution;
    }
    if (count == 0) {
        distribution.totalPrices.push_back(0.0);
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
            for (double base : entry.second) {
                target.push_back(base + item->price);
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
            if (input.avgPrice) {
                ok = ok && closeEnough(totalPrice / count, *input.avgPrice, 0.08, 1000.0);
            }
            if (ok) {
                distribution.totalPrices.push_back(totalPrice);
            }
        }
    }

    distribution.totalPrices = reduceSamples(std::move(distribution.totalPrices), kPriceSampleCap);
    return distribution;
}

std::vector<double> PriceCalculator::combineSamples(const std::vector<double>& lhs,
                                                    const std::vector<double>& rhs,
                                                    std::size_t cap)
{
    std::vector<double> values;
    values.reserve(std::min(cap, lhs.size() * rhs.size()));
    for (double left : lhs) {
        for (double right : rhs) {
            values.push_back(left + right);
            if (values.size() >= cap * 4) {
                values = reduceSamples(std::move(values), cap);
            }
        }
    }
    return reduceSamples(std::move(values), cap);
}

std::vector<std::string> PriceCalculator::qualitiesForKey(const std::string& key) const
{
    if (key == "white_green") {
        return {"white", "green"};
    }
    if (key == "gold") {
        return {"gold"};
    }
    return {key};
}

double PriceCalculator::percentile(std::vector<double> values, double q) const
{
    if (values.empty()) {
        return 0.0;
    }
    std::sort(values.begin(), values.end());
    const double pos = q * static_cast<double>(values.size() - 1);
    const auto lower = static_cast<std::size_t>(std::floor(pos));
    const auto upper = static_cast<std::size_t>(std::ceil(pos));
    if (lower == upper) {
        return values[lower];
    }
    const double fraction = pos - static_cast<double>(lower);
    return values[lower] * (1.0 - fraction) + values[upper] * fraction;
}

} // namespace bbae

