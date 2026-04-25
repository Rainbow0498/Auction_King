#include "data/ConfigLoader.h"

#include <fstream>
#include <stdexcept>

namespace bbae {

nlohmann::json ConfigLoader::loadJson(const std::filesystem::path& path)
{
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Failed to open JSON file: " + path.string());
    }

    nlohmann::json json;
    file >> json;
    return json;
}

AuctionRules ConfigLoader::loadAuctionRules(const std::filesystem::path& path)
{
    const auto json = loadJson(path);
    AuctionRules rules;

    for (const auto& entry : json.at("rounds")) {
        AuctionRoundRule rule;
        rule.round = entry.value("round", 1);
        rule.multiplier = entry.value("multiplier", 1.0);
        rule.payMode = entry.value("pay_mode", "");
        rules.push_back(rule);
    }

    return rules;
}

} // namespace bbae

