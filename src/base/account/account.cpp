#include "account.hpp"
#include "common_interface/common.hpp"
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

float Account::get_fee(const FeeConfig &fee_config, float quote_ammount) const {}
} // namespace back_trader