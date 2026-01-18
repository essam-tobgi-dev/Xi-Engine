#include "Log.h"

namespace Xi {

    std::vector<LogEntry> Log::s_Entries;
    std::mutex Log::s_Mutex;
    bool Log::s_Initialized = false;

    void Log::Init() {
        s_Initialized = true;
        Info("Log system initialized");
    }

    void Log::Shutdown() {
        Info("Log system shutdown");
        s_Initialized = false;
    }

    void Log::Trace(const std::string& message) {
        LogMessage(LogLevel::Trace, message);
    }

    void Log::Info(const std::string& message) {
        LogMessage(LogLevel::Info, message);
    }

    void Log::Warning(const std::string& message) {
        LogMessage(LogLevel::Warning, message);
    }

    void Log::Error(const std::string& message) {
        LogMessage(LogLevel::Error, message);
    }

    void Log::LogMessage(LogLevel level, const std::string& message) {
        std::lock_guard<std::mutex> lock(s_Mutex);

        LogEntry entry;
        entry.level = level;
        entry.message = message;
        entry.timestamp = GetTimestamp();

        s_Entries.push_back(entry);

        // Also print to console
        const char* levelStr = LevelToString(level);
        std::cout << "[" << entry.timestamp << "] [" << levelStr << "] " << message << std::endl;
    }

    std::string Log::GetTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream ss;
        std::tm tm_buf;
#ifdef _WIN32
        localtime_s(&tm_buf, &time);
#else
        localtime_r(&time, &tm_buf);
#endif
        ss << std::put_time(&tm_buf, "%H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }

    const char* Log::LevelToString(LogLevel level) {
        switch (level) {
            case LogLevel::Trace:   return "TRACE";
            case LogLevel::Info:    return "INFO";
            case LogLevel::Warning: return "WARN";
            case LogLevel::Error:   return "ERROR";
            default:                return "UNKNOWN";
        }
    }

}
