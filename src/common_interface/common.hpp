/*Common class, struct, data structure used across data_generator and backtesting trade*/

#include <cstdint>
namespace backtest {

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
    enum struct Type {
        MARKET,
        STOP,
        LIMIT,
    };

    enum struct Side {
        BUY,
        SELL,
    };

    struct oneof_amount {
        // The amount of base (crypto) currency to by buy / sell.
        float base_amount;
        // The (maximum) amount of quote to be spent on buying (or to be received
        // when selling) the base (crypto) currency.
        // The actual traded amount might be smaller due to exchange fees.
        float quote_amount;
    };
};
} // namespace backtest