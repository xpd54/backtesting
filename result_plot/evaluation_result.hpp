#pragma once
#include <vector>
// OHLC and Account
struct EvaluationResult {
    double timestamp_sec;
    double open;
    double high;
    double low;
    double close;
    double volume;
    double base_balance;
    double quote_balance;
    double total_fee;
    double portfolio_value; // calculate portfolio_value (base*close + quote)
    double beta; // Beta value shows how much of portfolio value are allocated into base currency or exposure to market
};

std::vector<EvaluationResult> read_result_file(const std::string &file_name);
void write_results_to_temp_file(const std::vector<EvaluationResult> &results, const std::string &temp_filename,
                                int count_to_plot);
void plot_data(const std::string &temp_filename);