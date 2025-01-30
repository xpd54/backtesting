#include "simulation_log.hpp"
#include "common_interface/common.hpp"
#include "common_util/Logger.hpp"
#include "common_util/string_format_util.hpp"
#include <array>
#include <cstddef>
#include <variant>

using namespace common_util;
namespace back_trader {
SimulationLogger::SimulationLogger(common_util::Logger &logger_instance) : logger(&logger_instance){};

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
    return string_format(order_type_to_string(order.type),                                                 // nowrap
                         order_side_to_string(order.side),                                                 // nowrap
                         (std::holds_alternative<Order::BaseAmount>(order.amount) ? "Base_Amount" : ""),   // nowrap
                         (std::holds_alternative<Order::QuoteAmount>(order.amount) ? "Quote_Amount" : ""), // nowrap
                         order.price);
}

std::string SimulationLogger::empty_order_csv(const Order &order) const { return ",,,,"; }

// log current account and ohlc state
void SimulationLogger::log_account_state(const OhlcTick &ohlc_tick, const Account &account) {}
// log current account, ohlc and order after execution
void SimulationLogger::log_account_state(const OhlcTick &ohlc_tick, const Account &account, const Order &order) {}
void SimulationLogger::log_simulator_state(std::string_view simulator_state) {}
} // namespace back_trader