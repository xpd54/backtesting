#pragma once
#include "../base/base.hpp"
#include <cstddef>
#include <cstdint>
#include <map>
#include <utility>

namespace back_trader {
// Gap or missing price history is a pair of timestamp
using HistoryGap = std::pair<int64_t, int64_t>;

std::vector<HistoryGap> get_price_history_gap(PriceHistory::const_iterator begin, PriceHistory::const_iterator end,
                                              int64_t start_timestamp_sec, int64_t end_timestamp_sec, size_t top_n);

/* Returns price history with removed outliers.
max_price_deviation_per_min is maximum allowed price deviation per minute.
outlier_indices is an optional output vector of removed outlier indices which is accumulated.
What if data some deviation error and have some sudden jumps.
 */
PriceHistory remove_outliers(PriceHistory::const_iterator begin, PriceHistory::const_iterator end,
                             float max_price_deviation_per_min, std::vector<size_t> *outlier_indices);

/* Returns a map from price_history indices to booleans indicating whether the
 indices correspond to outliers or not (taking the last_n outlier_indices).
 Every outlier has left_context_size indices to the left (if possible) and
 right_context_size indices to the right (if possible).
 price_history_size is the size of the original price history. */
std::map<size_t, bool> get_outlier_indices_with_context(const std::vector<size_t> &outlier_indices,
                                                        size_t price_history_size, size_t left_context_size,
                                                        size_t right_context_size, size_t last_n);

// Returns the resampled ohlc_history with the given sampling rate (in seconds).
OhlcHistory resample(PriceHistory::const_iterator begin, PriceHistory::const_iterator end, int sampling_rate_sec);
} // namespace back_trader