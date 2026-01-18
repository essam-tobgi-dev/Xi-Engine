#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <mutex>
#include <chrono>
#include <iomanip>

namespace Xi {

    enum class LogLevel {
        Trace,
        Info,
        Warning,
        Error
    };

    struct LogEntry {
        LogLevel level;
        std::string message;
        std::string timestamp;
    };

    class Log {
    public:
        static void Init();
        static void Shutdown();

        static void Trace(const std::string& message);
        static void Info(const std::string& message);
        static void Warning(const std::string& message);
        static void Error(const std::string& message);

        static const std::vector<LogEntry>& GetEntries() { return s_Entries; }
        static void Clear() { std::lock_guard<std::mutex> lock(s_Mutex); s_Entries.clear(); }

    private:
        static void LogMessage(LogLevel level, const std::string& message);
        static std::string GetTimestamp();
        static const char* LevelToString(LogLevel level);

        static std::vector<LogEntry> s_Entries;
        static std::mutex s_Mutex;
        static bool s_Initialized;
    };

#define XI_LOG_TRACE(msg) ::Xi::Log::Trace(msg)
#define XI_LOG_INFO(msg) ::Xi::Log::Info(msg)
#define XI_LOG_WARN(msg) ::Xi::Log::Warning(msg)
#define XI_LOG_ERROR(msg) ::Xi::Log::Error(msg)

}
