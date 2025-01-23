#pragma once
#include "../util/quick_log.hpp"
#include <common_util.hpp>
#include <vector>
using namespace common_util;
namespace back_trader {

template <typename T>
std::vector<T> read_history_from_binary_file(const std::string &file_name, const std::time_t start_time,
                                             const std::time_t end_time, std::function<bool(const T &)> validate) {
    const std::time_t latency_start_time = std::time(nullptr);
    logInfo("Reading history from binary file" + file_name);
    common_util::RMemoryMapped<T> read_file(file_name);
    const T *begin = read_file.begin();
    const T *end = read_file.end();
    std::vector<T> price_history;
    if (!validate) {
        price_history.assign(begin, end);
    } else {
        // run time validataion and push value to price history
        uint64_t valid_count = 0;
        while (begin != end) {
            T history = *begin;
            if (start_time > 0 && history.timestamp_sec < start_time)
                continue;
            if (end_time > 0 && history.timestamp_sec > end_time)
                break;
            if (validate(history)) {
                price_history.push_back(history);
                ++valid_count;
            } else {
                logError("Reading history from binary file, Invalid at line number " + std::to_string(valid_count));
                break;
            }
            ++begin;
        }
    }

    const std::time_t latency_end_time = std::time(nullptr);
    logInfo("loaded " + std::to_string(price_history.size()) + " records in " +
            std::to_string((latency_end_time - latency_start_time)) + " sec");
    return price_history;
}

template <typename T>
bool write_history_to_binary_file(const std::vector<T> &history, const std::string &output_price_history_binary_file) {
    const std::time_t latency_start_time = std::time(nullptr);
    logInfo("Number of records " + std::to_string(history.size()) + "to file " + output_price_history_binary_file);
    // Memory map the file to output binary file
    size_t file_size = sizeof(T) * history.size();
    common_util::WMemoryMapped<T> write_file(output_price_history_binary_file, file_size);
    T *begin = write_file.begin();
    std::copy(history.begin(), history.end(), begin);
    write_file.flush();
    const std::time_t latency_end_time = std::time(nullptr);
    logInfo("finished in " + std::to_string(latency_end_time - latency_start_time));
    return true;
}
} // namespace back_trader