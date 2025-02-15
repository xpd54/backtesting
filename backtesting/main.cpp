#include "common_util/Logger.hpp"
#include "execution/simulation_executor.hpp"
#include "execution/simulation_types.hpp"
#include "logs/simulation_log.hpp"
#include "simulators/simulator_factory.hpp"
#include "util/quick_log.hpp"
#include <base_header.hpp>
#include <common_util.hpp>
#include <cstddef>
#include <fstream>
#include <memory>
#include <string>
#include <utility>
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

void print_account_config(AccountConfig &account_config) {
    logInfo(string_format(                                                           // nowrap
        "\n-- Current Account Configuration --",                                     // nowrap
        "\n   start_base_balance: ", account_config.start_base_balance,              // nowrap
        "\n   start_quote_balance: ", account_config.start_quote_balance,            // nowrap
        "\n   base_uni: ", account_config.base_unit,                                 // nowrap
        "\n   quote_uni: ", account_config.quote_unit,                               // nowrap
        "\n   relative fee : ", account_config.market_order_fee_config.relative_fee, // nowrap
        "\n   fixed fee : ", account_config.market_order_fee_config.fixed_fee,       // nowrap
        "\n   minimum fee : ", account_config.market_order_fee_config.minimum_fee,   // nowrap
        "\n   market_liquidity: ", account_config.market_liquidity,                  // nowrap
        "\n   max_volume_ratio: ", account_config.max_volume_ratio));
}

template <typename T>
std::vector<T> read_from_binary_file(const std::string &binary_file_name, std::time_t start_time,
                                     std::time_t end_time) {
    std::vector<T> history = read_history_from_binary_file<T>(binary_file_name, start_time, end_time, nullptr);
    const std::vector<T> history_subset_with_time = history_subset_copy(history, start_time, end_time);
    Logger::get_instance()(Logger::Severity::INFO) << "Selected " << // nowrap
        history_subset_with_time.size() <<                           // nowrap
        " records within the time period: [" <<                      // nowrap
        formate_time_utc(start_time) <<                              // nowrap
        " - " <<                                                     // nowrap
        formate_time_utc(end_time) <<                                // nowrap
        ')' <<                                                       // nowrap
        Logger::endl;
    return history_subset_with_time;
}

std::unique_ptr<SimulationLogger> get_logger_instance(std::string &output_simulation_log_file,
                                                      std::string &output_simulator_log_file) {
    auto simulation_log_stream = get_log_stream(output_simulation_log_file);
    auto simulator_log_stream = get_log_stream(output_simulator_log_file);
    return std::make_unique<SimulationLogger>(simulation_log_stream.get(), simulator_log_stream.get());
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
    Logger &logger = Logger::get_instance();
    logger(Logger::Severity::INFO)
        << "--------------- Time Period ---------------  Strategy gain | Base gain(HODL) | Score | volatility"
        << Logger::endl;
    for (const SimulatorEvaluationResult::TimePeriod &period : sim_evaluation_result.periods) {
        logger(Logger::Severity::INFO) << '[' << formate_time_utc(period.start_timestamp_sec) << " - " << // nowrap
            formate_time_utc(period.end_timestamp_sec) << "):  " <<                                       // nowrap
            ((period.final_gain - 1.00f) * 100.0f) << "%  | " <<                                          // nowrap
            (period.base_final_gain - 1.00f) * 100.0f << "%  | " <<                                       // nowrap
            (period.final_gain / period.base_final_gain) << "  | " <<                                     // nowrap
            period.result.simulator_volatility << "  | " <<                                               // nowrap
            period.result.base_volatility << Logger::endl;
    }
}
} // namespace back_trader

using namespace back_trader;
int main(int argc, char *argv[]) {
    /*Initialize logger*/
    Logger &logger = Logger::get_instance();
    logger.init("log_file.log", Logger::Severity::DEBUG, Logger::OutputMode::UBIQUITOUS);
    logger.open();
    /* ----------------- Validate command line argument ------------------*/
    std::unordered_map<std::string, std::string> arg_map = get_command_line_argument(argc, argv);
    for (auto &val : arg_map) {
        if (!arg_valid(val.first)) {
            logger(Logger::Severity::ERROR) << val.first << " "
                                            << "argument is not valid" << Logger::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    /* ------------------ Get command line arguments ---------------------*/
    std::time_t start_time = (arg_map["start_time"] == "" ? convert_time_string(START_TIME) // nowrap
                                                          : convert_time_string(arg_map["start_time"]));

    std::time_t end_time = arg_map["end_time"] == "" ? convert_time_string(END_TIME) // nowrap
                                                     : convert_time_string(arg_map["end_time"]);

    float start_base_balance = arg_map["start_base_balance"] == "" ? 1.0f : std::stof(arg_map["start_base_balance"]);
    float start_quote_balance = arg_map["start_quote_balance"] == "" ? 0.0f : std::stof(arg_map["start_quote_balance"]);
    float market_liquidity = arg_map["market_liquidity"] == "" ? 0.5 : std::stof(arg_map["market_liquidity"]);
    float max_volume_ratio = arg_map["max_volume_ratio"] == "" ? 0.5 : std::stof(arg_map["max_volume_ratio"]);

    int evaluation_period_months = arg_map["evaluation_period_months"] == ""
                                       ? 0 // nowrap
                                       : std::stoi(arg_map["evaluation_period_months"]);

    bool evaluate_combination = arg_map["evaluate_combination"] == "" ? EVALUATE_COMBINATION // nowrap
                                                                      : std::stoi(arg_map["evaluate_combination"]);

    std::string strategy_name = arg_map["simulator"] == "" ? "rebalancing" : arg_map["simulator"];
    std::string input_price_history_binary_file = arg_map["input_price_history_binary_file"];
    std::string output_account_log_file = arg_map["output_account_log_file"];
    std::string output_simulator_log_file = arg_map["output_simulator_log_file"];

    /* --------------------------- Read price history -------------------------*/
    OhlcHistory ohlc_history = read_from_binary_file<OhlcTick>(input_price_history_binary_file, start_time, end_time);
    // TODO:- Read and handle fear and greed

    AccountConfig account_config =
        get_account_config(start_base_balance, start_quote_balance, market_liquidity, max_volume_ratio);
    print_account_config(account_config);
    SimEvaluationConfig sim_evaluation_config;
    sim_evaluation_config.start_timestamp_sec = start_time;
    sim_evaluation_config.end_timestamp_sec = end_time;
    sim_evaluation_config.evaluation_period_months = evaluation_period_months;

    // Take timestamp for latency check
    const std::time_t latency_start = std::time(nullptr);

    if (evaluate_combination) {
        sim_evaluation_config.fast_execute = true;
        logger(Logger::Severity::INFO) << "Evaluation Combination of simulators" << Logger::endl;
        std::vector<std::unique_ptr<SimulatorDispatcher>> sim_dispatchers =
            get_combination_of_simulators(strategy_name);

        std::vector<SimulatorEvaluationResult> simulation_evaluation_result =
            evaluate_combination_of_trade_simulators(account_config,        // nowrap
                                                     sim_evaluation_config, // nowrap
                                                     ohlc_history,          // nowrap
                                                     nullptr,               // nowrap
                                                     sim_dispatchers);

        std::sort(simulation_evaluation_result.begin(), simulation_evaluation_result.end(),
                  [](const SimulatorEvaluationResult &left, const SimulatorEvaluationResult &right) {
                      return left.score > right.score;
                  });
        print_combination_of_trade_evaluation_results(simulation_evaluation_result, 30);
    } else {

        sim_evaluation_config.fast_execute = false;
        std::unique_ptr<SimulatorDispatcher> sim_dispather = get_trade_simulator(strategy_name);
        logInfo(string_format(sim_dispather->get_names(), " evaluation"));
        logger(Logger::Severity::INFO) << sim_dispather->get_names() << " evaluation" << Logger::endl;
        std::unique_ptr<std::ofstream> account_log_stream = get_log_stream(output_account_log_file);
        std::unique_ptr<std::ofstream> simulator_log_stream = get_log_stream(output_simulator_log_file);
        SimulationLogger logger(account_log_stream.get(), simulator_log_stream.get());
        SimulatorEvaluationResult simulation_result = evaluate_trade_simulator(account_config,        // nowrap
                                                                               sim_evaluation_config, // nowrap
                                                                               ohlc_history,          // nowrap
                                                                               nullptr,               // nowrap
                                                                               *sim_dispather,        // nowrap
                                                                               &logger);
        print_trade_simulator_evaluation_result(simulation_result);
    }

    // Take end time stamp for latency check
    std::time_t latency_end = std::time(nullptr);
    logger(Logger::Severity::INFO) << "Evaluated in " << duration_to_string(latency_end - latency_start) << "sec"
                                   << Logger::endl;
}