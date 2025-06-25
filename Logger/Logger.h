#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <string>
#include <mutex>
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    static Logger* instance;
    static std::mutex mutex;
    
    std::ofstream logFile;
    LogLevel logLevel;
    bool enabled;
    
    // Private constructor for singleton
    Logger(const std::string& filename = "hulk_compiler.log", LogLevel level = LogLevel::INFO);
    
    // Get current timestamp as string
    std::string getTimestamp() const;
    
    // Convert LogLevel to string
    std::string levelToString(LogLevel level) const;
    
public:
    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    // Get singleton instance
    static Logger* getInstance();
    
    // Initialize with log file and log level
    static void initialize(const std::string& filename = "hulk_compiler.log", LogLevel level = LogLevel::INFO);
    
    // Close the log file
    void close();
    
    // Destructor
    ~Logger();
    
    // Set log level
    void setLogLevel(LogLevel level);
    
    // Enable or disable logging
    void setEnabled(bool enable);
    
    // Log message with specified level
    void log(LogLevel level, const std::string& message);
    
    // Convenience methods for different log levels
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
};

// Global helper functions for easy access
inline void LogDebug(const std::string& message) {
    Logger::getInstance()->debug(message);
}

inline void LogInfo(const std::string& message) {
    Logger::getInstance()->info(message);
}

inline void LogWarning(const std::string& message) {
    Logger::getInstance()->warning(message);
}

inline void LogError(const std::string& message) {
    Logger::getInstance()->error(message);
}

#endif // LOGGER_H
