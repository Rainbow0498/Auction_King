#include "core/AuctionAdvisor.h"

#include <algorithm>

namespace bbae {

AuctionAdvice AuctionAdvisor::advise(const EstimateResult& result,
                                     const AuctionRules& rules,
                                     int round) const
{
    AuctionAdvice advice;
    if (!result.valid) {
        return advice;
    }

    const double multiplier = multiplierForRound(rules, round);
    advice.conservativeBid = result.p10Value / multiplier;
    advice.balancedBid = result.medianValue / multiplier;
    advice.aggressiveBid = result.p90Value / multiplier;
    advice.maxAcceptableSecondPrice = result.expectedValue / multiplier;
    return advice;
}

double AuctionAdvisor::multiplierForRound(const AuctionRules& rules, int round) const
{
    const auto it = std::find_if(rules.begin(), rules.end(), [round](const AuctionRoundRule& rule) {
        return rule.round == round;
    });
    if (it == rules.end() || it->multiplier <= 0.0) {
        return 1.0;
    }
    return it->multiplier;
}

} // namespace bbae

