#include "data/ItemDatabase.h"

#include <cassert>
#include <filesystem>

int main()
{
    bbae::ItemDatabase database;
    database.loadFromFile(std::filesystem::current_path() / "data" / "items.json");
    assert(!database.items().empty());
    assert(!database.itemsByQuality("gold").empty());
    return 0;
}

