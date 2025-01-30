#pragma once
#include "../logs/simulation_log.hpp"
#include "simulation_types.hpp"
#include <base_header.hpp>

namespace back_trader {
/*
 *Execute and instance of simulator(strategy) on range of OHLC history.
 */
SimulationResult execute_trade_simulation(const AccountConfig &account_config, OhlcHistory::const_iterator ohlc_begin,
                                          OhlcHistory::const_iterator ohlc_end, const FearAndGreed *fear_and_greed,
                                          bool fast_execute, TradeSimulator &trade_simulator, SimulationLogger *logger);
} // namespace back_trader