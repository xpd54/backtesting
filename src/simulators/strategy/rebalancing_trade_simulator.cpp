#include "rebalancing_trade_simulator.hpp"
#include "common_interface/common.hpp"
#include "common_util/string_format_util.hpp"
#include <cassert>
#include <common_util.hpp>
#include <cstdint>
#include <memory>

using namespace common_util;
namespace back_trader {
// keep side input (fear & greed index for future dev)
void RebalancingTradeSimulator::update(const OhlcTick &ohlc_tick, const std::vector<float> &side_input_signals,
                                       float base_balance, float quote_balance, std::vector<Order> &orders) {
    const int64_t timestamp_sec = ohlc_tick.timestamp_sec;
    const float price = ohlc_tick.close;
    // we are getting next ohlc
    assert(timestamp_sec > last_timestamp_sec);
    assert(price > 0);
    assert(base_balance > 0 || quote_balance > 0);
    // whole value of this porfolio depends on the current price of base currency
    const float current_portfolio_value = base_balance * price + quote_balance;

    const float alpha = _sim_config.alpha;
    const float epsilon = _sim_config.epsilon;

    /*
     *epsilon is defined as allowed deviaton from original alocation of ratio of base_currency over quote_currency
     */
    const float alpha_max = alpha * (1 + epsilon);
    const float alpha_min = alpha * (1 - epsilon);

    /* Beta shows how much of portfolio value are allocated into base currency. Or can say the exposure to market,
     * (Basically how much it will affect the curren_portfolio value if price of base currency goes up or down) */
    const float beta = base_balance * price / current_portfolio_value;

    /* Base currency allocation is higher than maximum allowed allocation, Logically sell some base currency (crypto)
     and get USD back in protfolio, and that's the rebalancing Trade.*/
    if (beta > alpha_max) {
        // This will reduce the allocation in crypto back to original apha level. (even removing allowed epsilon)
        /* Example :- alpha = 0.7, portfolio_value = 100, beta = 0.9 (90 usd worth in btc and 10 in usd), epsilon = 0.1,
         * price (btc/usd) = 10. (10% deviation allowd but right now it's more), So we sell
         * ((1 - 0.7) * 100 - 10) /10 = 2 (BTC). That will bring current alpha or current allocation in base currency
         * down to original 0.7 which meas alpha == beta. And do this imidiatly so apply as market order*/
        const float sell_base_amount = ((1 - alpha) * current_portfolio_value - quote_balance) / price;
        orders.emplace_back();
        Order &sell_order = orders.back();
        sell_order.side = Order::Side::SELL;
        sell_order.type = Order::Type::MARKET;
        sell_order.amount = Order::BaseAmount{sell_base_amount};
    } else if (beta < alpha_min) {
        // This will reduce quote_balance and allocate more in crypto
        const float buy_base_amount = (quote_balance - (1 - alpha) * current_portfolio_value) / price;
        orders.emplace_back();
        Order &buy_order = orders.back();
        buy_order.side = Order::Side::BUY;
        buy_order.type = Order::Type::MARKET;
        buy_order.amount = Order::BaseAmount{buy_base_amount};
    } else if (base_balance > 1.0e-6f && quote_balance > 1.0e-6f) {
        // any other case when base and quote are not zero (avoid making base or quote to zero)and our protfolio_value
        // is not deviated, sell for profit unless alpha is 1 means allocate all in base currency
        // Buy / Sell strategy for profit.
        if (alpha * (1 + epsilon) < 1) {

            const float sell_price = (alpha * (1 + epsilon) * quote_balance) / (1 - alpha * (1 + epsilon));

            // 100.0f factor too good to be true
            if (sell_price > price && sell_price < 100.0f * price) {
                const float sell_base_amount = base_balance * epsilon / (1 + epsilon);
                orders.emplace_back();
                Order &sell_order = orders.back();
                sell_order.side = Order::Side::SELL;

                // put a limit order as we are selling profit not for balancing the portfolio
                sell_order.type = Order::Type::LIMIT;
                sell_order.amount = Order::BaseAmount{sell_base_amount};
                sell_order.price = sell_price;
            }
        }

        const float buy_price = (alpha * (1 - epsilon) * quote_balance) / (1 - alpha * (1 - epsilon));
        if (buy_price < price && buy_price > price / 100.0f) {
            const float buy_base_amount = base_balance * epsilon * epsilon / (1 - epsilon);
            orders.emplace_back();
            Order &buy_order = orders.back();
            buy_order.type = Order::Type::LIMIT;
            buy_order.side = Order::Side::BUY;
            buy_order.amount = Order::BaseAmount{buy_base_amount};
            buy_order.price = buy_price;
        }
    }

    // save the state of trade
    _last_base_balance = base_balance;
    _last_quote_balance = quote_balance;
    _last_timestamp_sec = timestamp_sec;
    _last_close = price;
}

std::string RebalancingTradeSimulator::get_internal_state() const {
    return string_format(_last_timestamp_sec, _last_base_balance, _last_quote_balance, _last_close);
}

std::string RebalancingSimulatorDispatcher::get_names() const {
    return string_format("rebalancing_trade_simulator[", config.alpha, '|', config.epsilon, ']');
}

std::unique_ptr<TradeSimulator> RebalancingSimulatorDispatcher::new_simulator() const {
    return std::make_unique<RebalancingTradeSimulator>(config);
}

std::vector<std::unique_ptr<SimulatorDispatcher>>
RebalancingSimulatorDispatcher::get_combination_of_simulator(const std::vector<float> &alphas,
                                                             const std::vector<float> &epsilons) {
    std::vector<std::unique_ptr<SimulatorDispatcher>> dispatchers;

    dispatchers.reserve(alphas.size() * epsilons.size());
    // get all combination of alphas and epsilons
    for (const auto &alpha : alphas)
        for (const auto &epsilon : epsilons) {
            RebalancingTradeSimulatorConfig sim_config{alpha, epsilon};
            dispatchers.emplace_back(new RebalancingSimulatorDispatcher(sim_config));
        }
    return dispatchers;
}
} // namespace back_trader