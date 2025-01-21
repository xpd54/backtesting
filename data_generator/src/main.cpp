#include "util/cmd_line_args.hpp"
#include <common_util.hpp>
#include <cstdlib>
#include <string>
#include <unordered_map>
using common_util::Logger;
int main(int argc, char *argv[]) {
    /*Initialize logger*/
    common_util::Logger &logger = common_util::Logger::get_instance();
    logger.init();
    logger.open();
    /*validate arguments*/
    std::unordered_map<std::string, std::string> arg_map = common_util::get_command_line_argument(argc, argv);
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
    std::string start_time = arg_map["start_time"];
    std::string end_time = arg_map["end_time"];

    logger.close();
    return 0;
}