#include "utils/Logger.h"

#include <iostream>

namespace bbae {

void Logger::info(const std::string& message)
{
    std::clog << "[info] " << message << '\n';
}

void Logger::warn(const std::string& message)
{
    std::clog << "[warn] " << message << '\n';
}

void Logger::error(const std::string& message)
{
    std::cerr << "[error] " << message << '\n';
}

} // namespace bbae

