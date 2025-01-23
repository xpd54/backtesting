#pragma once
#include "common_util/Logger.hpp"
#include <common_util.hpp>
using namespace common_util;
namespace back_trader {
inline void logInfo(std::string str) { Logger::get_instance().log(str, Logger::Severity::INFO); }
inline void logError(std::string str) { Logger::get_instance().log(str, Logger::Severity::ERROR); }
} // namespace back_trader