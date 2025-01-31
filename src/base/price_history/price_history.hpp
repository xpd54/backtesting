#pragma once
#include "history_subset.hpp"
#include <cstddef>
#include <cstdint>
#include <map>
#include <utility>

namespace back_trader {
// Gap or missing price history is a pair of timestamp
using HistoryGap = std::pair<int64_t, int64_t>;

std::vector<HistoryGap> get_price_history_gap(PriceHistory::const_iterator begin, // nowrap
                                              PriceHistory::const_iterator end,   // nowrap
                                              int64_t start_timestamp_sec,        // nowrap
                                              int64_t end_timestamp_sec, size_t top_n);

/* Returns price history with removed outliers.
max_price_deviation_per_min is maximum allowed price deviation per minute.
outlier_indexes is an optional output vector of removed outlier indexes which is accumulated.
What if data have some deviation error and have some sudden changes.
 */
PriceHistory clean_outliers(PriceHistory::const_iterator begin, // nowrap
                            PriceHistory::const_iterator end,   // nowrap
                            float max_price_deviation_per_min,  // nowrap
                            std::vector<size_t> *outlier_indexes);

/* Returns a map from price_history indexes to booleans indicating whether the
 indexes correspond to outliers or not (taking the last_n outlier_indexes).
 Every outlier has left_context_size indexes to the left (if possible) (not to be before the index of first TPV) and
 right_context_size indexes to the right (if possible) (not to exceed index of last TPV).
 price_history_size is the size of the original price history. */
std::map<size_t, bool> get_outlier_indexes_with_context(const std::vector<size_t> &outlier_indexes, // nowrap
                                                        size_t price_history_size,                  // nowrap
                                                        size_t left_context_size,                   // nowrap
                                                        size_t right_context_size,                  // nowrap
                                                        size_t last_n);

// Returns the updated intervaled ohlc_history with the given rate (in seconds).
OhlcHistory update_data_frequency(PriceHistory::const_iterator begin, // nowrap
                                  PriceHistory::const_iterator end,   // nowrap
                                  int interval_rate_sec);
} // namespace back_trader