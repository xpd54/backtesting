/*Common class, struct, data structure used across data_generator and backtesting trade*/

#include <cstdint>

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