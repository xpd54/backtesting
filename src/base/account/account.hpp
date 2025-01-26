#pragma once
#include "../common_interface/common.hpp"
#include "price_history/history_subset.hpp"
#include <limits>

namespace back_trader {
struct Account {
    // balance in BTC (Crypto)
    float base_balance = 0;
    // balance in Quote currency (USD)
    float quote_balance = 0;

    /*Total transaction fee over all executed order. Transaction fee is based on fee_config. It's totle quote ammount.
     * It's subtracted from quote currency balance.*/
    float total_fee = 0;

    /*Smallest unit of crypto currency. example 0.00000001BTC =  1 Satoshi */
    float base_unit = 0;

    /*Smalles indivisible unit for quote currency balance.*/
    float quote_unit = 0;

    /*
    If liquidity is 1.0f. Order will get executed at opening price (or stop order price) of given OHLC tick.
    If it's 0.0f thank market order buy/sell will get executed at highest/lowest price.
    Basically there aren't enough buyer or seller at the given price which will drive the buy at highest and sell at
    lowest. Anything between 0.0f and 1.0f is linear.
    */
    float market_liquidity = 1.0f;

    /* Fraction of OHLC tick volume that will be used to fill the limit order for the price. If volume (TOHLCV) *
     * max_volume_ration < limit order volume than limit order would be filled partially in this given OHLC tick.*/
    float max_volume_ratio = 0.0f;

    /* Initialize the account with config*/
    void init_account(const AccountConfig &account_confg);

    /* Fee in quote currency according to fee_config and given quote currency ammount in the transaciton.*/
    float get_fee(const FeeConfig &fee_config, float quote_ammount) const;

    /* Maximum tradeable base currency (BTC) ammount (volume) based on max_volume_ratio on given ohlc_tick*/
    float get_max_base_ammount(const OhlcTick &ohlc_tick) const;

    /*----------------------------------------- Price Of Type Of Orders ---------------------------------------------*/

    /* Price of market buy order based of market liquidity when executed over given OHLC tick */
    float get_market_buy_price(const OhlcTick &ohlc_tick) const;

    /* Price of market sell order based of market liquidity executed over given OHLC tick*/
    float get_market_sell_price(const OhlcTick &ohlc_tick) const;

    /* Price of Stop Buy order based of market liquidity executed over given OHLC tick and Stop price*/
    float get_stop_buy_price(const OhlcTick &ohlc_tick, float stop_price) const;

    /* Price of Stop sell order based of market liquidity executed over given OHLC tick and Stop price.*/
    float get_stop_sell_price(const OhlcTick &ohlc_tick, float stop_price) const;

    /*------------------------------------- Order At Specific Price ---------------------------------------------*/

    /* Buy ammount of base currency at given price*/
    bool buy_base_currency(const FeeConfig &fee_config, float base_ammount, float price);

    /* Buy as much of base currency(BTC) as possible at given price. Using most quote_ammount (USD), It's possible to
     * buy most max_base_ammount base.*/
    bool buy_at_quote(const FeeConfig &fee_config, float quote_ammount, float price,
                      float max_base_ammount = std::numeric_limits<float>::max());

    /* Sell ammount of base currency at given price.*/
    bool sell_base_currency(const FeeConfig &fee_config, float base_ammount, float price);

    /* Sell as much of base currency as possible at given price. Getting at most quote_ammout. It's possible to sell
     * max_base_ammount at the price*/
    bool sell_at_quote(const FeeConfig &fee_config, float quote_ammount, float price,
                       float max_base_ammount = std::numeric_limits<float>::max());

    /*------------- Execute Market Orders (Buy/Sell at market price itself) -----------------------------------*/

    /* Execute market buy order of base_ammount (volume of btc). (I want to buy 0.04 BTC)*/
    bool market_buy(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float base_ammout);

    /* Execute market buy order spending most of quote_ammount (USD). (I want to buy $100 worth of BTC/ may be just for
     * $94 depending on base_unit)*/
    bool market_buy_at_quote(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float quote_ammount);

    /* Execute market sell order of base_ammount (BTC). (I want to sell 0.04 BTC)*/
    bool market_sell(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float base_ammount);

    /* Execute market sell order receiving at most quote_ammount. (I want to sell $100 worth of BTC)*/
    bool market_sell_at_quote(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float quote_ammount);

    /*------------- Execute Stop Orders (Buy/Sell When Market Rise/Fall To Stop Price) ---------------------------*/
    // (https://www.investopedia.com/terms/s/stoporder.asp)
    /* Execute stop buy order of base ammount at stop_price. Stop Buy only get executed when price Rise to stop_price
     * price. (When given OHLC high price is >= stop_price) */
    bool stop_buy(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float base_amount, float stop_price);

    /* Execute stop buy order of quote_ammount of quote currency at stop_price. Stop Buy only get executed when price
     * RISE to stop_price price. (When given OHLC high price is >= stop_price) */
    bool stop_buy_at_quote(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float quote_amount,
                           float stop_price);

    /* Execute stop sell order of base_ammount in base currency (BTC) at stop_price. Stop Sell only get executed when
     * price FALLS to stop_price price. (When given OHLC low price is <= stop_price) */
    bool stop_sell(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float base_amount, float stop_price);

    /* Execute stop sell order of quote_ammount in quote currency (USD) at stop_price. Stop Sell only get executed when
     * price FALLS to stop_stop price. (When given OHLC low price is <= stop_price) */
    bool stop_sell_at_quote(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float quote_amount,
                            float stop_price);

    /*------------- Execute Limit Orders (Buy/Sell When Market Rise/Fall To Stop Price) ---------------------------*/

    /* Execute limit buy order of base ammount at limit_price. Limit Buy only get executed when price FALLS to
     * limit_price price. (When given OHLC low price is <= limit_price) */
    bool limit_buy(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float base_amount, float limit_price);

    /* Execute limit buy order of quote_ammount of quote currency at limit_price. Limit Buy only get executed when price
     * FALLS to limit_price price. (When given OHLC low price is <= limit_price) */
    bool limit_buy_at_quote(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float quote_amount,
                            float limit_price);

    /* Execute limit sell order of base_ammount in base currency (BTC) at limit_price. Limit Sell only get executed when
     * price RISE to limit_price price. (When given OHLC high price is >= limit_price) */
    bool limit_sell(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float base_amount, float limit_price);

    /* Execute limit sell order of quote_ammount in quote currency (USD) at limit_price. Limit Sell only get executed
     * when price RISE to limit_price price. (When given OHLC high price is >= limit_price) */
    bool limit_sell_at_quote(const FeeConfig &fee_config, const OhlcTick &ohlc_tick, float quote_amount,
                             float limit_price);

    /*------------- Execute General Orders---------------------------*/
    // Execute the order over the given ohlc_tick.
    bool execute_order(const AccountConfig &account_config, const Order &order, const OhlcTick &ohlc_tick);
};
} // namespace back_trader