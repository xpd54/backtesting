#include "price_history.hpp"
#include <algorithm>
#include <cstddef>
#include <queue>

namespace back_trader {
/* check if price history_timestamps are valid*/
bool check_price_history_timestamps(const PriceHistory &price_history) {
    for (size_t i = 1; i < price_history.size(); ++i) {
        if (price_history[i].timestamp_sec < price_history[i - 1].timestamp_sec)
            return false;
    }
    return true;
}

/*
As exchange api can be unresponsive there are gaps in (T,P,V).
Ex:-
(10:01, 100, 1)
(10:02, 101, 1)
(10:05, 104, 1)
(10:06, 104, 1)
(10:07, 104, 1)
(11:00, 100, 1)

If data is like given example for per sec, There is missing 2 data point (10:03, 10:04)
and huge gap between 10:07 to 11:00 which is 43 data point missing. Here Try to detect those gaps and print top_n gaps.
Push each interval in pariority_queue and compare the gap width.
*/

std::vector<HistoryGap> get_price_history_gap(PriceHistory::const_iterator begin, PriceHistory::const_iterator end,
                                              int64_t start_timestamp_sec, int64_t end_timestamp_sec, size_t top_n) {
    // if there is no time diff no price gap
    if (begin == end)
        return {};
    auto gap_compare = [](HistoryGap left, HistoryGap right) {
        /*if
        left (start:- 10:00, end:- 10:05); right (start:- 11:00, end:- 10:03)
        left is consider first. If gap is same length than chronologically
        */
        const int64_t delta = (left.second - left.first) - (right.second - right.first);
        return delta > 0 || (delta == 0 && left.first < right.first);
    };

    std::priority_queue<HistoryGap, std::vector<HistoryGap>, decltype(gap_compare)> queue;
    if (start_timestamp_sec > 0) {
        queue.push(HistoryGap{start_timestamp_sec, begin->timestamp_sec});
    }

    auto it_previous = end;
    for (auto it = begin; it != end; ++it) {
        if (it_previous != end) {
            queue.push({it_previous->timestamp_sec, it->timestamp_sec});
            if (queue.size() > top_n)
                queue.pop();
        }
        it_previous = it;
    }

    if (end_timestamp_sec > 0) {
        queue.push({it_previous->timestamp_sec, end_timestamp_sec});
        if (queue.size() > top_n) {
            queue.pop();
        }
    }

    std::vector<HistoryGap> history_gaps_result;
    while (!queue.empty()) {
        history_gaps_result.push_back(queue.top());
        queue.pop();
    }
    return std::vector<HistoryGap>(history_gaps_result.rbegin(), history_gaps_result.rend());
}

} // namespace back_trader