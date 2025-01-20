#include "price_history.hpp"
#include <algorithm>
#include <cstddef>
#include <iterator>
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

    auto it_previous = begin;
    for (auto it = begin + 1; it != end; ++it) {
        queue.push({it_previous->timestamp_sec, it->timestamp_sec});
        if (queue.size() > top_n)
            queue.pop();

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

PriceHistory remove_outliers(PriceHistory::const_iterator begin, PriceHistory::const_iterator end,
                             float max_price_deviation_per_min, std::vector<size_t> *outlier_indices) {
    // TODO :- check for null outlier_indices and return stop with error
    static constexpr int MAX_LOOKAHEAD = 10;
    static constexpr int MIN_LOOKAHEAD_PERSISTENT = 3;
    PriceHistory price_history_result;
    for (auto it = begin; it != end; ++it) {
        const PriceRecord &price_record = *it;
        const size_t price_record_index = std::distance(begin, it);
        // if price is less than or equal to 0 or volume was zero consider that as outlier and remove it from dataset
        if (price_record.price <= 0 || price_record.volume <= 0) {
            outlier_indices->push_back(price_record_index);
            continue;
        }

        // if price record is not outlier push that to result and look ahead for max_price_deviation
        if (price_history_result.empty()) {
            price_history_result.push_back(price_record);
            continue;
        }

        /*Try to detect if price jump happen that's natural or outlier. we look head in data set (coming more price TPV)
         * if this jump is valid with defined price deviation permin */
        const PriceRecord &price_record_prev = price_history_result.back();
        const float reference_price = price_record_prev.price;
        const float duration_min =
            std::max(1.0f, static_cast<float>(price_record.timestamp_sec - price_record_prev.timestamp_sec) / 60.0f);
        const float jump_factor = (1.0f + max_price_deviation_per_min) * std::sqrt(duration_min);
        const float jump_up_price = reference_price * jump_factor;
        const float jump_down_price = reference_price / jump_factor;
        const bool jumped_up = price_record.price > jump_up_price;
        const bool jumped_down = price_record.price < jump_down_price;
        bool is_outlier = false;
        if (jumped_up || jumped_down) {
            // look ahead in data if price jump persist
            int lookahead = 0;
            int lookahead_persistent = 0;
            const float middle_up_price = 0.8f * jump_up_price + 0.2f * reference_price;
            const float middle_down_price = 0.8f * jump_down_price + 0.2f * reference_price;
            for (auto jt = it + 1; jt != end && lookahead < MAX_LOOKAHEAD; ++jt) {
                if (jt->price <= 0 || jt->volume < 0) {
                    continue;
                }
                if ((jumped_up && jt->price > middle_up_price) || (jumped_down && jt->price < middle_down_price)) {
                    ++lookahead_persistent;
                }
                ++lookahead;
            }
            is_outlier = lookahead_persistent < MIN_LOOKAHEAD_PERSISTENT;
        }
        if (!is_outlier) {
            price_history_result.push_back(price_record);
        } else if (outlier_indices != nullptr) {
            outlier_indices->push_back(price_record_index);
        }
    }
    return price_history_result;
}

/*In outlier indices also get left and right context of indices. (mark them with false and later print that while
 * looking ahead in PriceHistory)*/
std::map<size_t, bool> get_outlier_indices_with_context(const std::vector<size_t> &outlier_indices,
                                                        size_t price_history_size, size_t left_context_size,
                                                        size_t right_context_size, size_t last_n) {
    std::map<size_t, bool> index_to_outlier;
    const size_t start_i = (last_n == 0 || outlier_indices.size() <= last_n) ? 0 : outlier_indices.size() - last_n;
    for (size_t i = start_i; i < outlier_indices.size(); ++i) {
        const size_t j = outlier_indices[i];
        /*Get left and right context range and mark them false for outliers*/
        const size_t left = (j <= left_context_size) ? 0 : j - left_context_size;
        const size_t right = std::min(j + right_context_size + 1, price_history_size);
        for (size_t k = left; k < right; ++k) {
            // Keep the existing outliers.
            index_to_outlier[k] = false;
        }
        // Keep the j (original outlier indices)
        index_to_outlier[j] = true;
    }
    return index_to_outlier;
}

} // namespace back_trader