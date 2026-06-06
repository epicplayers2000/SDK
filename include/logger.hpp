#pragma once

#include <string>
#include <vector>
#include <cstdint>

class Logger {
public:
    enum Level {
        LOG_DEBUG = 0,
        LOG_INFO = 1,
        LOG_WARNING = 2,
        LOG_ERROR = 3,
        LOG_CRITICAL = 4
    };

    static void Initialize(const std::string& log_file);
    static void Shutdown();

    static void Debug(const std::string& message);
    static void Info(const std::string& message);
    static void Warning(const std::string& message);
    static void Error(const std::string& message);
    static void Critical(const std::string& message);

    static void Log(Level level, const std::string& message);
    static void LogHex(const std::string& label, const void* data, size_t size);

private:
    static std::string log_file_;
    static Level current_level_;

    static std::string GetLevelString(Level level);
    static std::string GetTimestamp();
};

// Convenience macros
#define LOG_DEBUG(msg) Logger::Debug(msg)
#define LOG_INFO(msg) Logger::Info(msg)
#define LOG_WARNING(msg) Logger::Warning(msg)
#define LOG_ERROR(msg) Logger::Error(msg)
#define LOG_CRITICAL(msg) Logger::Critical(msg)
