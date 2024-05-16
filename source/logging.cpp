#include "logging.h"

void initLogging(spdlog::level::level_enum level, bool logToConsole, bool logToFile)
{
    try
    {
        //auto logger = spdlog::basic_logger_mt("file_logger_", "logs/touchpy-log.txt");
        std::vector<spdlog::sink_ptr> sinks; 
        if (logToConsole)
        {
            sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
            sinks.back()->set_pattern("[%H:%M:%S.%e] [%t] [%^%l%$]: %v");
        }
        if (logToFile)
        {
            sinks.push_back(std::make_shared < spdlog::sinks::basic_file_sink_mt >("logs/touchpy-log.txt"));
            sinks.back()->set_pattern("[%D %H:%M:%S.%e] [%t] [%^%l%$]: %v");
        }

        auto logger = std::make_shared<spdlog::logger>("logger", begin(sinks), end(sinks));

        logger->flush_on(level);
        spdlog::set_default_logger(logger);
        //spdlog::register_logger(logger);
        

        //spdlog::flush_every(std::chrono::seconds(2))
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cout << "Log init failed: " << ex.what() << std::endl;
    }

    spdlog::set_level(level);
}

void setLogLevel(spdlog::level::level_enum level)
{
	spdlog::set_level(level);
	spdlog::default_logger()->flush();
}

