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

// Estructura de configuración para el logger
struct LoggerConfig {
    std::string logFilename = "hulk_compiler.log";
    LogLevel log_level = LogLevel::INFO;
    std::string component = "General";
    bool use_unique_filename = false;
    bool log_to_console = false;
    
    // Método para configurar nombre único de archivo
    LoggerConfig& withUniqueFilename(const std::string& baseFilename = "hulk_compiler") {
        this->logFilename = baseFilename;
        this->use_unique_filename = true;
        return *this;
    }
};

class Logger {
private:
    static Logger* instance;
    static std::mutex mutex;
    
    std::ofstream logFile;
    LogLevel logLevel;
    bool enabled;
    bool log_to_console;
    
    // Private constructor for singleton
    Logger(const std::string& filename = "hulk_compiler.log", LogLevel level = LogLevel::INFO);
    
    // Get current timestamp as string
    std::string getTimestamp() const;
    
    // Convert LogLevel to string
    std::string levelToString(LogLevel level) const;
    
    // Generate a unique log filename based on current date and time
    static std::string generateUniqueLogFilename(const std::string& baseFilename = "hulk_compiler");
    
public:
    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    // Get singleton instance
    static Logger* getInstance();
    
    // Initialize with log file, log level and component name
    static void initialize(const std::string& filename = "hulk_compiler.log", 
                          LogLevel level = LogLevel::INFO,
                          const std::string& component = "General");
    
    // Initialize with a unique log file name (containing timestamp)
    static void initializeWithUniqueFile(const std::string& baseFilename = "hulk_compiler", 
                                       LogLevel level = LogLevel::INFO,
                                       const std::string& component = "General");
    
    // Initialize with a LoggerConfig structure
    static void Initialize(const LoggerConfig& config);
    
    // Close the log file
    void close();
    
    // Destructor
    ~Logger();
    
    // Set log level
    void setLogLevel(LogLevel level);
    
    // Enable or disable logging
    void setEnabled(bool enable);
    
    // Enable or disable console logging
    void setConsoleLogging(bool enable);
    
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
