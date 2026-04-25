#pragma once

#include "core/Clue.h"
#include "core/Item.h"

#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace bbae {

class ItemDatabase {
public:
    void loadFromFile(const std::filesystem::path& path);

    const std::vector<Item>& items() const;
    const std::vector<const Item*>& itemsByQuality(const std::string& quality) const;

    std::vector<const Item*> query(const Clue& clue) const;

private:
    using Index = std::map<std::string, std::vector<const Item*>>;

    std::vector<Item> items_;
    Index byQuality_;
    Index byShape_;
    Index bySize_;
    Index byQualityShape_;
    Index byQualitySize_;
    Index byQualityShapeSize_;

    void buildIndexes();
    static std::string key(const std::string& first, const std::string& second);
    static std::string key(const std::string& first, const std::string& second, const std::string& third);
    static const std::vector<const Item*>& emptyVector();
};

} // namespace bbae

