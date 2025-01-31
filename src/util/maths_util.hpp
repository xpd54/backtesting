#pragma once
#include <cassert>
#include <cmath>

namespace back_trader {
template <typename C, typename Func> double get_avrage_of_container(const C &container, Func value_selector) {
    if (container.empty())
        return 0;
    double sum = 0;
    for (const auto &val : container)
        sum += value_selector(val);

    return sum / container.size();
}

template <typename C, typename Func> double get_geometric_avrage_of_container(const C &container, Func value_selector) {
    if (container.empty())
        return 0;
    double mul = 1;
    for (const auto &val : container) {
        mul *= value_selector(val);
    }
    assert(mul >= 0);
    return std::pow(mul, 1.0 / container.size());
}
} // namespace back_trader