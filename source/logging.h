#pragma once


#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO // All DEBUG/TRACE statements will be removed by the pre-processor

#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include <string>
#include <iostream>

using Logger = std::shared_ptr<spdlog::logger>;

// implemented in touchpy.cpp for Python and in logging_gtest.cpp for GTest
void printInfo(const std::string& info);

void initLogging(spdlog::level::level_enum level = spdlog::level::debug, bool logToConsole = true, bool logToFile = false);
void setLogLevel(spdlog::level::level_enum level = spdlog::level::info);

#define SPDLOG_FLUSH spdlog::default_logger()->flush();

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG
#define SPDLOG_FLUSH_DEBUG spdlog::default_logger()->flush();
#else
#define SPDLOG_FLUSH_DEBUG
#endif

