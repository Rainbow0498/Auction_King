#pragma once

#include <string>

namespace bbae {

struct Item {
    std::string id;
    std::string name;
    std::string quality;
    int width = 0;
    int height = 0;
    int size = 0;
    double price = 0.0;
    std::string category;
    std::string shape;
};

} // namespace bbae

