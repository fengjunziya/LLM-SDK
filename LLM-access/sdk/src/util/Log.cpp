#include "../../include/util/Log.h"

namespace chat_sdk {
    std::shared_ptr<spdlog::logger> Log::_instance = nullptr;
    std::mutex Log::_mutex;

    Log::Log() {}

    Log::~Log() {}

    // Init Logger
    void Log::init(const std::string& logger_name, const std::string& log_file, spdlog::level::level_enum log_level) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (!_instance) {
            // Set log default flush level, if the log level is higher than the flush level, the log will be flushed immediately.
            spdlog::flush_on(log_level);
            // Initialize thread pool with queue size and number of threads
            spdlog::init_thread_pool(32768, 1); 
            if(log_file == "stdout"){
                _instance = spdlog::stdout_color_mt<spdlog::async_factory>(logger_name);
            } else {
                _instance = spdlog::basic_logger_mt<spdlog::async_factory>(logger_name, log_file);
            }
            // Set log pattern:
            // %Y-%m-%d %H:%M:%S.%e: log time with milliseconds
            // %n: logger name
            // %-7l: log level with fixed width of 7 characters
            // %v: log message
            _instance->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%-7l] %v");
            _instance->set_level(log_level);
        }
    }

    std::shared_ptr<spdlog::logger> Log::getInstance() {
        if (!_instance) {
            init();
        }
        return _instance;
    }
}