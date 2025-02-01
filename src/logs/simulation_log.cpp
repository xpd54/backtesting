#include "simulation_log.hpp"
#include <variant>

using namespace common_util;
namespace back_trader {
SimulationLogger::SimulationLogger(std::ostream *account_os, std::ostream *simulater_os)
    : account_state_os(account_os), simulator_state_os(simulater_os){};

std::string SimulationLogger::ohlc_to_csv(const OhlcTick &ohlc_tick) const {
    return string_format(ohlc_tick.timestamp_sec, ',', // nowrap
                         ohlc_tick.open, ',',          // nowrap
                         ohlc_tick.high, ',',          // nowrap
                         ohlc_tick.low, ',',           // nowrap
                         ohlc_tick.close, ',',         // nowrap
                         ohlc_tick.volume);
}
std::string SimulationLogger::account_to_csv(const Account &account) const {
    return string_format(account.base_balance, ',',  // nowrap
                         account.quote_balance, ',', // nowrap
                         account.total_fee);
}
std::string SimulationLogger::order_to_csv(const Order &order) const {
    return string_format(order_type_to_string(order.type), ',', // nowrap // nowrap
                         order_side_to_string(order.side),
                         ',', // nowrap                                         // nowrap
                         (std::holds_alternative<Order::BaseAmount>(order.amount)
                              ? (std::get<Order::BaseAmount>(order.amount).base_amount)
                              : ' '),
                         ',', // nowrap
                         (std::holds_alternative<Order::QuoteAmount>(order.amount)
                              ? (std::get<Order::BaseAmount>(order.amount).base_amount)
                              : ' '),
                         ',', // nowrap
                         order.price);
}

std::string SimulationLogger::empty_order_csv() const { return ",,,,"; }

// log current account and ohlc state
void SimulationLogger::log_account_state(const OhlcTick &ohlc_tick, const Account &account) {
    if (account_state_os)
        *account_state_os << ohlc_to_csv(ohlc_tick) << account_to_csv(account) << empty_order_csv() << '\n';
}
// log current account, ohlc and order after execution
void SimulationLogger::log_account_state(const OhlcTick &ohlc_tick, const Account &account, const Order &order) {
    if (account_state_os)
        *account_state_os << ohlc_to_csv(ohlc_tick) << account_to_csv(account) << ',' << order_to_csv(order) << '\n';
}

void SimulationLogger::log_simulator_state(std::string_view simulator_internal_state) {
    if (simulator_state_os)
        *simulator_state_os << simulator_internal_state << '\n';
}
} // namespace back_trader