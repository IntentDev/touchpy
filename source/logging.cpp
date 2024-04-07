#include "logging.h"

void init_logging()
{
    spdlog::set_level(spdlog::level::trace);

    try
    {
        auto logger = spdlog::basic_logger_mt("file_logger", "logs/touchpy-log.txt");
        logger->set_pattern("[%H:%M:%S] [%t] [%l]: %v");
        logger->flush_on(spdlog::level::info);
        spdlog::set_default_logger(logger);
        spdlog::register_logger(logger);
        

        //spdlog::flush_every(std::chrono::seconds(2))
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cout << "Log init failed: " << ex.what() << std::endl;
    }
}