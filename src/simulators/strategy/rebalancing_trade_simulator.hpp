#pragma once
#include "common_strategy_config.hpp"
#include <base_header.hpp>

#include <memory>

namespace back_trader {
// Rebalancing keeps the base (crypto) currency value to quote value ratio constant.
class RebalancingTradeSimulator : public TradeSimulator {
  public:
    explicit RebalancingTradeSimulator(const RebalancingTradeSimulatorConfig &simulator_config)
        : _sim_config(simulator_config) {}
    virtual ~RebalancingTradeSimulator() {}
    void update(const OhlcTick &ohlc_tick, const std::vector<float> &side_input_signals, float base_balance,
                float quote_balance, std::vector<Order> &orders) override;
    std::string get_internal_state() const override;

  private:
    RebalancingTradeSimulatorConfig _sim_config;
    // Last seen account balance.
    float _last_base_balance = 0.0f;
    float _last_quote_balance = 0.0f;
    // Last seen UNIX timestamp (in seconds).
    int64_t _last_timestamp_sec = 0;
    // Last seen close price.
    float _last_close = 0.0f;
};

// helper class of Rrebalancing simulator to get different instance of same simulator
class RebalancingSimulatorDispatcher : public SimulatorDispatcher {
  public:
    explicit RebalancingSimulatorDispatcher(const RebalancingTradeSimulatorConfig &simulator_config)
        : config(simulator_config) {}
    virtual ~RebalancingSimulatorDispatcher() {}
    std::string get_names() const override;
    std::unique_ptr<TradeSimulator> new_simulator() const override;

    // Take list of alphas and epsilons and return simulaterdispatcher with all combinations of alphas and epsilons
    static std::vector<std::unique_ptr<SimulatorDispatcher>> get_batch_of_simulator(const std::vector<float> &alphas,
                                                                                    const std::vector<float> &epsilons);

  private:
    RebalancingTradeSimulatorConfig config;
};

} // namespace back_trader