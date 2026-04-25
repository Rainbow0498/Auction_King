#pragma once

#include <string>
#include <vector>

namespace bbae {

struct AuctionRoundRule {
    int round = 1;
    double multiplier = 1.0;
    std::string payMode;
};

using AuctionRules = std::vector<AuctionRoundRule>;

} // namespace bbae

