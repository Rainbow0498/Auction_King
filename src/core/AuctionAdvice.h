#pragma once

namespace bbae {

struct AuctionAdvice {
    double conservativeBid = 0.0;
    double balancedBid = 0.0;
    double aggressiveBid = 0.0;
    double maxAcceptableSecondPrice = 0.0;
};

} // namespace bbae

