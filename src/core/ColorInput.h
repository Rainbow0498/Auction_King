#pragma once

#include <optional>
#include <string>

namespace bbae {

struct ColorInputValues {
    std::string key;
    std::string label;
    std::optional<double> unitGridPrice;
    std::optional<int> totalCount;
    std::optional<double> totalGrid;
    std::optional<double> avgPrice;
    std::optional<double> totalPrice;
};

} // namespace bbae
