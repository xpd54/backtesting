#pragma once
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#define START_DELIMITER "-"
#define EQUAL_DELIMITER "="

inline std::unordered_map<std::string, std::string> get_command_line_argument(int argc, char *argv[]) {
    std::vector<std::string> cmd_arguments;
    for (int i = 0; i < argc; ++i) {
        cmd_arguments.push_back(argv[i]);
    }
    // Erase first argument that's refer to self file name
    cmd_arguments.erase(cmd_arguments.begin());

    std::unordered_map<std::string, std::string> argument_table;
    for (auto &val : cmd_arguments) {
        // TODO :- Check for valid second and first argument if not throw error
        std::stringstream argument(val);
        std::string first;
        std::string second;
        argument.seekg(2);
        std::getline(argument, first, '=');
        std::getline(argument, second);
        argument_table[first] = second;
    }
    // Log all command line input
    std::for_each(argument_table.begin(), argument_table.end(),
                  [](auto &value) { std::cout << value.first << "=" << value.second << '\n'; });
    return argument_table;
}