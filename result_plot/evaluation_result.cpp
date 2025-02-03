#include "evaluation_result.hpp"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

std::vector<EvaluationResult> read_result_file(const std::string &file_name) {
    std::vector<EvaluationResult> results;
    std::ifstream accont_output_file(file_name);
    std::string line;
    while (std::getline(accont_output_file, line)) {
        std::stringstream ss(line);
        EvaluationResult evaluation_result;
        ss >> evaluation_result.timestamp_sec;
        ss.ignore(1, ',');
        ss >> evaluation_result.open;
        ss.ignore(1, ',');
        ss >> evaluation_result.high;
        ss.ignore(1, ',');
        ss >> evaluation_result.low;
        ss.ignore(1, ',');
        ss >> evaluation_result.close;
        ss.ignore(1, ',');
        ss >> evaluation_result.volume;
        ss.ignore(1, ',');
        ss >> evaluation_result.base_balance;
        ss.ignore(1, ',');
        ss >> evaluation_result.quote_balance;
        ss.ignore(1, ',');
        ss >> evaluation_result.total_fee;

        evaluation_result.portfolio_value =
            (evaluation_result.base_balance * evaluation_result.close) + evaluation_result.quote_balance;
        evaluation_result.beta =
            (evaluation_result.base_balance * evaluation_result.close) / evaluation_result.portfolio_value;
        results.push_back(evaluation_result);
    }
    return results;
}

/* using gnuplot which which requires data dump */
void write_results_to_temp_file(const std::vector<EvaluationResult> &results, const std::string &temp_filename) {
    std::ofstream temp_file(temp_filename);
    for (const auto &log : results) {
        temp_file << log.timestamp_sec << "," << log.beta << "\n";
    }
}

void plot_data(const std::string &temp_filename) {
    std::string which_gnuplot = "which gnuplot";
    int which_gnuplot_status = std::system(which_gnuplot.c_str());
    if (which_gnuplot_status != 0) {
        std::cerr << "Error missing Gnu Plot :- 'brew install gnuplot' to install " << std::endl;
        std::exit(EXIT_FAILURE);
    } else {
        std::cout << "Gnu Plot Found:- " << which_gnuplot << '\n';
    }
    std::string gnuplot_command = "gnuplot -p -e \""
                                  "set datafile separator ','; "
                                  "set yrange [0.5:1.1]; " // Set the y-axis range from 0.5 to 1.1
                                  "set xlabel 'Time'; "
                                  "set ylabel 'Beta'; "
                                  "plot '" +
                                  temp_filename + "' with lines title 'Beta Values'\"";

    int gnu_output = std::system(gnuplot_command.c_str());
    if (gnu_output != 0) {
        std::cerr << "Error executing Gnu Plot!" << std::endl;
    }
}