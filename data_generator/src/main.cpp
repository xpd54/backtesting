#include "util/cmd_line_args.hpp"
#include <common_util.hpp>
#include <cstdlib>
#include <string>
#include <unordered_map>
int main(int argc, char *argv[]) {
    /*Initialize logger*/
    common_util::Logger &logger = common_util::Logger::get_instance();
    logger.init();
    logger.open();
    /*validate arguments*/
    std::unordered_map<std::string, std::string> arg_map = common_util::get_command_line_argument(argc, argv);
    for (auto &val : arg_map) {
        if (!arg_valid(val.first)) {
            logger.log(val.first + " arg is not valid", common_util::Logger::Severity::ERROR);
            std::exit(EXIT_FAILURE);
        }
    }

    logger.close();
    return 0;
}