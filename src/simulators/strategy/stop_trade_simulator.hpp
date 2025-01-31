#pragma once
#include "common_strategy_config.hpp"
#include <base_header.hpp>
namespace back_trader {
class StopTradeSimulator : public TradeSimulator {
  public:
    explicit StopTradeSimulator(const StopTradeSimulatorConfig &config) : _sim_config(config) {}
    virtual ~StopTradeSimulator() {}
    void update(const OhlcTick &ohlc_tick, const std::vector<float> &side_input_signals, float base_balance,
                float quote_balance, std::vector<Order> &orders) override;
    std::string get_internal_state() const override;

  private:
    enum class Mode {
        NONE, // Undefined.
        LONG, // Holding most of its assets in the base (crypto) currency.
        CASH  // Holding most of its assets in the quote currency.
    };
    StopTradeSimulatorConfig _sim_config;
    // Last seen account balance.
    float _last_base_balance = 0.0f;
    float _last_quote_balance = 0.0f;
    // Last seen UNIX timestamp (in seconds).
    int64_t _last_timestamp_sec = 0;
    // Last seen close price.
    float _last_close = 0.0f;
    // Last sim mode
    Mode _mode = Mode::NONE;
    // Last base currency price
    float _stop_order_price = 0;

    int _max_allowed_gap_sec = 1 * 60 * 60;

    // Update price of stop order for this simulator
    void update_stop_order_price(Mode mode, int64_t timestamp_sec, float price);

    void emit_stop_order(float price, std::vector<Order> &orders) const;
};

class StopTradeSimulatorDispatcher : public SimulatorDispatcher {

  public:
    explicit StopTradeSimulatorDispatcher(const StopTradeSimulatorConfig &config) : _sim_config(config) {}
    virtual ~StopTradeSimulatorDispatcher() {}

    std::string get_names() const override;
    std::unique_ptr<TradeSimulator> new_simulator() const override;

    static std::vector<std::unique_ptr<SimulatorDispatcher>> get_combination_of_simulator(
        const std::vector<float> &stop_order_margins, const std::vector<float> &stop_order_move_margins,
        const std::vector<float> &stop_order_increases_per_day, const std::vector<float> &stop_order_decreases_per_day);

  private:
    StopTradeSimulatorConfig _sim_config;
};
} // namespace back_trader