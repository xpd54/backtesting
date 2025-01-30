#pragma once
#include "price_history/history_subset.hpp"

namespace back_trader {
class FearAndGreed {
  public:
    FearAndGreed(const FearAndGreedHistory &fear_and_greed_history);
    virtual ~FearAndGreed() {}
};
} // namespace back_trader