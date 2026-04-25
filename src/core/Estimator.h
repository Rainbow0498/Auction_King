#pragma once

#include "core/Clue.h"
#include "core/EstimateResult.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace bbae {

class ItemDatabase;

class Estimator {
public:
    explicit Estimator(const ItemDatabase& itemDatabase);

    EstimateResult estimateByClue(const Clue& clue) const;
    EstimateResult estimateVictor(const VictorClue& clue) const;

private:
    struct Distribution {
        unsigned long long candidateCount = 0;
        std::vector<double> samples;
    };

    struct GroupKey {
        std::string quality;
        int count = 0;
        int totalSize = -1;
        int averagePriceWanTimes100 = -1;

        bool operator<(const GroupKey& other) const;
    };

    const ItemDatabase& itemDatabase_;
    mutable std::map<GroupKey, Distribution> groupCache_;

    Distribution buildGroupDistribution(const std::string& quality,
                                        int count,
                                        std::optional<int> totalSize,
                                        std::optional<double> averagePriceWan) const;

    std::vector<int> countOptions(std::optional<int> fixed, int total) const;
    double percentile(std::vector<double> values, double q) const;
    double confidenceFor(const VictorClue& clue, const EstimateResult& result) const;
    static std::vector<double> cappedMerge(std::vector<double> values, std::size_t cap);
    static std::vector<double> combineSamples(const std::vector<double>& lhs,
                                              const std::vector<double>& rhs,
                                              std::size_t cap);
    static unsigned long long saturatedMultiply(unsigned long long lhs, unsigned long long rhs);
    static unsigned long long saturatedAdd(unsigned long long lhs, unsigned long long rhs);
};

} // namespace bbae

