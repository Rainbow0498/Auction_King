#include "core/AuctionAdvisor.h"

#include <cassert>

int main()
{
    bbae::EstimateResult result;
    result.valid = true;
    result.p10Value = 100.0;
    result.medianValue = 200.0;
    result.p90Value = 300.0;
    result.expectedValue = 220.0;

    bbae::AuctionRules rules{{1, 2.0, "second_price_multiplier"}};
    bbae::AuctionAdvisor advisor;
    const auto advice = advisor.advise(result, rules, 1);
    assert(advice.conservativeBid == 50.0);
    assert(advice.balancedBid == 100.0);
    return 0;
}

