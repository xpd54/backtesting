#include "common_util/Logger.hpp"
#include "common_util/time_util.hpp"
#include "util/cmd_line_args.hpp"
#include <common_util.hpp>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <string>
#include <unordered_map>
using namespace common_util;
int main(int argc, char *argv[]) {
    /*Initialize logger*/
    Logger &logger = Logger::get_instance();
    logger.init("log_file.log", Logger::Severity::DEBUG, Logger::OutputMode::CONSOLE);
    logger.open();
    /*validate arguments*/
    std::unordered_map<std::string, std::string> arg_map = get_command_line_argument(argc, argv);
    for (auto &val : arg_map) {
        if (!arg_valid(val.first)) {
            logger.log(val.first + " arg is not valid", Logger::Severity::ERROR);
            std::exit(EXIT_FAILURE);
        }
    }
    std::string input_price_history_csv_file = arg_map["input_price_history_csv_file"];
    std::string input_price_history_binary_file = arg_map["input_price_history_binary_file"];
    std::string output_price_history_binary_file = arg_map["output_price_history_binary_file"];

    std::string input_ohlc_history_csv_file = arg_map["input_ohlc_history_csv_file"];
    std::string input_ohlc_history_binary_file = arg_map["input_ohlc_history_binary_file"];
    std::string output_ohlc_history_binary_file = arg_map["output_ohlc_history_binary_file"];

    std::string input_side_history_csv_file = arg_map["input_side_history_csv_file"];
    std::string output_side_history_binary_file = arg_map["output_side_history_binary_file"];

    double max_price_deviation_per_min = arg_map["max_price_deviation_per_min"] == ""
                                             ? MAX_PRICE_DEVIATION_PER_MIN
                                             : std::stod(arg_map["max_price_deviation_per_min"]);
    int sampling_rate_sec =
        arg_map["sampling_rate_sec"] == "" ? SAMPLING_RATE_SEC : std::stoi(arg_map["sampling_rate_sec"]);
    int top_n_gaps = arg_map["top_n_gaps"] == "" ? TOP_N_GAPS : std::stoi(arg_map["top_n_gaps"]);
    int last_n_outliers = arg_map["last_n_outliers"] == "" ? LAST_N_OUTLIERS : std::stoi(arg_map["last_n_outliers"]);
    bool compress_in_byte =
        arg_map["compress_in_byte"] == "" ? COMPRESS_IN_BYTE : std::stoi(arg_map["compress_in_byte"]);
    std::time_t start_time =
        (arg_map["start_time"] == "" ? convert_time_string(START_TIME) : convert_time_string(arg_map["start_time"]));
    std::time_t end_time =
        arg_map["end_time"] == "" ? convert_time_string(END_TIME) : convert_time_string(arg_map["end_time"]);
    logger.log("Selected time period:- [" + formate_time_utc(start_time) + "] - [" + formate_time_utc(end_time) + "]",
               Logger::Severity::INFO);

    // Error :- Getting two input at same time
    if (!input_price_history_csv_file.empty() && !input_price_history_binary_file.empty()) {
        logger.log("Cannot process two input price history files", Logger::Severity::ERROR);
        std::exit(EXIT_FAILURE);
    }

    // Error :- Getting two ohlc history files
    if (!input_ohlc_history_csv_file.empty() && !input_ohlc_history_binary_file.empty()) {
        logger.log("Cannot process two input OHLC history files", Logger::Severity::ERROR);
        std::exit(EXIT_FAILURE);
    }

    const bool read_price_history = !input_price_history_csv_file.empty() || !input_price_history_binary_file.empty();
    const bool read_ohlc_history = input_ohlc_history_csv_file.empty() || input_ohlc_history_binary_file.empty();
    const bool read_side_history = !input_side_history_csv_file.empty();

    const int num_history_files =
        (read_price_history ? 1 : 0) + (read_ohlc_history ? 1 : 0) + (read_side_history ? 1 : 0);

    // we would ignore side_history (fear & greed) for now
    if (num_history_files > 1) {
        logger.log("Cannot read more than one input history file", Logger::Severity::ERROR);
        std::exit(EXIT_FAILURE);
    }

    if (num_history_files == 0) {
        logger.log("Input history file not specified", Logger::Severity::ERROR);
        std::exit(EXIT_FAILURE);
    }
    logger.close();
    return 0;
}