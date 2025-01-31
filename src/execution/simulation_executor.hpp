#pragma once
#include "../logs/simulation_log.hpp"
#include "simulation_types.hpp"
#include <base_header.hpp>
#include <memory>

namespace back_trader {
/*
 *Execute and instance of simulator(strategy) on range of OHLC history.
 */
SimulationResult execute_trade_simulation(const AccountConfig &account_config,      // nowrap
                                          OhlcHistory::const_iterator ohlc_begin,   // nowrap
                                          OhlcHistory::const_iterator ohlc_end,     // nowrap
                                          const FearAndGreed *fear_and_greed_input, // nowrap
                                          bool fast_execute,                        // nowrap
                                          TradeSimulator &trade_simulator,          // nowrap
                                          SimulationLogger *logger);

/*
 * Evalulate single simulator
 */
SimulatorEvaluationResult evaluate_trade_simulator(const AccountConfig &account_config,              // nowrap
                                                   const SimEvaluationConfig &sim_evaluation_config, // nowrap
                                                   const OhlcHistory &ohlc_histroy,                  // nowrap
                                                   const FearAndGreed *fear_and_greed_input,         // nowrap
                                                   const SimulatorDispatcher &simulator_dispatcher,  // nowrap
                                                   SimulationLogger *logger);

/*
 * Evaluate combination (Strategy sim we have use all combination of
 * config ex:- alpha and epsilon to generate list of
 * simulators) of simulatators no multi threads;
 */
std::vector<SimulatorEvaluationResult> evaluate_combination_of_trade_simulators(
    const AccountConfig &account_config,
    const SimEvaluationConfig &sim_evaluation_config, // nowrap
    const OhlcHistory &ohlc_history,                  // nowrap
    const FearAndGreed *fear_and_greed_input,
    const std::vector<std::unique_ptr<SimulatorDispatcher>> &simulator_dispatchers);
} // namespace back_trader