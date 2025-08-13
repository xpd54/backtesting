#pragma once
#include "common_interface/common.hpp"
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>
namespace back_trader {
constexpr int64_t SecondsPerMinute = 60;
constexpr int64_t SecondsPerHour = 60 * 60;
constexpr int64_t SecondsPerDay = 24 * 60 * 60;
constexpr int64_t SecondsPerWeek = 7 * 24 * 60 * 60;

// Historical prices over time.
using PriceHistory = std::vector<PriceRecord>;

// Historical OHLC ticks over time.
using OhlcHistory = std::vector<OhlcTick>;

// Historical FearAndGreeHistory inputs.
using FearAndGreedHistory = std::vector<FearAndGreedRecord>;

/* Returns a std::pair of iterators covering the time interval [start, end) (not accidental pair don't include end) of
 * the given history, We can run these over all 3 types PriceRecord, OHLCTick and FearAndGreedRecord */
template <typename T>
std::pair<typename std::vector<T>::const_iterator, typename std::vector<T>::const_iterator>
history_subset(const std::vector<T> &history, int64_t start_timestamp_sec, int64_t end_timestamp_sec) {
    const auto record_compare = [](const T &record, int64_t timestamp_sec) {
        return record.timestamp_sec < timestamp_sec;
    };
    const auto record_begin =
        start_timestamp_sec > 0 ? std::lower_bound(history.begin(), history.end(), start_timestamp_sec, record_compare)
                                : history.begin();
    const auto record_end = end_timestamp_sec > 0
                                ? std::lower_bound(history.begin(), history.end(), end_timestamp_sec, record_compare)
                                : history.end();
    return std::make_pair(record_begin, record_end);
}

// Returns the subset value of the given history covering the time interval
// [start_timestamp_sec, end_timestamp_sec).
template <typename T>
std::vector<T> history_subset_copy(const std::vector<T> &history, int64_t start_timestamp_sec,
                                   int64_t end_timestamp_sec) {
    const auto history_subset_it = history_subset(history, start_timestamp_sec, end_timestamp_sec);
    const auto size = std::distance(history_subset_it.first, history_subset_it.second);
    if (size == 0)
        return {};

    std::vector<T> history_subset_result;
    history_subset_result.reserve(size);
    std::copy(history_subset_it.first, history_subset_it.second, std::back_inserter(history_subset_result));
    return history_subset_result;
}

} // namespace back_trader