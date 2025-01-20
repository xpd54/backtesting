#pragma once
#include "../base/base.hpp"
#include <cstddef>
#include <cstdint>
#include <utility>

namespace back_trader {
// Gap or missing price history is a pair of timestamp
using HistoryGap = std::pair<int64_t, int64_t>;

std::vector<HistoryGap> get_price_history_gap(PriceHistory::const_iterator begin, PriceHistory::const_iterator end,
                                              int64_t start_timestamp_sec, int64_t end_timestamp_sec, size_t top_n);

} // namespace back_trader