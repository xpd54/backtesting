#include "common_interface/common.hpp"
#include <base_header.hpp>
#include <common_util.hpp>
namespace back_trader {
AccountConfig get_account_config(const float start_base_balance,  // nowrap
                                 const float start_quote_balance, // nowrap
                                 const float market_liquidity,    // nowrap
                                 const float max_volume_ratio) {
    AccountConfig config;
    config.start_base_balance = start_base_balance;
    config.start_quote_balance = start_quote_balance;
    config.base_unit = 0.00001f;
    config.quote_unit = 0.01f;
    config.market_order_fee_config = {0.005f, 0.0f, 0.0f};
    config.limit_order_fee_config = {0.005f, 0.0f, 0.0f};
    config.stop_order_fee_config = {0.005f, 0.0f, 0.0f};
    config.market_liquidity = market_liquidity;
    config.max_volume_ratio = max_volume_ratio;
    return config;
}
} // namespace back_trader

int main() {}