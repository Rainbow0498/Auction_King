#pragma once

#include <optional>
#include <string>

namespace bbae {

struct Clue {
    std::optional<std::string> quality;
    std::optional<std::string> shape;
    std::optional<int> size;
};

struct VictorQualityClue {
    std::optional<double> averagePriceWan;
    std::optional<int> count;
    std::optional<int> totalSize;
};

struct VictorClue {
    std::optional<int> totalHighQualityCount;
    VictorQualityClue gold;
    VictorQualityClue purple;
};

} // namespace bbae

