#include "simulation_executor.hpp"
#include "../util/maths_util.hpp"
#include "common_util/time_util.hpp"
#include "price_history/history_subset.hpp"
#include "simulation_types.hpp"
#include "trade_simulator/trade_simulator.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace back_trader {
SimulationResult execute_trade_simulation(const AccountConfig &account_config,      // nowrap
                                          OhlcHistory::const_iterator ohlc_begin,   // nowrap
                                          OhlcHistory::const_iterator ohlc_end,     // nowrap
                                          const FearAndGreed *fear_and_greed_input, // nowrap
                                          bool fast_execute,                        // nowrap
                                          TradeSimulator &trade_simulator,          // nowrap
                                          SimulationLogger *logger) {
    SimulationResult simulation_result;
    // No data to process
    if (ohlc_begin == ohlc_end)
        return {};
    assert(logger != nullptr);

    // Every simulator (strategy) would have it's own account to track the transaction
    Account account;
    account.init_account(account_config);
    std::vector<Order> orders;
    constexpr size_t DispatchedOrderReserve = 8;
    orders.reserve(DispatchedOrderReserve);
    int count_executed_orders = 0;
    for (auto ohlc_it = ohlc_begin; ohlc_it != ohlc_end; ++ohlc_it) {
        // TODO :- handle fear_and_greed_input here according to each ohlc tick
        const OhlcTick &ohlc_tick = *ohlc_it;

        // Log current ohlc and account
        logger->log_account_state(ohlc_tick, account);

        /*
         *  The trade simulator was updated on the previous OHLC tick OHLC_HISTORY[i-1] and emitted
         *  "orders". There are no other active orders on the exchange.
         *  Execute (or cancel) "orders" on the current OHLC tick OHLC_HISTORY[i].
         */

        for (const Order &order : orders) {
            const bool executed = account.execute_order(account_config, order, ohlc_tick);
            if (executed) {
                ++count_executed_orders;
                logger->log_account_state(ohlc_tick, account, order);
            }
        }

        if (ohlc_tick.volume == 0) {
            /*
             * zero volume on ohlc tick is missing price history so we keep our order as what was generated for previous
             * order In real world exchange api is down let's pause for a bit.
             */
            continue;
        }

        // as we have already executed previous tick order let update for current ohlc tick
        orders.clear();
        trade_simulator.update(ohlc_tick, {}, account.base_balance, account.quote_balance, orders);
        logger->log_simulator_state(trade_simulator.get_internal_state());

        // TODO :- handle calculation of volatility according to fast_execute

        simulation_result.start_base_balance = account_config.start_base_balance;
        simulation_result.start_quote_balance = account_config.start_quote_balance;
        simulation_result.end_base_balance = account.base_balance;
        simulation_result.end_quote_balance = account.quote_balance;
        simulation_result.start_price = ohlc_begin->close;
        // end iterator are 1 pass end
        simulation_result.end_price = (--ohlc_end)->close;
        simulation_result.start_value = simulation_result.start_quote_balance +
                                        simulation_result.start_price * simulation_result.start_base_balance;

        simulation_result.end_value =
            simulation_result.end_quote_balance + simulation_result.end_price * simulation_result.end_base_balance;

        simulation_result.total_order = count_executed_orders;
        simulation_result.total_fee = account.total_fee;
        // TODO :- calculate volatility for now keep it zero
        simulation_result.base_volatility = 0.0f;
        simulation_result.simulator_volatility = 0.0f;
    }
    return simulation_result;
}

SimulatorEvaluationResult evaluate_trade_simulator(const AccountConfig &account_config,              // nowrap
                                                   const SimEvaluationConfig &sim_evaluation_config, // nowrap
                                                   const OhlcHistory &ohlc_histroy,                  // nowrap
                                                   const FearAndGreed *fear_and_greed_input,         // nowrap
                                                   const SimulatorDispatcher &simulator_dispatcher,  // nowrap
                                                   SimulationLogger *logger) {
    SimulatorEvaluationResult simulation_eval_result;
    simulation_eval_result.account_config = account_config;
    simulation_eval_result.sim_evaluation_config = sim_evaluation_config;
    simulation_eval_result.name = simulator_dispatcher.get_names();
    for (int month_offset = 0;; ++month_offset) {
        const int64_t start_evalueation_timestamp_sec =
            add_months(sim_evaluation_config.start_timestamp_sec, month_offset);

        const int64_t end_evaluation_timestamp_sec =
            sim_evaluation_config.evaluation_period_months > 0
                ? add_months(start_evalueation_timestamp_sec, sim_evaluation_config.evaluation_period_months)
                : sim_evaluation_config.end_timestamp_sec;
        if (end_evaluation_timestamp_sec > sim_evaluation_config.end_timestamp_sec) {
            break;
        }

        // Get pair of iterator which denote start and end of time stamp
        auto ohlc_history_subset =
            history_subset(ohlc_histroy, start_evalueation_timestamp_sec, end_evaluation_timestamp_sec);
        // skip no data found
        if (ohlc_history_subset.first == ohlc_history_subset.second)
            continue;
        std::unique_ptr<TradeSimulator> trade_simulator = simulator_dispatcher.new_simulator();
        SimulationResult sim_result = execute_trade_simulation(
            account_config, ohlc_history_subset.first, ohlc_history_subset.second, {}, false, *trade_simulator, logger);
        simulation_eval_result.periods.emplace_back();
        SimulatorEvaluationResult::TimePeriod *time_period = &simulation_eval_result.periods.back();
        time_period->start_timestamp_sec = start_evalueation_timestamp_sec;
        time_period->end_timestamp_sec = end_evaluation_timestamp_sec;
        time_period->result = sim_result;
        assert(sim_result.start_value > 0);
        time_period->final_gain = (sim_result.end_value / sim_result.start_value);
        assert(sim_result.start_price > 0 && sim_result.end_price > 0);
        // gain for buy and hold
        time_period->base_final_gain = (sim_result.end_price / sim_result.start_price);
        if (sim_evaluation_config.evaluation_period_months == 0) {
            break;
        }
    }

    simulation_eval_result.score = get_geometric_avrage_of_container(
        simulation_eval_result.periods,
        [](const SimulatorEvaluationResult::TimePeriod &period) { return period.final_gain / period.base_final_gain; });

    simulation_eval_result.avg_gain =
        get_avrage_of_container(simulation_eval_result.periods,
                                [](const SimulatorEvaluationResult::TimePeriod &period) { return period.final_gain; });

    simulation_eval_result.avg_base_gain = get_avrage_of_container(
        simulation_eval_result.periods,
        [](const SimulatorEvaluationResult::TimePeriod &period) { return period.base_final_gain; });

    simulation_eval_result.avg_total_executed_orders = get_avrage_of_container(
        simulation_eval_result.periods,
        [](const SimulatorEvaluationResult::TimePeriod &period) { return period.result.total_order; });

    simulation_eval_result.avg_total_fee =
        get_avrage_of_container(simulation_eval_result, [](const SimulatorEvaluationResult::TimePeriod &period) {
            return period.result.total_fee;
        });
    return simulation_eval_result;
}

} // namespace back_trader