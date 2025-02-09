#pragma once
#include "common_util/string_format_util.hpp"
#include <common_util.hpp>
using namespace common_util;
namespace back_trader {
inline void logInfo(std::string str) { Logger::get_instance()(Logger::Severity::INFO) << str << Logger::endl; }
inline void logError(std::string str) { Logger::get_instance()(Logger::Severity::ERROR) << str << Logger::endl; }

inline std::unique_ptr<std::ofstream> get_log_stream(const std::string &log_filename) {
    if (log_filename.empty()) {
        return nullptr;
    }
    auto log_file_stream = std::make_unique<std::ofstream>(log_filename);
    if (!log_file_stream->is_open()) {
        logInfo(string_format("Can not open log file ", log_filename));
    }
    return log_file_stream;
}
} // namespace back_trader