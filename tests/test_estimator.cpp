#include "core/Estimator.h"
#include "data/ItemDatabase.h"

#include <cassert>
#include <filesystem>

int main()
{
    bbae::ItemDatabase database;
    database.loadFromFile(std::filesystem::current_path() / "data" / "items.json");
    bbae::Estimator estimator(database);

    bbae::VictorClue clue;
    clue.totalHighQualityCount = 3;
    clue.gold.count = 1;
    clue.purple.count = 1;

    const auto result = estimator.estimateVictor(clue);
    assert(result.valid);
    assert(result.candidateCount > 0);
    return 0;
}

