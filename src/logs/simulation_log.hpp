#pragma once
#include <base_header.hpp>
#include <ostream>
#include <string>
#include <string_view>
namespace back_trader {
// Log results in seperater files rather than same system log this will allow to look into excution result progress
// TODO :- Use log file to ploat how account state vary
class SimulationLogger {
  public:
    SimulationLogger(std::ostream *account_os, std::ostream *simulater_os);
    // log current account and ohlc state
    void log_account_state(const OhlcTick &ohlc_tick, const Account &account);
    // log current account, ohlc and order after execution
    void log_account_state(const OhlcTick &ohlc_tick, const Account &account, const Order &order);
    void log_simulator_state(std::string_view simulator_internal_state);

  private:
    std::ostream *account_state_os;
    std::ostream *simulator_state_os;
    std::string ohlc_to_csv(const OhlcTick &ohlc_tick) const;
    std::string account_to_csv(const Account &account) const;
    std::string order_to_csv(const Order &order) const;
    std::string empty_order_csv() const;
};
} // namespace back_trader