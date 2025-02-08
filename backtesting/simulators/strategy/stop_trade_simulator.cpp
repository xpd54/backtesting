#include "stop_trade_simulator.hpp"
#include "common_interface/common.hpp"
#include "common_util/string_format_util.hpp"
#include "price_history/history_subset.hpp"
#include <cassert>
#include <common_util.hpp>
#include <cstdint>
#include <memory>

using namespace common_util;
namespace back_trader {

// TODO :- fear_and_greed_input_signals signal handling (https://alternative.me/crypto/fear-and-greed-index/)
void StopTradeSimulator::update(const OhlcTick &ohlc_tick, const std::vector<float> &fear_and_greed_input_signals,
                                float base_balance, float quote_balance, std::vector<Order> &orders) {
    const int64_t timestamp_sec = ohlc_tick.timestamp_sec;
    const float price = ohlc_tick.close;
    // ohlc is coming as time moving forward
    assert(timestamp_sec > _last_timestamp_sec);
    assert(price > 0);
    // Do not trade simulate for account balance == 0
    assert(base_balance > 0 || quote_balance > 0);
    // if most of the currency are in crypto it's Long or otherwise. Get current account mode
    const Mode current_mode = (base_balance * price >= quote_balance) ? Mode::LONG : Mode::CASH;
    // current ohlc shouldn't be coming more than 1 hour intervals or mode has been changed
    if (timestamp_sec >= timestamp_sec + _max_allowed_gap_sec || current_mode != _mode) {
        if (current_mode == Mode::LONG) {
            _stop_order_price = (1 - _sim_config.stop_order_margin) * price;
        } else {
            assert(current_mode == Mode::CASH);
            _stop_order_price = (1 + _sim_config.stop_order_margin) * price;
        }
    } else {
        // If last last executed mode was same as current ohlc just update the price
        assert(current_mode == _mode);
        update_stop_order_price(current_mode, timestamp_sec, price);
    }
    _last_timestamp_sec = timestamp_sec;
    _last_quote_balance = quote_balance;
    _last_base_balance = base_balance;
    _last_close = price;
    _mode = current_mode;
    emit_stop_order(price, orders);
}

void StopTradeSimulator::update_stop_order_price(Mode mode, int64_t timestamp_sec, float price) {
    const float current_interval_rate_sec = std::min(SecondsPerDay, timestamp_sec - _last_timestamp_sec);
    const float ticks_per_day = SecondsPerDay / current_interval_rate_sec;
    if (mode == Mode::LONG) {
        const float stop_order_increase_threshold = (1 - _sim_config.stop_order_move_margin) * price;
        if (_stop_order_price <= stop_order_increase_threshold) {
            const float stop_order_increase_per_tick =
                std::exp(std::log(1 + _sim_config.stop_order_increase_per_day) / ticks_per_day) - 1;
            _stop_order_price =
                std::max(_stop_order_price, std::min(stop_order_increase_threshold,
                                                     (1 + stop_order_increase_per_tick) * _stop_order_price));
        }
    } else {
        assert(mode == Mode::CASH);
        const float stop_order_decrease_threshold = (1 + _sim_config.stop_order_move_margin) * price;
        if (_stop_order_price >= stop_order_decrease_threshold) {
            const float stop_order_decrease_per_tick =
                1 - std::exp(std::log(1 - _sim_config.stop_order_decrease_per_day) / ticks_per_day);
            _stop_order_price =
                std::min(_stop_order_price, std::max(stop_order_decrease_threshold,
                                                     (1 - stop_order_decrease_per_tick) * _stop_order_price));
        }
    }
}

void StopTradeSimulator::emit_stop_order(float price, std::vector<Order> &orders) const {
    orders.emplace_back();
    Order &order = orders.back();
    order.type = Order::Type::STOP;
    if (_mode == Mode::LONG) {
        order.side = Order::Side::SELL;
        order.amount = Order::BaseAmount{_last_base_balance};
    } else {
        assert(_mode == Mode::CASH);
        order.side = Order::Side::BUY;
        order.amount = Order::QuoteAmount{_last_quote_balance};
    }
    order.price = _stop_order_price;
}

std::string StopTradeSimulator::get_internal_state() const {
    return string_format(_last_timestamp_sec, ',', _last_base_balance, ',', _last_quote_balance, ',', _last_close, ',',
                         _mode == Mode::LONG ? "LONG" : "CASH", _stop_order_price);
}

std::string StopTradeSimulatorDispatcher::get_names() const {
    return string_format("stop_trade_simulator[", _sim_config.stop_order_margin, '|',
                         _sim_config.stop_order_move_margin, '|', _sim_config.stop_order_increase_per_day, '|',
                         _sim_config.stop_order_decrease_per_day, ']');
}

std::unique_ptr<TradeSimulator> StopTradeSimulatorDispatcher::new_simulator() const {
    return std::make_unique<StopTradeSimulator>(_sim_config);
}

// get simulator with all combination of parameters
std::vector<std::unique_ptr<SimulatorDispatcher>> StopTradeSimulatorDispatcher::get_combination_of_simulator(
    const std::vector<float> &stop_order_margins, const std::vector<float> &stop_order_move_margins,
    const std::vector<float> &stop_order_increases_per_day, const std::vector<float> &stop_order_decreases_per_day) {
    std::vector<std::unique_ptr<SimulatorDispatcher>> dispatchers;
    dispatchers.reserve(stop_order_margins.size() * stop_order_move_margins.size() *
                        stop_order_increases_per_day.size() * stop_order_decreases_per_day.size());
    for (const auto &stop_order_margin : stop_order_margins)
        for (const auto &stop_order_move_margin : stop_order_move_margins)
            for (const auto &stop_order_increase_per_day : stop_order_increases_per_day)
                for (const auto &stop_order_decrease_per_day : stop_order_decreases_per_day) {
                    StopTradeSimulatorConfig sim_config;
                    sim_config.stop_order_margin = stop_order_margin;
                    sim_config.stop_order_move_margin = stop_order_move_margin;
                    sim_config.stop_order_increase_per_day = stop_order_increase_per_day;
                    sim_config.stop_order_decrease_per_day = stop_order_decrease_per_day;
                    dispatchers.emplace_back(new StopTradeSimulatorDispatcher(sim_config));
                }
    return dispatchers;
}

} // namespace back_trader