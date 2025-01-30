#pragma once
/*Common class, struct, data structure used across data_generator and backtesting trade*/

#include <array>
#include <cstdint>
#include <variant>
namespace back_trader {

/*
TPV (time, price, volume)
This dataset is from bitstamp. (BTC => USD)
*/
struct PriceRecord {
    int64_t timestamp_sec;
    // BTC price (BTC => USD price).
    float price;
    // Traded currency volume.
    float volume;
};

// open, hight, low, close prices for time interval
// Timestamp of end of interval is assumed implicit
struct OhlcTick {
    // timestamp start of time interval
    int64_t timestamp_sec;
    // opening base currency price at start of time interval.
    float open;
    // highest base currency price at time interval.
    float high;
    // lowest base currency price at time interval.
    float low;
    // closing base currency price at end of the time interval.
    float close;
    // Traded volume during the time interval
    float volume;
};

struct FearAndGreedRecord {
    int64_t timestamp_sec;
    float signal;
};

struct FeeConfig {
    float relative_fee;
    float fixed_fee;
    float minimum_fee;
};

// Trading account configurration
struct AccountConfig {
    // starting BTC balance
    float start_base_balance;
    // starting USD balance
    float start_quote_balance;
    // smallest denomination unit for base currency balance.
    float base_unit;
    // smalllest denomination unit for quote currency balance.
    float quote_unit;
    // Transaction fee for market order
    FeeConfig market_order_fee_config;
    // Transaction fee for stop order
    FeeConfig stop_order_fee_config;
    // Transaction fee for limit order
    FeeConfig limit_order_fee_config;
    // Liquidity for executing market (stop) orders w.r.t. the given OHLC tick
    // from the interval [0; 1].
    // If 1.0 then the market (stop) order will be executed at the opening price
    // (stop order price). This is the best price for the given order.
    // If 0.0 then the buy (sell) order will be executed at the highest (lowest)
    // price of the given OHLC tick. This is the worst price for the given order.
    // Anything in between 0.0 and 1.0 will be linearly interpolated.
    float market_liquidity;
    // Fraction of the OHLC tick volume that will be used to fill the limit order.
    // If the actual traded volume * max_volume_ratio is less than the limit
    // order size, then the limit order will be filled only partially.
    // Not used if zero.
    float max_volume_ratio;
};

struct Order {
    enum class Type {

        MARKET,
        STOP,
        LIMIT,
        Count,
    };

    enum class Side {
        BUY,
        SELL,
        Count,
    };

    // The amount of base (crypto) currency to by buy / sell.
    struct BaseAmount {
        float base_amount;
    };

    /* The (maximum) amount of quote to be spent on buying (or to be received
    when selling) the base (crypto) currency.
    The actual traded amount might be smaller due to exchange fees. */
    struct QuoteAmount {
        float quote_amount;
    };

    using AmountType = std::variant<BaseAmount, QuoteAmount>;

    AmountType amount;
    Type type;
    Side side;
    float price;
};

constexpr std::array<const char *, static_cast<std::size_t>(Order::Side::Count)> get_side_strings() {
    return {"BUY", "SELL"};
}

constexpr std::array<const char *, static_cast<std::size_t>(Order::Type::Count)> get_type_strings() {
    return {
        "MARKET",
        "STOP",
        "LIMIT",
    };
}

constexpr const char *order_side_to_string(Order::Side side) {
    if (static_cast<size_t>(side) < static_cast<size_t>(Order::Side::Count)) {
        return get_type_strings()[static_cast<size_t>(side)];
    } else {
        return "NONE";
    }
}

constexpr const char *order_type_to_string(Order::Type type) {
    if (static_cast<size_t>(type) < static_cast<size_t>(Order::Type::Count)) {
        return get_type_strings()[static_cast<size_t>(type)];
    } else {
        return "NONE";
    }
}

} // namespace back_trader