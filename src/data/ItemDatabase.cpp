#include "data/ItemDatabase.h"

#include "data/ConfigLoader.h"

#include <algorithm>
#include <stdexcept>

namespace bbae {

void ItemDatabase::loadFromFile(const std::filesystem::path& path)
{
    const auto json = ConfigLoader::loadJson(path);
    if (!json.is_array()) {
        throw std::runtime_error("items.json must be a JSON array.");
    }

    items_.clear();
    items_.reserve(json.size());

    for (const auto& entry : json) {
        Item item;
        item.id = entry.value("id", "");
        item.name = entry.value("name", "");
        item.quality = entry.value("quality", "");
        item.width = entry.value("width", 0);
        item.height = entry.value("height", 0);
        item.size = entry.value("size", 0);
        item.price = entry.value("price", 0.0);
        item.category = entry.value("category", "");
        item.shape = entry.value("shape", "");
        items_.push_back(std::move(item));
    }

    buildIndexes();
}

const std::vector<Item>& ItemDatabase::items() const
{
    return items_;
}

const std::vector<const Item*>& ItemDatabase::itemsByQuality(const std::string& quality) const
{
    const auto it = byQuality_.find(quality);
    return it == byQuality_.end() ? emptyVector() : it->second;
}

std::vector<const Item*> ItemDatabase::query(const Clue& clue) const
{
    if (clue.quality && clue.shape && clue.size) {
        const auto it = byQualityShapeSize_.find(key(*clue.quality, *clue.shape, std::to_string(*clue.size)));
        return it == byQualityShapeSize_.end() ? std::vector<const Item*>{} : it->second;
    }
    if (clue.quality && clue.shape) {
        const auto it = byQualityShape_.find(key(*clue.quality, *clue.shape));
        return it == byQualityShape_.end() ? std::vector<const Item*>{} : it->second;
    }
    if (clue.quality && clue.size) {
        const auto it = byQualitySize_.find(key(*clue.quality, std::to_string(*clue.size)));
        return it == byQualitySize_.end() ? std::vector<const Item*>{} : it->second;
    }
    if (clue.shape && clue.size) {
        std::vector<const Item*> result;
        const auto sizeIt = bySize_.find(std::to_string(*clue.size));
        if (sizeIt == bySize_.end()) {
            return result;
        }
        std::copy_if(sizeIt->second.begin(), sizeIt->second.end(), std::back_inserter(result), [&clue](const Item* item) {
            return item->shape == *clue.shape;
        });
        return result;
    }
    if (clue.quality) {
        const auto it = byQuality_.find(*clue.quality);
        return it == byQuality_.end() ? std::vector<const Item*>{} : it->second;
    }
    if (clue.shape) {
        const auto it = byShape_.find(*clue.shape);
        return it == byShape_.end() ? std::vector<const Item*>{} : it->second;
    }
    if (clue.size) {
        const auto it = bySize_.find(std::to_string(*clue.size));
        return it == bySize_.end() ? std::vector<const Item*>{} : it->second;
    }

    std::vector<const Item*> all;
    all.reserve(items_.size());
    for (const auto& item : items_) {
        all.push_back(&item);
    }
    return all;
}

void ItemDatabase::buildIndexes()
{
    byQuality_.clear();
    byShape_.clear();
    bySize_.clear();
    byQualityShape_.clear();
    byQualitySize_.clear();
    byQualityShapeSize_.clear();

    for (const auto& item : items_) {
        const auto sizeKey = std::to_string(item.size);
        byQuality_[item.quality].push_back(&item);
        byShape_[item.shape].push_back(&item);
        bySize_[sizeKey].push_back(&item);
        byQualityShape_[key(item.quality, item.shape)].push_back(&item);
        byQualitySize_[key(item.quality, sizeKey)].push_back(&item);
        byQualityShapeSize_[key(item.quality, item.shape, sizeKey)].push_back(&item);
    }
}

std::string ItemDatabase::key(const std::string& first, const std::string& second)
{
    return first + "|" + second;
}

std::string ItemDatabase::key(const std::string& first, const std::string& second, const std::string& third)
{
    return first + "|" + second + "|" + third;
}

const std::vector<const Item*>& ItemDatabase::emptyVector()
{
    static const std::vector<const Item*> empty;
    return empty;
}

} // namespace bbae

