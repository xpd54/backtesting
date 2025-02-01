#include "common_interface/common.hpp"
#include "common_util/Logger.hpp"
#include "common_util/string_format_util.hpp"
#include "common_util/time_util.hpp"
#include "execution/simulation_types.hpp"
#include "logs/simulation_log.hpp"
#include "price_history/history_subset.hpp"
#include "util/binary_io/binary_read_write.hpp"
#include "util/quick_log.hpp"
#include <base_header.hpp>
#include <common_util.hpp>
#include <cstddef>
#include <memory>
#include <optional>
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

template <typename T>
std::vector<T> read_from_binary_file(const std::string &binary_file_name, std::time_t start_time,
                                     std::time_t end_time) {
    std::vector<T> history = read_history_from_binary_file<T>(binary_file_name, start_time, end_time, nullptr);
    const std::vector<T> history_subset_with_time = history_subset_copy(history, start_time, end_time);
    logInfo(string_format("Selected ", history_subset_with_time.size(), " records within the time period: [",
                          formate_time_utc(start_time), " - ", formate_time_utc(end_time), ')'));
    return history_subset_with_time;
}

std::unique_ptr<SimulationLogger> get_logger_instance(Logger &common_logger) {
    return std::make_unique<SimulationLogger>(common_logger);
}

void print_combination_of_trade_evaluation_results(const std::vector<SimulatorEvaluationResult> &evaluation_results,
                                                   size_t top) {
    int it_count = std::min(evaluation_results.size(), top);
    auto result_it = evaluation_results.begin();
    while (it_count) {
        logInfo(string_format(result_it->name, ": ", result_it->score));
        ++result_it;
        --it_count;
    }
}

void print_trade_simulator_evaluation_result(const SimulatorEvaluationResult &sim_evaluation_result) {
    logInfo(string_format("-------------- Time Period --------------  Trader & Base gain score volatility"));
    for (const SimulatorEvaluationResult::TimePeriod &period : sim_evaluation_result.periods) {
        logInfo(string_format('[', formate_time_utc(period.start_timestamp_sec), " - ", // nowrap
                              formate_time_utc(period.end_timestamp_sec), ')',          // nowrap
                              ((period.final_gain - 1.0f) * 100.0f), '%',               // nowrap
                              (period.base_final_gain - 1.0f) * 100.0f, '%',            // nowrap
                              (period.final_gain / period.base_final_gain),             // nowrap
                              period.result.simulator_volatility,                       // nowrap
                              period.result.base_volatility));
    }
}
} // namespace back_trader

int main() {}