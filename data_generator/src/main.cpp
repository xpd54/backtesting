#include "../../src/base/base.hpp"
#include "binary_io/binary_read_write.hpp"
#include "common_util/Logger.hpp"
#include "common_util/memory_map_util.hpp"
#include "common_util/time_util.hpp"
#include "util/cmd_line_args.hpp"
#include "util/quick_log.hpp"
#include <common_util.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
using namespace common_util;
using namespace back_trader;

namespace back_trader {

PriceHistory read_price_history_from_csv_file(const std::string &file_name, const std::time_t start_time,
                                              const std::time_t end_time) {
    const std::time_t latency_start_time = std::time(nullptr);
    logInfo("Reading price from csv file: -" + file_name);

    // Memory map the file as string_view
    common_util::RMemoryMapped<char> read_file(file_name);
    const char *begin = read_file.begin();
    size_t view_size = read_file.size();
    std::string_view input_file_view = std::string_view(begin, view_size);

    // have a lambda to find location of next new line
    auto new_line_position = [&](uint64_t start_pos) {
        uint64_t found = input_file_view.find('\n', start_pos);
        return found;
    };

    PriceHistory price_history;
    // Creat TVP object and push it to PriceHistory
    int64_t time;
    float price;
    float volume;
    uint64_t start = 0;
    uint64_t found = view_size;
    while (start < view_size) {
        std::string temp_hold;
        found = input_file_view.find(',', start);
        temp_hold = input_file_view.substr(start, found - start);
        time = std::stoul(temp_hold);
        // skip the line if time is not valid with start time
        if (start_time > 0 && time < start_time) {
            start = new_line_position(start) + 1;
            continue;
        }
        // have reached to the limit of end time
        if (end_time > 0 && time >= end_time) {
            break;
        }
        start = found + 1;

        found = input_file_view.find(',', start);
        temp_hold = input_file_view.substr(start, found - start);
        start = found + 1;
        price = std::stof(temp_hold.data());

        found = new_line_position(start);
        temp_hold = input_file_view.substr(start, found - start);
        start = found + 1;
        volume = std::stof(temp_hold.data());
        price_history.push_back({time, price, volume});
        if (!(price_history.size() % 1000000))
            std::cout << "so far " << price_history.size() << " " << time << '\n';
    }
    const std::time_t latency_end_time = std::time(nullptr);
    const size_t total_record = price_history.size();

    logInfo("Loaded " + std::to_string(total_record) + " records in " +
            std::to_string(latency_end_time - latency_start_time) + " sec");
    return price_history;
}

PriceHistory read_price_histry_from_binary_file(const std::string &file_name, const std::time_t start_time,
                                                const std::time_t end_time) {
    return read_history_from_binary_file<PriceRecord>(file_name, start_time, end_time, [](const PriceRecord &record) {
        if (record.price <= 0) {
            logError("Invalid Price");
            return false;
        }
        if (record.volume < 0) {
            logError("Invalid volume");
            return false;
        }
        return true;
    });
}

// Read OHLC input file from csv
OhlcHistory read_ohlc_history_from_csv_file(const std::string &file_name, const std::time_t start_time,
                                            const std::time_t end_time) {
    const std::time_t latency_start_time = std::time(nullptr);
    logInfo("Reading OHLC history from csv file " + file_name);
    common_util::RMemoryMapped<char> read_file(file_name);
    const char *begin = read_file.begin();
    size_t view_size = read_file.size();
    std::string_view input_file_view = std::string_view(begin, view_size);
    // have a lambda to find location of next new line
    auto new_line_pos = [&](uint64_t &start_pos) { return input_file_view.find('\n', start_pos); };
    auto new_comma_pos = [&](uint64_t &start_pos) { return input_file_view.find(',', start_pos); };
    auto next_comma_value = [&](uint64_t &start_pos, uint64_t &found_pos) {
        found_pos = new_comma_pos(start_pos);
        std::string temp_hold(input_file_view.substr(start_pos, found_pos));
        start_pos = found_pos + 1;
        return std::stof(temp_hold);
    };
    auto last_comma_value = [&](uint64_t &start_pos, uint64_t &found_pos) {
        found_pos = new_line_pos(start_pos);
        std::string temp_hold(input_file_view.substr(start_pos, found_pos));
        start_pos = found_pos + 1;
        return std::stof(temp_hold);
    };

    uint64_t row = 0;
    int64_t timestamp_sec_prev = 0;

    // ohlc
    int64_t timestamp_sec = 0;
    float open = 0;
    float high = 0;
    float low = 0;
    float close = 0;
    float volume = 0;
    OhlcHistory ohlc_history;

    uint64_t start = 0;
    uint64_t found = view_size;

    while (start < view_size) {
        std::string temp_hold;

        found = new_comma_pos(start);
        temp_hold = input_file_view.substr(start, found);
        timestamp_sec = std::stol(temp_hold);
        // Validate timestamp
        if (start > 0 && timestamp_sec < start_time) {
            continue;
        }
        if (end_time > 0 && timestamp_sec >= end_time) {
            break;
        }
        if (timestamp_sec <= 0 || timestamp_sec < timestamp_sec_prev) {
            logError("Invalid timestamp on line " + std::to_string(row));
            break;
        }
        start = found + 1;

        open = next_comma_value(start, found);
        high = next_comma_value(start, found);
        low = next_comma_value(start, found);
        close = next_comma_value(start, found);
        volume = last_comma_value(start, found);

        if (open <= 0 || high <= 0 || low <= 0 || close <= 0 || low > open || low > high || low > close ||
            high < open || high < close) {
            logError("Invalid OHLC prices on line " + std::to_string(row));
            break;
        }
        if (volume < 0) {
            logError("Invalid volume on the line" + std::to_string(row));
            break;
        }
        timestamp_sec_prev = timestamp_sec;
        ohlc_history.push_back({timestamp_sec, open, high, low, close, volume});
    }
    const std::time_t latency_end_time = std::time(nullptr);
    const size_t total_record = ohlc_history.size();
    logInfo("Loaded " + std::to_string(total_record) + " OHLC ticks in " +
            std::to_string(latency_end_time - latency_start_time) + " sec");
    return ohlc_history;
}

} // namespace back_trader

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
    const bool read_ohlc_history = !input_ohlc_history_csv_file.empty() || !input_ohlc_history_binary_file.empty();
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
    PriceHistory history;
    if (!input_price_history_csv_file.empty())
        history = read_price_history_from_csv_file(input_price_history_csv_file, start_time, end_time);
    if (!output_price_history_binary_file.empty())
        write_history_to_binary_file(history, output_price_history_binary_file);

    if (!input_price_history_binary_file.empty())
        history = read_price_histry_from_binary_file(input_price_history_binary_file, start_time, end_time);
    OhlcHistory ohlc_history;
    if (!input_ohlc_history_csv_file.empty())
        ohlc_history = read_ohlc_history_from_csv_file(input_ohlc_history_csv_file, start_time, end_time);
    std::cout << ohlc_history.size() << " " << ohlc_history.front().timestamp_sec << '\n';
    std::cout << ohlc_history.size() << " " << ohlc_history.back().timestamp_sec << '\n';

    logger.close();
    return 0;
}