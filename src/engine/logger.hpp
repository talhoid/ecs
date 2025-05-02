#pragma once
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <string_view>

#define LOG_DEBUG(...) Logger::debug(__PRETTY_FUNCTION__, __VA_ARGS__)
#define LOG_INFO(...) Logger::info(__PRETTY_FUNCTION__, __VA_ARGS__)
#define LOG_WARNING(...) Logger::warning(__PRETTY_FUNCTION__, __VA_ARGS__)
#define LOG_ERROR(...) Logger::error(__PRETTY_FUNCTION__, __VA_ARGS__)

class Logger {
public:
    enum class Level {
        Debug,
        Info,
        Warning,
        Error
    };

    static void set_level(Level level) {
        min_level = level;
    }

    template<typename... Args>
    static void debug(std::string_view scope, const Args&... args) {
        log(Level::Debug, scope, args...);
    }

    template<typename... Args>
    static void info(std::string_view scope, const Args&... args) {
        log(Level::Info, scope, args...);
    }

    template<typename... Args>
    static void warning(std::string_view scope, const Args&... args) {
        log(Level::Warning, scope, args...);
    }

    template<typename... Args>
    static void error(std::string_view scope, const Args&... args) {
        log(Level::Error, scope, args...);
    }

private:
    static Level min_level;

    template<typename... Args>
    static void log(Level level, std::string_view scope, const Args&... args) {
        if (level < min_level) return;

        auto now = std::chrono::system_clock::now();
        auto now_c = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::stringstream ss;
        ss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S")
           << '.' << std::setfill('0') << std::setw(3) << ms.count()
           << " [" << level_to_string(level) << "] "
           << "[" << scope << "] ";
        
        (ss << ... << args);
        std::cout << ss.str() << std::endl;
    }

    static const char* level_to_string(Level level) {
        switch (level) {
            case Level::Debug: return "DEBUG";
            case Level::Info: return "INFO ";
            case Level::Warning: return "WARN ";
            case Level::Error: return "ERROR";
            default: return "UNKNOWN";
        }
    }
};