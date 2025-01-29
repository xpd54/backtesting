
#pragma once
#include <base_header.hpp>
#include <memory>
namespace back_trader {

/*

The TradeSimulator operates in following way:

- At every step, the simulator receives the latest OHLC tick (T[i]), along with any additional side input signals
(which is empty as haven't consider for this phase), and the current account balances. Based on this information, the
simulator updates its internal state and data structures. The current time of the simulator is at the end of the OHLC
tick (T[i]). The simulator does not receive zero-volume OHLC ticks, as these indicate a gap in the price
history.

- After processing the data, the TradeSimulator decides which orders to emit. At this moment, there are no active
orders on the exchange.

- Once the simulator determines the orders to emit, the exchange will execute (or cancel) all these orders on the
follow-up OHLC tick (T[i+1]). The simulator does not have visibility into the follow-up OHLC tick (T[i+1]) (nor
any subsequent side input), ensuring that it cannot peek into the future.

- After all orders are executed (or canceled) by the exchange, the simulator receives the follow-up OHLC tick (T[i+1])

Every order gets either executed or canceled by the exchange at each step. This design
simplifies the process, eliminating the need for the simulator to maintain active orders over time. In practical
applications, orders would not be canceled if they are to be re-emitted; instead, existing orders would be modified
based on the updated state.

Moreover, the sampling rate of the OHLC history defines how frequently the simulator is updated and emits orders.
Their behavior and performance should remain consistent regardless of how often they are called. This is crucial because
exchanges (or their APIs) can become unresponsive for random periods, resulting in gaps in price histories. Therefore,
it is encouraged to test the TradeSimulator on OHLC histories with varying sampling rates and gaps.
 */
class TradeSimulator {
  public:
    TradeSimulator(){};
    virtual ~TradeSimulator() {}
    /*
      Updates the (internal) trader state and emits zero or more orders.
      We assume that "orders" is not null and points to an empty vector.
      This method is called consecutively (by the exchange) on every OHLC tick.
      Trader can assume that there are no active orders when this method is
      called. The emitted orders will be either executed or cancelled by the
      exchange at the next OHLC tick.
    */
    virtual void update(const OhlcTick &ohlc_tick, const std::vector<float> &side_input_signals, float base_balance,
                        float quote_balance, std::vector<Order> &orders) = 0;
    // Returns the internal TradeSimulator state (as a string).
    virtual std::string get_internal_state() const = 0;
};

// It can emit a new instance of the same simulator (with the same configuration) whenever needed.
class SimulatorDispatcher {
  public:
    SimulatorDispatcher() {}
    virtual ~SimulatorDispatcher() {}

    /*
     Returns a name identifying all TradeSimulator dispatched.
     The name should be escaped for the CSV file format.
    */
    virtual std::string get_names() const = 0;

    // Returns a new instance of a TradeSimulator
    virtual std::unique_ptr<TradeSimulator> new_simulator() const = 0;
};
} // namespace back_trader