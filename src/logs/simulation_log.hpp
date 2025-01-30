#pragma once
#include <base_header.hpp>
#include <common_util.hpp>
#include <string>
#include <string_view>
namespace back_trader {
class SimulationLogger {
  public:
    SimulationLogger(common_util::Logger &logger_instance);
    // log current account and ohlc state
    void log_account_state(const OhlcTick &ohlc_tick, const Account &account);
    // log current account, ohlc and order after execution
    void log_account_state(const OhlcTick &ohlc_tick, const Account &account, const Order &order);
    void log_simulator_state(std::string_view simulator_internal_state);

  private:
    common_util::Logger *logger;
    std::string ohlc_to_csv(const OhlcTick &ohlc_tick) const;
    std::string account_to_csv(const Account &account) const;
    std::string order_to_csv(const Order &order) const;
    std::string empty_order_csv() const;
};
} // namespace back_trader