#include "simulation_executor.hpp"
#include "../util/maths_util.hpp"
#include "common_interface/common.hpp"
#include "simulation_types.hpp"
#include <cassert>
#include <cstddef>

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
} // namespace back_trader