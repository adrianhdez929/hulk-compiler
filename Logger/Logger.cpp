#include "Logger.h"

// Initialize static members
Logger* Logger::instance = nullptr;
std::mutex Logger::mutex;

Logger::Logger(const std::string& filename, LogLevel level)
    : logLevel(level), enabled(true) {
    logFile.open(filename, std::ios::out | std::ios::app);
    
    if (!logFile.is_open()) {
        std::cerr << "Error: Could not open log file: " << filename << std::endl;
    } else {
        logFile << "\n\n" << getTimestamp() << " [INFO] ======== LOGGING SESSION STARTED ========" << std::endl;
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile << getTimestamp() << " [INFO] ======== LOGGING SESSION ENDED ========\n" << std::endl;
        logFile.close();
    }
}

Logger* Logger::getInstance() {
    if (instance == nullptr) {
        std::lock_guard<std::mutex> lock(mutex);
        if (instance == nullptr) {
            instance = new Logger();
        }
    }
    return instance;
}

void Logger::initialize(const std::string& filename, LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex);
    if (instance != nullptr) {
        delete instance;
    }
    instance = new Logger(filename, level);
}

void Logger::close() {
    if (logFile.is_open()) {
        logFile << getTimestamp() << " [INFO] ======== LOGGING SESSION ENDED ========\n" << std::endl;
        logFile.close();
    }
}

std::string Logger::getTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t_now), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

std::string Logger::levelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

void Logger::setLogLevel(LogLevel level) {
    this->logLevel = level;
}

void Logger::setEnabled(bool enable) {
    this->enabled = enable;
}

void Logger::log(LogLevel level, const std::string& message) {
    if (!enabled || level < logLevel || !logFile.is_open()) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex);
    logFile << getTimestamp() << " [" << levelToString(level) << "] " << message << std::endl;
    logFile.flush(); // Ensure message is written immediately
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}
