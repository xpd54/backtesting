#include "simulator_factory.hpp"
#include "strategy/rebalancing_trade_simulator.hpp"
#include "strategy/stop_trade_simulator.hpp"
#include "trade_simulator/trade_simulator.hpp"
#include <cstdlib>
#include <memory>
#include <vector>

namespace back_trader {
static constexpr char RebalancingTradeSimulatorName[] = "rebalancing";
static constexpr char StopTradeSimulatorName[] = "stop";

std::unique_ptr<SimulatorDispatcher> get_default_rebalancing_simulator_dispatcher() {
    RebalancingTradeSimulatorConfig config{0.7f, 0.05f};
    return std::unique_ptr<SimulatorDispatcher>(new RebalancingSimulatorDispatcher(config));
}

std::vector<std::unique_ptr<SimulatorDispatcher>> get_combination_of_rebalancing_trade_simulators() {
    return RebalancingSimulatorDispatcher::get_combination_of_simulator({0.1f, 0.3f, 0.5f, 0.7f, 0.9f},
                                                                        {0.01f, 0.05f, 0.1f, 0.2f});
}

std::unique_ptr<SimulatorDispatcher> get_default_stop_trade_simulator_dispatcher() {
    StopTradeSimulatorConfig config{0.1f, 0.1f, 0.01f, 0.1f};
    return std::unique_ptr<SimulatorDispatcher>(new StopTradeSimulatorDispatcher(config));
}

std::vector<std::unique_ptr<SimulatorDispatcher>> get_combination_of_stop_trade_simulators() {
    return StopTradeSimulatorDispatcher::get_combination_of_simulator(
        /*stop_order_margins=*/{0.05, 0.1, 0.15, 0.2},
        /*stop_order_move_margins=*/{0.05, 0.1, 0.15, 0.2},
        /*stop_order_increases_per_day=*/{0.01, 0.05, 0.1},
        /*stop_order_decreases_per_day=*/{0.01, 0.05, 0.1});
}

/* Update with other strategy if get added, If name of strategy not found end it*/

std::unique_ptr<SimulatorDispatcher> get_trade_simulator(std::string_view strategy_name) {
    if (strategy_name == RebalancingTradeSimulatorName) {
        return get_default_rebalancing_simulator_dispatcher();
    } else if (strategy_name == StopTradeSimulatorName) {
        return get_default_stop_trade_simulator_dispatcher();
    } else {
        std::exit(EXIT_FAILURE);
    }
}
std::vector<std::unique_ptr<SimulatorDispatcher>> get_combination_of_simulators(std::string_view strategy_name) {
    if (strategy_name == RebalancingTradeSimulatorName) {
        return get_combination_of_rebalancing_trade_simulators();
    } else if (strategy_name == StopTradeSimulatorName) {
        return get_combination_of_stop_trade_simulators();
    } else {
        std::exit(EXIT_FAILURE);
    }
}
} // namespace back_trader
