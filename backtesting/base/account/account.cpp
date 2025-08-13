#include "account.hpp"
#include "common_interface/common.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <variant>
namespace back_trader {

float Floor(float amount, float unit) { return unit > 0 ? unit * std::floor(amount / unit) : amount; }
float Ceil(float amount, float unit) { return unit > 0 ? unit * std::ceil(amount / unit) : amount; }
float Round(float amount, float unit) { return unit > 0 ? unit * std::round(amount / unit) : amount; }

// Positive price is required for non-market orders.
// Every order must specify a positive base amount or quote amount.
bool is_valid_order(const Order &order) {
    return (order.type == Order::Type::MARKET || order.price > 0) &&
           ((std::holds_alternative<Order::BaseAmount>(order.amount) &&
             std::get<Order::BaseAmount>(order.amount).base_amount > 0) ||
            (std::holds_alternative<Order::QuoteAmount>(order.amount) &&
             std::get<Order::QuoteAmount>(order.amount).quote_amount > 0));
}

void Account::init_account(const AccountConfig &account_confg) {
    base_balance = account_confg.start_base_balance;
    quote_balance = account_confg.start_quote_balance;
    total_fee = 0;
    base_unit = account_confg.base_unit;
    quote_unit = account_confg.quote_unit;
    market_liquidity = account_confg.market_liquidity;
    max_volume_ratio = account_confg.max_volume_ratio;
}

float Account::get_fee(const FeeConfig &fee_config, float quote_amount) const {
    return Ceil(std::max(fee_config.minimum_fee, fee_config.fixed_fee + quote_amount * fee_config.relative_fee),
                quote_unit);
}

float Account::get_max_base_amount(const OhlcTick &ohlc_tick) const {
    return max_volume_ratio > 0 ? Floor(max_volume_ratio * ohlc_tick.volume, base_unit)
                                : std::numeric_limits<float>::max();
}

/*----------------------------------------- Price Of Type Of Orders ---------------------------------------------*/

/* If liquidity is 1 buy order will execute at OHLC open price. If liquidity is 0 buy order will execute at OHLC high */
float Account::get_market_buy_price(const OhlcTick &ohlc_tick) const {
    return market_liquidity * ohlc_tick.open + (1.0f - market_liquidity) * ohlc_tick.high;
}

/*
 *If liquidity is 1 sell order will execute at OHLC open price. If liquidity is 0 sell order will execute at OHLC low
 */
float Account::get_market_sell_price(const OhlcTick &ohlc_tick) const {
    return market_liquidity * ohlc_tick.open + (1.0f - market_liquidity) * ohlc_tick.low;
}

/*
 * If liquidity is 1 stop buy order will execute at OHLC open price . If liquidity is 0 stop buy order will execute at
 * OHLC high
 */
float Account::get_stop_buy_price(const OhlcTick &ohlc_tick, float stop_price) const {
    return market_liquidity * std::max(stop_price, ohlc_tick.open) + (1.0f - market_liquidity) * ohlc_tick.high;
}

/*
 * If liquidity is 1 stop sell order will execute at OHLC open price. If liquidity is 0 stop sell order will execute at
 * OHLC low
 */
float Account::get_stop_sell_price(const OhlcTick &ohlc_tick, float stop_price) const {
    return market_liquidity * std::min(stop_price, ohlc_tick.open) + (1.0f - market_liquidity) * ohlc_tick.low;
}

/*------------------------------------- Execute Order At Specific Price ---------------------------------------------*/

bool Account::buy_base_currency(const FeeConfig &fee_config, float base_amount, float price) {
    assert(price > 0);
    assert(base_amount > 0);
    base_amount = Round(base_amount, base_unit);

    /*base unit is lowest denomination of base currency*/
    if (base_amount < base_unit) {
        return false;
    }

    const float quote_amount = Ceil(base_amount * price, quote_unit);
    const float quote_fee = get_fee(fee_config, quote_amount);
    const float total_quote_amount = quote_amount + quote_fee;

    /*Account is holding less balance than buy price*/
    if (total_quote_amount > quote_balance) {
        return false;
    }

    // update account
    base_balance = Round(base_balance + base_amount, base_unit);
    quote_balance = Round(quote_balance - total_quote_amount, quote_unit);
    total_fee = Round(total_fee + quote_fee, quote_unit);
    return true;
}

bool Account::buy_at_quote(const FeeConfig &fee_config, float quote_amount, float price, float max_base_amount) {
    assert(price > 0);
    assert(quote_amount >= 0);
    quote_amount = Round(quote_balance, quote_unit);

    /* can't exicute when balance is low or amount is lower than lowest denomination*/
    if (quote_amount < quote_unit || quote_amount > quote_balance) {
        return false;
    }
    const float quote_fee = get_fee(fee_config, quote_amount);

    /*all amount will get paid as fee*/
    if (quote_amount < quote_fee) {
        return false;
    }

    const float base_amount = Floor(std::min((quote_amount - quote_fee) / price, max_base_amount), base_unit);

    /* buy base amount is smaller than lowest denomination*/
    if (base_amount < base_unit) {
        return false;
    }

    return buy_base_currency(fee_config, base_amount, price);
}

bool Account::sell_base_currency(const FeeConfig &fee_config, float base_amount, float price) {
    assert(price > 0);
    assert(base_amount >= 0);
    base_amount = Round(base_amount, base_unit);

    /* can't sell lower than lowest denomination for base currency, or more than which account have in base balance*/
    if (base_amount < base_unit || base_amount > base_balance) {
        return false;
    }
    const float quote_amount = Floor(base_amount * price, base_unit);
    const float quote_fee = get_fee(fee_config, quote_amount);
    const float total_quote_amount = quote_amount - quote_fee;

    /* After selling account is getting lower than lowest denomination.*/
    if (total_quote_amount < quote_unit) {
        return false;
    }

    // update account
    base_balance = Round(base_balance - base_amount, base_unit);
    quote_balance = Round(quote_balance + total_quote_amount, quote_unit);
    total_fee = Round(total_fee + quote_fee, quote_unit);
    return true;
}

bool Account::sell_at_quote(const FeeConfig &fee_config, float quote_amount, float price, float max_base_amount) {
    assert(price > 0);
    assert(quote_amount >= 0);
    quote_amount = Round(quote_amount, quote_unit);
    if (quote_amount < quote_unit) {
        return false;
    }

    const float quote_fee = get_fee(fee_config, quote_amount);
    const float base_amount = Floor(std::min((quote_amount + quote_fee) / price, max_base_amount), base_unit);

    /* Selling lower amount of base currency than lowest denomination of base currency*/
    if (base_amount < base_unit) {
        return false;
    }

    /*
     * Note: When we sell base_amount of base currency, we receive at most:
     * (quote_amount + quote_fee) - GetFee(quote_amount + quote_fee)
     * Since GetFee(quote_amount) <= GetFee(quote_amount + quote_fee),
     * We receive at most quote_amount of quote currency.
     */
    return sell_base_currency(fee_config, base_amount, price);
}

/*------------- Execute Market Orders (Buy/Sell at market price itself) -----------------------------------*/

bool Account::market_buy(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float base_amount) {
    const float price = get_market_buy_price(ohlc_tick);
    return buy_base_currency(fee_config, base_amount, price);
}

bool Account::market_buy_at_quote(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float quote_amount) {
    const float price = get_market_buy_price(ohlc_tick);
    return buy_at_quote(fee_config, quote_amount, price);
}

bool Account::market_sell(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float base_amount) {
    const float price = get_market_sell_price(ohlc_tick);
    return sell_base_currency(fee_config, base_amount, price);
}

bool Account::market_sell_at_quote(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float quote_amount) {
    const float price = get_market_sell_price(ohlc_tick);
    return sell_at_quote(fee_config, quote_amount, price);
}

/*------------- Execute Stop Orders (Buy/Sell When Market Rise/Fall To Stop Price) ---------------------------*/

bool Account::stop_buy(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float base_amount, float stop_price) {
    assert(stop_price > 0);
    assert(base_amount >= 0);

    /* Stop buy order only get executed when price RISE above the stop_price*/
    if (ohlc_tick.high < stop_price) {
        return false;
    }
    const float price = get_stop_buy_price(ohlc_tick, stop_price);
    return buy_base_currency(fee_config, base_amount, price);
}

bool Account::stop_buy_at_quote(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float quote_amount,
                                float stop_price) {
    assert(stop_price > 0);
    assert(quote_amount >= 0);
    /* Stop buy order only get executed when price RISE above the stop_price*/
    if (ohlc_tick.high < stop_price) {
        return false;
    }
    const float price = get_stop_buy_price(ohlc_tick, stop_price);
    return buy_at_quote(fee_config, quote_amount, price);
}

bool Account::stop_sell(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float base_amount, float stop_price) {
    assert(stop_price > 0);
    assert(base_amount >= 0);

    /* Stop sell order only get executed when price FALL below the stop_price*/
    if (ohlc_tick.low > stop_price) {
        return false;
    }
    const float price = get_stop_sell_price(ohlc_tick, stop_price);
    return sell_base_currency(fee_config, base_amount, price);
}

bool Account::stop_sell_at_quote(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float quote_amount,
                                 float stop_price) {
    assert(stop_price > 0);
    assert(quote_amount >= 0);

    /* Stop sell order only get executed when price FALL below the stop_price*/
    if (ohlc_tick.low > stop_price) {
        return false;
    }
    const float price = get_stop_sell_price(ohlc_tick, stop_price);
    return sell_at_quote(fee_config, quote_amount, price);
}

/*------------- Execute Limit Orders (Buy/Sell When Market Rise/Fall To Stop Price) ---------------------------*/

bool Account::limit_buy(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float base_amount, float limit_price) {
    assert(limit_price > 0);
    assert(base_amount >= 0);

    /* Limit buy order only get executed when price FALL below the limit_price */
    if (ohlc_tick.low > limit_price) {
        return false;
    }

    base_amount = std::min(base_amount, get_max_base_amount(ohlc_tick));
    return buy_base_currency(fee_config, base_amount, limit_price);
}

bool Account::limit_buy_at_quote(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float quote_amount,
                                 float limit_price) {
    assert(limit_price > 0);
    assert(quote_amount >= 0);

    /* Limit buy order only get executed when price FALL below the limit_price */
    if (ohlc_tick.low > limit_price) {
        return false;
    }

    const float max_base_amount = get_max_base_amount(ohlc_tick);
    return buy_at_quote(fee_config, quote_amount, limit_price, max_base_amount);
}

bool Account::limit_sell(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float base_amount, float limit_price) {
    assert(limit_price > 0);
    assert(base_amount >= 0);

    /* Limit sell order only get executed when price RISE above the limit_price */
    if (ohlc_tick.high < limit_price) {
        return false;
    }

    base_amount = std::min(base_amount, get_max_base_amount(ohlc_tick));
    return sell_base_currency(fee_config, base_amount, limit_price);
}

bool Account::limit_sell_at_quote(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float quote_amount,
                                  float limit_price) {
    assert(limit_price > 0);
    assert(quote_amount >= 0);

    /* Limit sell order only get executed when price RISE above the limit_price */
    if (ohlc_tick.high < limit_price) {
        return false;
    }
    const float max_base_amount = get_max_base_amount(ohlc_tick);
    return sell_at_quote(fee_config, quote_amount, limit_price, max_base_amount);
}

bool Account::execute_order(const AccountConfig &account_config, const Order &order, const OhlcTick &ohlc_tick) {
    assert(is_valid_order(order));
    float possible_base_amount = std::holds_alternative<Order::BaseAmount>(order.amount)
                                     ? std::get<Order::BaseAmount>(order.amount).base_amount
                                     : 0.0;
    float possible_quote_amount = std::holds_alternative<Order::QuoteAmount>(order.amount)
                                      ? std::get<Order::QuoteAmount>(order.amount).quote_amount
                                      : 0.0;
    switch (order.type) {
    case Order::Type::MARKET: {
        if (order.side == Order::Side::BUY) {
            if (std::holds_alternative<Order::BaseAmount>(order.amount)) {
                return market_buy(account_config.market_order_fee_config, ohlc_tick, possible_base_amount);
            } else {
                return market_buy_at_quote(account_config.market_order_fee_config, ohlc_tick, possible_quote_amount);
            }
        } else {
            assert(order.side == Order::Side::SELL);
            if (std::holds_alternative<Order::BaseAmount>(order.amount)) {
                return market_sell(account_config.market_order_fee_config, ohlc_tick, possible_base_amount);
            } else {
                assert(std::holds_alternative<Order::QuoteAmount>(order.amount));
                return market_sell_at_quote(account_config.market_order_fee_config, ohlc_tick, possible_quote_amount);
            }
        }
    } break;
    case Order::Type::STOP: {
        if (order.side == Order::Side::BUY) {
            if (std::holds_alternative<Order::BaseAmount>(order.amount)) {
                return stop_buy(account_config.stop_order_fee_config, ohlc_tick, possible_base_amount, order.price);
            } else {
                assert(std::holds_alternative<Order::QuoteAmount>(order.amount));
                return stop_buy_at_quote(account_config.stop_order_fee_config, ohlc_tick, possible_quote_amount,
                                         order.price);
            }
        } else {
            assert(order.side == Order::Side::SELL);
            if (std::holds_alternative<Order::BaseAmount>(order.amount)) {
                return stop_sell(account_config.stop_order_fee_config, ohlc_tick, possible_base_amount, order.price);
            } else {
                assert(std::holds_alternative<Order::QuoteAmount>(order.amount));
                return stop_sell_at_quote(account_config.stop_order_fee_config, ohlc_tick, possible_quote_amount,
                                          order.price);
            }
        }
    } break;
    case Order::Type::LIMIT: {
        if (order.side == Order::Side::BUY) {
            if (std::holds_alternative<Order::BaseAmount>(order.amount)) {
                return limit_buy(account_config.limit_order_fee_config, ohlc_tick, possible_base_amount, order.price);
            } else {
                assert(std::holds_alternative<Order::QuoteAmount>(order.amount));
                return limit_buy_at_quote(account_config.limit_order_fee_config, ohlc_tick, possible_quote_amount,
                                          order.price);
            }
        } else {
            assert(order.side == Order::Side::SELL);
            if (std::holds_alternative<Order::BaseAmount>(order.amount)) {
                return limit_sell(account_config.limit_order_fee_config, ohlc_tick, possible_base_amount, order.price);
            } else {
                assert(std::holds_alternative<Order::QuoteAmount>(order.amount));
                return limit_sell_at_quote(account_config.limit_order_fee_config, ohlc_tick, possible_quote_amount,
                                           order.price);
            }
        }
    } break;
    default:
        assert(false);
    }
    return true;
}
} // namespace back_trader