#pragma once

#include "core/AuctionAdvice.h"
#include "core/AuctionRule.h"
#include "core/EstimateResult.h"

namespace bbae {

class AuctionAdvisor {
public:
    AuctionAdvice advise(const EstimateResult& result,
                         const AuctionRules& rules,
                         int round) const;

private:
    double multiplierForRound(const AuctionRules& rules, int round) const;
};

} // namespace bbae

