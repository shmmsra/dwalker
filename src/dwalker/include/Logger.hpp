#pragma once

#include <iostream>
#include <sstream>
#include <string>

enum class LogLevel {
    LOG_ERROR = 0,
    LOG_WARN = 1,
    LOG_INFO = 2,
    LOG_DEBUG = 3
};

class Logger {
private:
    static LogLevel currentLevel;
    static std::wostream* outputStream;
    static std::wostream* errorStream;

public:
    static void Initialize(LogLevel level = LogLevel::LOG_INFO, 
                          std::wostream& output = std::wcout,
                          std::wostream& error = std::wcerr);
    
    static void SetLevel(LogLevel level);
    static LogLevel GetLevel();
    
    static void Debug(const std::wstring& message);
    static void Info(const std::wstring& message);
    static void Warn(const std::wstring& message);
    static void Error(const std::wstring& message);
    
    // Convenience methods for common debug scenarios
    static void Debug(const std::wstring& function, const std::wstring& message);
    static void Debug(const std::wstring& function, const std::wstring& message, const std::wstring& value);
    
    // Check if debug logging is enabled
    static bool IsDebugEnabled();
    static bool IsInfoEnabled();
    static bool IsWarnEnabled();
    static bool IsErrorEnabled();
};

// Macro for easier debug logging (only compiles when debug is enabled)
#ifdef _DEBUG
#define LOG_DEBUG(msg) Logger::Debug(msg)
#define LOG_DEBUG_FUNC(func, msg) Logger::Debug(func, msg)
#define LOG_DEBUG_FUNC_VAL(func, msg, val) Logger::Debug(func, msg, val)
#else
#define LOG_DEBUG(msg) do {} while(0)
#define LOG_DEBUG_FUNC(func, msg) do {} while(0)
#define LOG_DEBUG_FUNC_VAL(func, msg, val) do {} while(0)
#endif

#define LOG_INFO(msg) Logger::Info(msg)
#define LOG_WARN(msg) Logger::Warn(msg)
#define LOG_ERROR(msg) Logger::Error(msg)
