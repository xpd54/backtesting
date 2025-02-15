#pragma once
#include <base_header.hpp>
#include <cstdint>
#include <ctime>
#include <string>
namespace back_trader {
// Result of trade simulation over a region of the OHLC history.
struct SimulationResult {
    // balance in base currency before trade
    float start_base_balance;
    // balance in quote currency before trade
    float start_quote_balance;
    // balance in base currency after trade
    float end_base_balance;
    // balance in quote currency after trade
    float end_quote_balance;
    // base currency price in quote currency before trade
    float start_price;
    // base currency price in quote currency after trade
    float end_price;
    // totle value in quote currency before trade
    float start_value;
    // total value in quote currency after trade
    float end_value;
    // total number of orders during region of OHLC
    int32_t total_order;
    // total fee in quote currecy
    float total_fee;

    // TODO :- Implement market volatility indicator
    // https://docs.google.com/spreadsheets/d/1lQTz1Fad2TIi42-jBjwd-42UWKsIuSP1/
    float base_volatility;
    float simulator_volatility;
};

struct SimEvaluationConfig {
    // Start timestamp (in sec).
    std::time_t start_timestamp_sec;
    // Ending timestamp (in sec).
    std::time_t end_timestamp_sec;
    // execution period (in months).
    int32_t evaluation_period_months;
    // When true, avoids computing volatility (to speed up the computation).
    // This is useful when evaluating a combination of traders in parallel.
    bool fast_execute;
};

// Result of trade simulation over given execution config.
struct SimulatorEvaluationResult {
    AccountConfig account_config;
    SimEvaluationConfig sim_evaluation_config;
    // strategy_name.
    std::string name;
    // execution over a specific period.
    struct TimePeriod {
        // Start timestamp of the execution period (included).
        std::time_t start_timestamp_sec;
        // End timestamp of the execution period (excluded).
        std::time_t end_timestamp_sec;
        SimulationResult result;
        // percent gain (after fees).
        float final_gain;
        // percent gain of the baseline (Buy and HODL) method.
        float base_final_gain;
    };
    std::vector<TimePeriod> periods;
    float score;
    // percent gain of the trade (after fees).
    float avg_gain;
    // Average percent gain of the baseline (Buy and HODL) method.
    float avg_base_gain;
    // Average total number of executed orders.
    float avg_total_executed_orders;
    // Average total trader fees in quote currency.
    float avg_total_fee;
};

} // namespace back_trader