#pragma once
namespace back_trader {
// Rebalancing configuration.
struct RebalancingTradeSimulatorConfig {
    /*
      alpha = 0.7 means that we would like to keep 70%
      of the total portfolio value in the base (crypto) currency,
      and 30% in the quote currency (USD).
    */
    float alpha;

    /*
     Maximum allowed deviation from the desired alpha-allocation.
     We allow the actual base (crypto) currency allocation to be within
     the interval: (alpha * (1 - epsilon); alpha * (1 + epsilon).
    */
    float epsilon;
};

// Stop configuration.
struct StopTradeSimulatorConfig {
    // Margin for setting the stop order price w.r.t. the current price.
    float stop_order_margin;
    // Margin for moving the stop order price w.r.t. the current price.
    float stop_order_move_margin;
    // Maximum relative stop order price increase per day.
    float stop_order_increase_per_day;
    // Maximum relative stop order price decrease per day.
    float stop_order_decrease_per_day;
};
} // namespace back_trader