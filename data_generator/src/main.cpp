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
    std::string input_price_history_delimited_file = arg_map["input_price_history_delimited_file"];
    std::string output_price_history_delimited_file = arg_map["output_price_history_delimited_file"];

    std::string input_ohlc_history_csv_file = arg_map["input_ohlc_history_csv_file"];
    std::string input_ohlc_history_delimited_file = arg_map["input_ohlc_history_delimited_file"];
    std::string output_ohlc_history_delimited_file = arg_map["output_ohlc_history_delimited_file"];

    std::string input_side_history_csv_file = arg_map["input_side_history_csv_file"];
    std::string output_side_history_delimited_file = arg_map["output_side_history_delimited_file"];

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
    logger.log("Selected time period:- " + formate_time_utc(start_time) + " " + formate_time_utc(end_time),
               Logger::Severity::INFO);
    logger.close();
    return 0;
}