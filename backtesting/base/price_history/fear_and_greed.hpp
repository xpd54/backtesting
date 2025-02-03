#pragma once
#include "price_history/history_subset.hpp"
// TODO :- get fear and greed indexes from https://alternative.me/crypto/fear-and-greed-index/
namespace back_trader {
class FearAndGreed {
  public:
    FearAndGreed(const FearAndGreedHistory &fear_and_greed_history);
    virtual ~FearAndGreed() {}
};
} // namespace back_trader