#pragma once
#include <base_header.hpp>
#include <memory>
#include <string_view>
namespace back_trader {
// return new instace of trade dispatcher with default set of parameter. ex:- rebalancing sim with alpha 0.7 and epsilon
// 0.1 etc
std::unique_ptr<SimulatorDispatcher> get_trade_simulator(std::string_view &strategy_name);

// return simulators with all combinations of default config
std::vector<std::unique_ptr<SimulatorDispatcher>> get_combination_of_simulators(std::string_view &strategy_name);
} // namespace back_trader