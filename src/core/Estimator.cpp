#include "core/Estimator.h"

#include "data/ItemDatabase.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <numeric>

namespace bbae {
namespace {

constexpr std::size_t kSampleCap = 512;
constexpr unsigned long long kSaturatedMax = std::numeric_limits<unsigned long long>::max();

struct DpState {
    unsigned long long count = 0;
    std::vector<double> samples;
};

} // namespace

Estimator::Estimator(const ItemDatabase& itemDatabase)
    : itemDatabase_(itemDatabase)
{
}

bool Estimator::GroupKey::operator<(const GroupKey& other) const
{
    if (quality != other.quality) {
        return quality < other.quality;
    }
    if (count != other.count) {
        return count < other.count;
    }
    if (totalSize != other.totalSize) {
        return totalSize < other.totalSize;
    }
    return averagePriceWanTimes100 < other.averagePriceWanTimes100;
}

EstimateResult Estimator::estimateByClue(const Clue& clue) const
{
    EstimateResult result;
    const auto candidates = itemDatabase_.query(clue);
    if (candidates.empty()) {
        result.message = "No item matched the clue.";
        return result;
    }

    std::vector<double> values;
    values.reserve(candidates.size());
    for (const auto* item : candidates) {
        values.push_back(item->price);
    }

    result.valid = true;
    result.message = "OK";
    result.candidateCount = static_cast<unsigned long long>(values.size());
    result.minValue = *std::min_element(values.begin(), values.end());
    result.maxValue = *std::max_element(values.begin(), values.end());
    result.expectedValue = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    result.medianValue = percentile(values, 0.50);
    result.p10Value = percentile(values, 0.10);
    result.p90Value = percentile(values, 0.90);
    result.confidence = 0.50;
    return result;
}

EstimateResult Estimator::estimateVictor(const VictorClue& clue) const
{
    EstimateResult result;
    if (!clue.totalHighQualityCount || *clue.totalHighQualityCount <= 0) {
        result.message = "Please enter total count for gold + purple + red items.";
        return result;
    }

    const int total = *clue.totalHighQualityCount;
    auto goldCounts = countOptions(clue.gold.count, total);
    auto purpleCounts = countOptions(clue.purple.count, total);

    std::vector<double> allValues;
    unsigned long long totalCandidates = 0;

    for (int goldCount : goldCounts) {
        for (int purpleCount : purpleCounts) {
            const int redCount = total - goldCount - purpleCount;
            if (redCount < 0) {
                continue;
            }

            auto gold = buildGroupDistribution("gold", goldCount, clue.gold.totalSize, clue.gold.averagePriceWan);
            auto purple = buildGroupDistribution("purple", purpleCount, clue.purple.totalSize, clue.purple.averagePriceWan);
            auto red = buildGroupDistribution("red", redCount, std::nullopt, std::nullopt);

            if (gold.samples.empty() || purple.samples.empty() || red.samples.empty()) {
                continue;
            }

            auto combined = combineSamples(gold.samples, purple.samples, kSampleCap);
            combined = combineSamples(combined, red.samples, kSampleCap);
            if (combined.empty()) {
                continue;
            }

            const auto localCandidates = saturatedMultiply(saturatedMultiply(gold.candidateCount, purple.candidateCount),
                                                           red.candidateCount);
            totalCandidates = saturatedAdd(totalCandidates, localCandidates);
            allValues.insert(allValues.end(), combined.begin(), combined.end());

            EstimateBreakdownRow row;
            row.goldCount = goldCount;
            row.purpleCount = purpleCount;
            row.redCount = redCount;
            row.minValue = *std::min_element(combined.begin(), combined.end());
            row.maxValue = *std::max_element(combined.begin(), combined.end());
            row.medianValue = percentile(combined, 0.50);
            row.weight = static_cast<double>(localCandidates == 0 ? 1 : localCandidates);
            result.breakdown.push_back(row);
        }
    }

    allValues = cappedMerge(std::move(allValues), kSampleCap * 2);
    if (allValues.empty()) {
        result.message = "No feasible combination matched Victor clues.";
        return result;
    }

    result.valid = true;
    result.message = "OK";
    result.candidateCount = totalCandidates;
    result.minValue = *std::min_element(allValues.begin(), allValues.end());
    result.maxValue = *std::max_element(allValues.begin(), allValues.end());
    result.expectedValue = std::accumulate(allValues.begin(), allValues.end(), 0.0) / allValues.size();
    result.medianValue = percentile(allValues, 0.50);
    result.p10Value = percentile(allValues, 0.10);
    result.p90Value = percentile(allValues, 0.90);
    result.confidence = confidenceFor(clue, result);

    std::sort(result.breakdown.begin(), result.breakdown.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.medianValue > rhs.medianValue;
    });
    if (result.breakdown.size() > 50) {
        result.breakdown.resize(50);
    }

    return result;
}

Estimator::Distribution Estimator::buildGroupDistribution(const std::string& quality,
                                                          int count,
                                                          std::optional<int> totalSize,
                                                          std::optional<double> averagePriceWan) const
{
    if (count < 0) {
        return {};
    }

    GroupKey key;
    key.quality = quality;
    key.count = count;
    key.totalSize = totalSize.value_or(-1);
    key.averagePriceWanTimes100 = averagePriceWan ? static_cast<int>(std::round(*averagePriceWan * 100.0)) : -1;
    const auto cached = groupCache_.find(key);
    if (cached != groupCache_.end()) {
        return cached->second;
    }

    const auto& items = itemDatabase_.itemsByQuality(quality);
    if (count == 0) {
        Distribution empty;
        empty.candidateCount = 1;
        empty.samples.push_back(0.0);
        groupCache_[key] = empty;
        return empty;
    }
    if (items.empty() || count > static_cast<int>(items.size())) {
        groupCache_[key] = {};
        return {};
    }

    using StateKey = std::pair<int, int>;
    std::map<StateKey, DpState> states;
    states[{0, 0}] = DpState{1, {0.0}};

    for (const auto* item : items) {
        auto snapshot = states;
        for (const auto& entry : snapshot) {
            const int nextCount = entry.first.first + 1;
            const int nextSize = entry.first.second + item->size;
            if (nextCount > count) {
                continue;
            }
            if (totalSize && nextSize > *totalSize) {
                continue;
            }

            auto& target = states[{nextCount, nextSize}];
            target.count = saturatedAdd(target.count, entry.second.count);
            target.samples.reserve(target.samples.size() + entry.second.samples.size());
            for (double value : entry.second.samples) {
                target.samples.push_back(value + item->price);
            }
            target.samples = cappedMerge(std::move(target.samples), kSampleCap);
        }
    }

    Distribution distribution;
    const auto wantedSize = totalSize.value_or(-1);
    for (const auto& entry : states) {
        if (entry.first.first != count) {
            continue;
        }
        if (wantedSize >= 0 && entry.first.second != wantedSize) {
            continue;
        }
        distribution.candidateCount = saturatedAdd(distribution.candidateCount, entry.second.count);
        distribution.samples.insert(distribution.samples.end(), entry.second.samples.begin(), entry.second.samples.end());
    }

    distribution.samples = cappedMerge(std::move(distribution.samples), kSampleCap);

    if (averagePriceWan && count > 0) {
        const double targetTotal = *averagePriceWan * 10000.0 * count;
        const double tolerance = std::max(500.0 * count, targetTotal * 0.06);
        std::vector<double> filtered;
        for (double value : distribution.samples) {
            if (std::abs(value - targetTotal) <= tolerance) {
                filtered.push_back(value);
            }
        }
        distribution.samples = cappedMerge(std::move(filtered), kSampleCap);
        if (distribution.samples.empty()) {
            distribution.candidateCount = 0;
        }
    }

    groupCache_[key] = distribution;
    return distribution;
}

std::vector<int> Estimator::countOptions(std::optional<int> fixed, int total) const
{
    if (fixed) {
        return (*fixed >= 0 && *fixed <= total) ? std::vector<int>{*fixed} : std::vector<int>{};
    }
    std::vector<int> values;
    values.reserve(total + 1);
    for (int i = 0; i <= total; ++i) {
        values.push_back(i);
    }
    return values;
}

double Estimator::percentile(std::vector<double> values, double q) const
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

double Estimator::confidenceFor(const VictorClue& clue, const EstimateResult& result) const
{
    int filled = 0;
    filled += clue.totalHighQualityCount.has_value() ? 1 : 0;
    filled += clue.gold.averagePriceWan.has_value() ? 1 : 0;
    filled += clue.gold.count.has_value() ? 1 : 0;
    filled += clue.gold.totalSize.has_value() ? 1 : 0;
    filled += clue.purple.averagePriceWan.has_value() ? 1 : 0;
    filled += clue.purple.count.has_value() ? 1 : 0;
    filled += clue.purple.totalSize.has_value() ? 1 : 0;

    const double clueScore = std::min(1.0, filled / 7.0);
    const double spread = std::max(0.0, result.p90Value - result.p10Value);
    const double base = std::max(1.0, result.expectedValue);
    const double spreadScore = std::clamp(1.0 - spread / base, 0.0, 1.0);
    const double candidateScore = result.candidateCount <= 1 ? 1.0 : std::clamp(1.0 / std::log10(static_cast<double>(result.candidateCount) + 10.0), 0.0, 1.0);

    return std::clamp(0.20 + clueScore * 0.45 + spreadScore * 0.25 + candidateScore * 0.10, 0.0, 1.0);
}

std::vector<double> Estimator::cappedMerge(std::vector<double> values, std::size_t cap)
{
    if (values.size() <= cap) {
        return values;
    }
    std::sort(values.begin(), values.end());
    std::vector<double> reduced;
    reduced.reserve(cap);
    for (std::size_t i = 0; i < cap; ++i) {
        const double pos = static_cast<double>(i) * static_cast<double>(values.size() - 1) / static_cast<double>(cap - 1);
        reduced.push_back(values[static_cast<std::size_t>(std::round(pos))]);
    }
    return reduced;
}

std::vector<double> Estimator::combineSamples(const std::vector<double>& lhs,
                                              const std::vector<double>& rhs,
                                              std::size_t cap)
{
    std::vector<double> combined;
    combined.reserve(std::min(cap, lhs.size() * rhs.size()));
    for (double left : lhs) {
        for (double right : rhs) {
            combined.push_back(left + right);
            if (combined.size() >= cap * 4) {
                combined = cappedMerge(std::move(combined), cap);
            }
        }
    }
    return cappedMerge(std::move(combined), cap);
}

unsigned long long Estimator::saturatedMultiply(unsigned long long lhs, unsigned long long rhs)
{
    if (lhs == 0 || rhs == 0) {
        return 0;
    }
    if (lhs > kSaturatedMax / rhs) {
        return kSaturatedMax;
    }
    return lhs * rhs;
}

unsigned long long Estimator::saturatedAdd(unsigned long long lhs, unsigned long long rhs)
{
    if (kSaturatedMax - lhs < rhs) {
        return kSaturatedMax;
    }
    return lhs + rhs;
}

} // namespace bbae

