#pragma once

#include "core/AuctionRule.h"

#include <nlohmann/json.hpp>

#include <filesystem>

namespace bbae {

class ConfigLoader {
public:
    static nlohmann::json loadJson(const std::filesystem::path& path);
    static AuctionRules loadAuctionRules(const std::filesystem::path& path);
};

} // namespace bbae

