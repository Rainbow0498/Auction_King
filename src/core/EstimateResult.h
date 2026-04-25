#pragma once

#include <string>
#include <vector>

namespace bbae {

struct EstimateBreakdownRow {
    int goldCount = 0;
    int purpleCount = 0;
    int redCount = 0;
    double minValue = 0.0;
    double medianValue = 0.0;
    double maxValue = 0.0;
    double weight = 0.0;
};

struct EstimateResult {
    bool valid = false;
    std::string message;
    double expectedValue = 0.0;
    double minValue = 0.0;
    double maxValue = 0.0;
    double medianValue = 0.0;
    double p10Value = 0.0;
    double p90Value = 0.0;
    double confidence = 0.0;
    unsigned long long candidateCount = 0;
    std::vector<EstimateBreakdownRow> breakdown;
};

} // namespace bbae

