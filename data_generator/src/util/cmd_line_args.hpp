#pragma once

#include <array>
#include <string_view>

#define MAX_PRICE_DEVIATION_PER_MIN 0.05
#define SAMPLING_RATE_SEC 300
#define TOP_N_GAPS 50
#define LAST_N_OUTLIERS 20
#define COMPRESS_IN_BYTE true
// available data full range
#define START_TIME "2011-09-14"
#define END_TIME "2024-06-13"
#define NONE "NONE"
constexpr std::array<std::pair<std::string_view, std::string_view>, 15> args{
    {{"input_price_history_csv_file", "input_price_history_csv_file"},
     {"input_price_history_binary_file", "input_price_history_binary_file"},
     {"output_price_history_binary_file", "output_price_history_binary_file"},
     {"input_ohlc_history_csv_file", "input_ohlc_history_csv_file"},
     {"input_ohlc_history_binary_file", "input_ohlc_history_binary_file"},
     {"output_ohlc_history_binary_file", "output_ohlc_history_binary_file"},
     {"input_side_history_csv_file", "input_side_history_csv_file"},
     {"output_side_history_binary_file", "output_side_history_binary_file"},
     {"start_time", "start_time"},
     {"end_time", "end_time"},
     {"max_price_deviation_per_min", "max_price_deviation_per_min"},
     {"sampling_rate_sec", "sampling_rate_sec"},
     {"top_n_gaps", "top_n_gaps"},
     {"last_n_outliers", "last_n_outliers"},
     {"compress_in_byte", "compress_in_byte"}}};

constexpr std::string_view get_value(std::string_view key) {
    for (const auto &val : args) {
        if (val.first == key) {
            return val.second;
        }
    }
    return "NONE";
}

constexpr bool arg_valid(std::string_view key) { return get_value(key) != NONE; }
