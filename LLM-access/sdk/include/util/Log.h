#pragma once
#include<spdlog/spdlog.h>
#include<spdlog/logger.h>
#include<spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/async.h>
#include<mutex>

namespace chat_sdk {

    class Log {
    private:
        static std::shared_ptr<spdlog::logger> _instance;
        static std::mutex _mutex;

    private:
        Log();
        ~Log();
        Log& operator=(const Log&) = delete;

    public:
        static void init(const std::string& logger_name = "chat_sdk_logger", const std::string& log_file = "chat_sdk.log", spdlog::level::level_enum log_level = spdlog::level::info);
        static std::shared_ptr<spdlog::logger> getInstance();
    };

    // Log macros for different log levels, with file name and line number
    #define TRC(format,...) chat_sdk::Log::getInstance()->trace(std::string("[{:>10s} {:<4d}]")+format,__FILE__, __LINE__, ##__VA_ARGS__)
    #define DBG(format,...) chat_sdk::Log::getInstance()->debug(std::string("[{:>10s} {:<4d}]")+format,__FILE__, __LINE__, ##__VA_ARGS__)
    #define INF(format,...) chat_sdk::Log::getInstance()->info(std::string("[{:>10s} {:<4d}]")+format,__FILE__, __LINE__, ##__VA_ARGS__)
    #define WRN(format,...) chat_sdk::Log::getInstance()->warn(std::string("[{:>10s} {:<4d}]")+format,__FILE__, __LINE__, ##__VA_ARGS__)
    #define ERR(format,...) chat_sdk::Log::getInstance()->error(std::string("[{:>10s} {:<4d}]")+format,__FILE__, __LINE__, ##__VA_ARGS__)
    #define CRI(format,...) chat_sdk::Log::getInstance()->critical(std::string("[{:>10s} {:<4d}]")+format,__FILE__, __LINE__, ##__VA_ARGS__)

}