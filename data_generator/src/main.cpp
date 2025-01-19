#include <common_util.hpp>
int main() {
    common_util::Logger &logger = common_util::Logger::get_instance();
    logger.init();
    logger.open();
    logger.log(12, common_util::Logger::Severity::DEBUG);
    return 0;
}