#include <Logger.hpp>

// Static member initialization
LogLevel Logger::currentLevel = LogLevel::LOG_INFO;
std::wostream* Logger::outputStream = &std::wcout;
std::wostream* Logger::errorStream = &std::wcerr;

void Logger::Initialize(LogLevel level, std::wostream& output, std::wostream& error) {
    currentLevel = level;
    outputStream = &output;
    errorStream = &error;
}

void Logger::SetLevel(LogLevel level) {
    currentLevel = level;
}

LogLevel Logger::GetLevel() {
    return currentLevel;
}

void Logger::Debug(const std::wstring& message) {
    if (currentLevel >= LogLevel::LOG_DEBUG) {
        *outputStream << L"[DEBUG] " << message << std::endl;
    }
}

void Logger::Info(const std::wstring& message) {
    if (currentLevel >= LogLevel::LOG_INFO) {
        *outputStream << L"[INFO] " << message << std::endl;
    }
}

void Logger::Warn(const std::wstring& message) {
    if (currentLevel >= LogLevel::LOG_WARN) {
        *outputStream << L"[WARN] " << message << std::endl;
    }
}

void Logger::Error(const std::wstring& message) {
    if (currentLevel >= LogLevel::LOG_ERROR) {
        *errorStream << L"[ERROR] " << message << std::endl;
    }
}

void Logger::Debug(const std::wstring& function, const std::wstring& message) {
    if (currentLevel >= LogLevel::LOG_DEBUG) {
        *outputStream << L"[DEBUG][" << function << L"] " << message << std::endl;
    }
}

void Logger::Debug(const std::wstring& function, const std::wstring& message, const std::wstring& value) {
    if (currentLevel >= LogLevel::LOG_DEBUG) {
        *outputStream << L"[DEBUG][" << function << L"] " << message << L": " << value << std::endl;
    }
}

bool Logger::IsDebugEnabled() {
    return currentLevel >= LogLevel::LOG_DEBUG;
}

bool Logger::IsInfoEnabled() {
    return currentLevel >= LogLevel::LOG_INFO;
}

bool Logger::IsWarnEnabled() {
    return currentLevel >= LogLevel::LOG_WARN;
}

bool Logger::IsErrorEnabled() {
    return currentLevel >= LogLevel::LOG_ERROR;
}
