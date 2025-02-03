#include "evaluation_result.hpp"
#include <common_util.hpp>
#include <filesystem>
int main(int argc, char *argv[]) {
    std::unordered_map<std::string, std::string> arg_map = common_util::get_command_line_argument(argc, argv);
    const std::string filename = arg_map["output_account_log_file"];
    std::vector<EvaluationResult> logs = read_result_file(filename);
    // Create a temporary file for plotting
    std::string temp_filename = "temp_data.dat";
    write_results_to_temp_file(logs, temp_filename);
    // gnu plot needs written data in data block
    plot_data(temp_filename);
    // Let's just delete the temp file
    std::filesystem::remove(temp_filename);
    return 0;
}