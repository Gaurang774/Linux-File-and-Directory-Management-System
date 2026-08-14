#pragma once

///////////////////////////////////////////////////////////////////////////////
// Logger.hpp — Thread-safe logging utility
//
// Levels:  ERROR < WARN < INFO < DEBUG
// Output:  stderr (never pollutes stdout)
//
// Usage:
//   Logger::instance().setLevel(LogLevel::DEBUG);
//   LOG_INFO("Copied file: " + path);
///////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <mutex>
#include <string>

namespace filemgr {

enum class LogLevel { QUIET, ERROR, WARN, INFO, DEBUG };

class Logger {
public:
    // ── Singleton access ────────────────────────────────────────────────
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    // ── Configuration ───────────────────────────────────────────────────
    void setLevel(LogLevel level) { level_ = level; }
    [[nodiscard]] LogLevel getLevel() const noexcept { return level_; }

    // ── Logging methods ─────────────────────────────────────────────────
    void error(const std::string& msg) const { log(LogLevel::ERROR, msg); }
    void warn (const std::string& msg) const { log(LogLevel::WARN,  msg); }
    void info (const std::string& msg) const { log(LogLevel::INFO,  msg); }
    void debug(const std::string& msg) const { log(LogLevel::DEBUG, msg); }

    void log(LogLevel level, const std::string& msg) const {
        if (level > level_) return;

        std::lock_guard<std::mutex> lock(mutex_);
        std::cerr << "[" << levelToString(level) << "] " << msg << "\n";
    }

private:
    Logger() = default;
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    LogLevel level_ = LogLevel::ERROR;  // default: errors only
    mutable std::mutex mutex_;

    static const char* levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::QUIET: return "QUIET";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::WARN:  return "WARN";
            case LogLevel::INFO:  return "INFO";
            case LogLevel::DEBUG: return "DEBUG";
        }
        return "UNKNOWN";
    }
};

// ── Convenience macros ──────────────────────────────────────────────────────
#define LOG_ERROR(msg) ::filemgr::Logger::instance().error(msg)
#define LOG_WARN(msg)  ::filemgr::Logger::instance().warn(msg)
#define LOG_INFO(msg)  ::filemgr::Logger::instance().info(msg)
#define LOG_DEBUG(msg) ::filemgr::Logger::instance().debug(msg)

} // namespace filemgr
