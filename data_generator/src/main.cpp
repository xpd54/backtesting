#include "binary_io/binary_read_write.hpp"
#include "common_util/Logger.hpp"
#include "common_util/memory_map_util.hpp"
#include "common_util/string_format_util.hpp"
#include "common_util/time_util.hpp"
#include "price_history/price_history.hpp"
#include "util/cmd_line_args.hpp"
#include "util/quick_log.hpp"
#include <base_header.hpp>
#include <cassert>
#include <common_util.hpp>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <string>
#include <string_view>
#include <unordered_map>
using namespace common_util;
using namespace back_trader;

namespace back_trader {

PriceHistory read_price_history_from_csv_file(const std::string &file_name, const std::time_t start_time,
                                              const std::time_t end_time) {
    const std::time_t latency_start_time = std::time(nullptr);
    logInfo(string_format("Reading price from csv file:- ", file_name));

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
    }
    const std::time_t latency_end_time = std::time(nullptr);
    const size_t total_record = price_history.size();

    logInfo(string_format("Loaded ", total_record, " records in ",
                          duration_to_string(latency_end_time - latency_start_time)));
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
    logInfo(string_format("Reading OHLC history from:- ", file_name));
    common_util::RMemoryMapped<char> read_file(file_name);
    const char *begin = read_file.begin();
    size_t view_size = read_file.size();
    std::string_view input_file_view = std::string_view(begin, view_size);

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
    logInfo(string_format("Loaded ", total_record, " OHLC ticks in ",
                          duration_to_string(latency_end_time - latency_start_time)));
    return ohlc_history;
}

OhlcHistory read_ohlc_history_from_binary_file(const std::string &file_name, const std::time_t start_time,
                                               const std::time_t end_time) {
    return read_history_from_binary_file<OhlcTick>(file_name, start_time, end_time, [](const OhlcTick &ohlc_tick) {
        if (ohlc_tick.open <= 0 || ohlc_tick.high <= 0 || ohlc_tick.low <= 0 || ohlc_tick.close <= 0 ||
            ohlc_tick.low > ohlc_tick.open || ohlc_tick.low > ohlc_tick.high || ohlc_tick.low > ohlc_tick.close ||
            ohlc_tick.high < ohlc_tick.open || ohlc_tick.high < ohlc_tick.close) {
            logError("Invalid OHLC prices");
            return false;
        }
        if (ohlc_tick.volume < 0) {
            logError("Invalid volume");
        }
        return true;
    });
}

// Prints the top_n largest (chronologically sorted) price history gaps.
void print_price_history_gaps(const PriceHistory &price_history, size_t top_n) {
    const std::vector<HistoryGap> history_gaps =
        get_price_history_gap(price_history.begin(), price_history.end(), 0, 0, top_n);
    for (const HistoryGap &history_gap : history_gaps) {
        const int64_t gap_duration_sec = history_gap.second - history_gap.first;
        logInfo(string_format(history_gap.first, " ", '[', formate_time_utc(history_gap.first, "%Y-%m-%d %H:%M:%S"),
                              "] - ", history_gap.second, " [",
                              formate_time_utc(history_gap.second, "%Y-%m-%d %H:%M:%S"),
                              "]: ", duration_to_string(gap_duration_sec)));
    }
}

/* Prints a subset of the given price history that covers the last_n outliers.
 Every outlier is surrounded by left_context_size of previous prices (if
 possible) and right_context_size of follow-up prices (if possible). */
void print_outliers_with_context(PriceHistory::const_iterator begin, PriceHistory::const_iterator end,
                                 const std::vector<size_t> &outlier_indices, size_t left_context_size,
                                 size_t right_context_size, size_t last_n) {
    const size_t price_history_size = std::distance(begin, end);
    std::map<size_t, bool> index_to_outlier = get_outlier_indices_with_context(
        outlier_indices, price_history_size, left_context_size, right_context_size, last_n);

    size_t index_prev = 0;
    for (const auto &index_outlier_pair : index_to_outlier) {
        const size_t index = index_outlier_pair.first;
        const bool is_outlier = index_outlier_pair.second;
        assert(index < price_history_size);
        const PriceRecord &price_record = *(begin + index);
        if (index_prev > 0 && index > index_prev + 1) {
            logInfo("   ...");
        }

        logInfo(string_format((is_outlier ? " x" : "  "), ' ', price_record.timestamp_sec, " [",
                              formate_time_utc(price_record.timestamp_sec, "%Y-%m-%d %H:%M:%S"),
                              "]: ", price_record.price, " [", price_record.volume, ']'));
        index_prev = index;
    }
}

// Removes outliers and resamples the price history into OHLC history.
// Keeping Max_price_deviation_fix
OhlcHistory convert_price_history_to_ohlc_history(const PriceHistory &price_history,
                                                  const int sampling_rate_sec = SAMPLING_RATE_SEC) {
    std::vector<size_t> outlier_indices;

    const PriceHistory price_history_clean =
        remove_outliers(price_history.begin(), price_history.end(), MAX_PRICE_DEVIATION_PER_MIN, &outlier_indices);

    logInfo(string_format("Removed ", outlier_indices.size(), " outliers"));
    logInfo(string_format("Last ", LAST_N_OUTLIERS, " outliers:"));
    print_outliers_with_context(price_history.begin(), price_history.end(), outlier_indices, 5, 5, LAST_N_OUTLIERS);
    const OhlcHistory ohlc_history =
        resample(price_history_clean.begin(), price_history_clean.end(), sampling_rate_sec);
    logInfo(
        string_format("Resampled ", price_history_clean.size(), " records to ", ohlc_history.size(), " OHLC ticks"));
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
            logger.log(string_format(val.first, " argument is not valid"), Logger::Severity::ERROR);
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
    logger.log(string_format("Selected time period:- [", formate_time_utc(start_time, "%Y-%m-%d %H:%M:%S"), "] - [",
                             formate_time_utc(end_time, "%Y-%m-%d %H:%M:%S") + "]"),
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

    // Read PriceRecord(TPV) from csv or binary
    PriceHistory price_history = [&]() {
        if (!input_price_history_csv_file.empty()) {
            return read_price_history_from_csv_file(input_price_history_csv_file, start_time, end_time);
        } else if (!input_price_history_binary_file.empty()) {
            return read_price_histry_from_binary_file(input_price_history_binary_file, start_time, end_time);
        }
        return PriceHistory{};
    }();

    // Read OhlcTick(OHLC) from csv or binary
    OhlcHistory ohlc_history = [&]() {
        if (!input_ohlc_history_csv_file.empty()) {
            return read_ohlc_history_from_csv_file(input_ohlc_history_csv_file, start_time, end_time);
        } else if (!input_ohlc_history_binary_file.empty()) {
            return read_ohlc_history_from_binary_file(input_ohlc_history_binary_file, start_time, end_time);
        }
        return OhlcHistory{};
    }();

    // Ignore side history for now

    if (!price_history.empty()) {
        logInfo(string_format("Top ", top_n_gaps, " gaps:"));
        print_price_history_gaps(price_history, top_n_gaps);
    }

    if (!price_history.empty() && ohlc_history.empty() && !output_ohlc_history_binary_file.empty()) {
        ohlc_history = convert_price_history_to_ohlc_history(price_history, sampling_rate_sec);
    }

    if (!price_history.empty() && !output_price_history_binary_file.empty())
        write_history_to_binary_file(price_history, output_price_history_binary_file);

    if (!ohlc_history.empty() && !output_ohlc_history_binary_file.empty())
        write_history_to_binary_file(ohlc_history, output_ohlc_history_binary_file);

    // std::cout << ohlc_history.size() << " " << ohlc_history.front().timestamp_sec << '\n';
    // std::cout << ohlc_history.size() << " " << ohlc_history.back().timestamp_sec << '\n';

    logger.close();
    return 0;
}