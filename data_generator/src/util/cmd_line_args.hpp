#pragma once

#include <array>
#include <string_view>

#define max_price_deviation_per_min 0.05
#define sampling_rate_sec 300
#define top_n_gaps 50
#define last_n_outliers 20
#define compress_in_byte true
// available data full range
#define start_time "2017-01-01"
#define end_time "2021-01-01"
#define NONE "NONE"
constexpr std::array<std::pair<std::string_view, std::string_view>, 15> args{
    {{"input_price_history_csv_file", "input_price_history_csv_file"},
     {"input_price_history_delimited_file", "input_price_history_delimited_file"},
     {"output_price_history_delimited_file", "output_price_history_delimited_file"},
     {"input_ohlc_history_csv_file", "input_ohlc_history_csv_file"},
     {"input_ohlc_history_delimited_file", "input_ohlc_history_delimited_file"},
     {"output_ohlc_history_delimited_file", "output_ohlc_history_delimited_file"},
     {"input_side_history_csv_file", "input_side_history_csv_file"},
     {"output_side_history_delimited_file", "output_side_history_delimited_file"},
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