#include "logging.h"

void initLogging(spdlog::level::level_enum level)
{
    spdlog::set_level(spdlog::level::trace);

    try
    {
        auto logger = spdlog::basic_logger_mt("file_logger_", "logs/touchpy-log.txt");
        logger->set_pattern("[%D:%H:%M:%S] [%t] [%l]: %v");
        logger->flush_on(level);
        spdlog::set_default_logger(logger);
        //spdlog::register_logger(logger);
        

        //spdlog::flush_every(std::chrono::seconds(2))
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cout << "Log init failed: " << ex.what() << std::endl;
    }
}

void setLogLevel(spdlog::level::level_enum level)
{
	spdlog::set_level(level);
	spdlog::default_logger()->flush();
}

