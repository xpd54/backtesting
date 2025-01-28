#pragma once
#include "simulators_config.hpp"
#include "trade_simulator/trade_simulator.hpp"

#include <memory>

namespace back_trader {
// Rebalancing keeps the base (crypto) currency value to quote value ratio constant.
class RebalancingTradeSimulator : public TradeSimulator {
  public:
    explicit RebalancingTradeSimulator(const RebalancingTradeSimulatorConfig &simulator_config)
        : config(simulator_config) {}
    virtual ~RebalancingTradeSimulator() {}
    void update(const OhlcTick &ohlc_tick, const std::vector<float> &side_input_signals, float base_balance,
                float quote_balance, std::vector<Order> &orders) override;

  private:
    RebalancingTradeSimulatorConfig config;
    // Last seen account balance.
    float last_base_balance = 0.0f;
    float last_quote_balance = 0.0f;
    // Last seen UNIX timestamp (in seconds).
    int64_t last_timestamp_sec = 0;
    // Last seen close price.
    float last_close = 0.0f;
};

class RebalancingSimulatorDispatcher : public SimulatorDispatcher {
  public:
    explicit RebalancingSimulatorDispatcher(const RebalancingTradeSimulatorConfig &simulator_config)
        : config(simulator_config) {}
    virtual ~RebalancingSimulatorDispatcher() {}
    std::string get_names() const override;
    std::unique_ptr<TradeSimulator> new_simulator() const override;

    static std::vector<std::unique_ptr<SimulatorDispatcher>> get_batch_of_simulator(const std::vector<float> &alphas,
                                                                                    const std::vector<float> &epsilons);

  private:
    RebalancingTradeSimulatorConfig config;
};

} // namespace back_trader