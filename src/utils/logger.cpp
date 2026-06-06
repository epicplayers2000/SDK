#include "logger.hpp"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <android/log.h>

std::string Logger::log_file_;
Logger::Level Logger::current_level_ = LOG_INFO;

void Logger::Initialize(const std::string& log_file) {
    log_file_ = log_file;
    Info("Logger initialized: " + log_file);
}

void Logger::Shutdown() {
    Info("Logger shutdown");
}

void Logger::Debug(const std::string& message) {
    Log(LOG_DEBUG, message);
}

void Logger::Info(const std::string& message) {
    Log(LOG_INFO, message);
}

void Logger::Warning(const std::string& message) {
    Log(LOG_WARNING, message);
}

void Logger::Error(const std::string& message) {
    Log(LOG_ERROR, message);
}

void Logger::Critical(const std::string& message) {
    Log(LOG_CRITICAL, message);
}

void Logger::Log(Level level, const std::string& message) {
    if (level < current_level_) {
        return;
    }

    std::string timestamp = GetTimestamp();
    std::string level_str = GetLevelString(level);
    
    std::string log_message = "[" + timestamp + "] [" + level_str + "] " + message;

    // Print to logcat
    __android_log_print(ANDROID_LOG_INFO, "UE4Dumper", "%s", log_message.c_str());

    // Also write to file if specified
    if (!log_file_.empty()) {
        std::ofstream file(log_file_, std::ios::app);
        if (file.is_open()) {
            file << log_message << std::endl;
            file.close();
        }
    }
}

void Logger::LogHex(const std::string& label, const void* data, size_t size) {
    std::stringstream ss;
    ss << label << ": ";
    
    const uint8_t* bytes = (const uint8_t*)data;
    for (size_t i = 0; i < size; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[i] << " ";
    }
    
    Info(ss.str());
}

std::string Logger::GetLevelString(Level level) {
    switch (level) {
        case LOG_DEBUG: return "DEBUG";
        case LOG_INFO: return "INFO";
        case LOG_WARNING: return "WARNING";
        case LOG_ERROR: return "ERROR";
        case LOG_CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

std::string Logger::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}
