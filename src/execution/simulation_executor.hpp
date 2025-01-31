#pragma once
#include "../logs/simulation_log.hpp"
#include "simulation_types.hpp"
#include <base_header.hpp>

namespace back_trader {
/*
 *Execute and instance of simulator(strategy) on range of OHLC history.
 */
SimulationResult execute_trade_simulation(const AccountConfig &account_config,    // nowrap
                                          OhlcHistory::const_iterator ohlc_begin, // nowrap
                                          OhlcHistory::const_iterator ohlc_end,   // nowrap
                                          const FearAndGreed *fear_and_greed,     // nowrap
                                          bool fast_execute,                      // nowrap
                                          TradeSimulator &trade_simulator,        // nowrap
                                          SimulationLogger *logger);

SimulatorEvaluationResult evaluate_trade_simulator(const AccountConfig &account_config,             // nowrap
                                                   const SimEvaluationConfig &sim_eval_config,      // nowrap
                                                   const OhlcHistory &ohlc_tick,                    // nowrap
                                                   const FearAndGreed *fear_and_greed,              // nowrap
                                                   const SimulatorDispatcher &simulator_dispatcher, // nowrap
                                                   SimulationLogger *logger);
std::vector<SimulatorEvaluationResult>
} // namespace back_trader