#include "Logger.h"

// Initialize static members
Logger* Logger::instance = nullptr;
std::mutex Logger::mutex;

Logger::Logger(const std::string& filename, LogLevel level)
    : logLevel(level), enabled(true) {
    // No hacemos nada con el encabezado aquí, ya que la inicialización real
    // se maneja en el método initialize
    logFile.open(filename, std::ios::out | std::ios::app);
    
    if (!logFile.is_open()) {
        std::cerr << "Error: Could not open log file: " << filename << std::endl;
        std::cerr << "Please ensure the directory exists and has write permissions." << std::endl;
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile << getTimestamp() << " [INFO] ========== LOGGING SESSION ENDED ==========\n" << std::endl;
        logFile.close();
    }
}

Logger* Logger::getInstance() {
    if (instance == nullptr) {
        std::lock_guard<std::mutex> lock(mutex);
        if (instance == nullptr) {
            // Si se llama a getInstance sin inicializar primero, creamos una instancia con valores por defecto
            // y registramos que fue creada automáticamente
            instance = new Logger();
            if (instance->logFile.is_open()) {
                // Obtener timestamp actual para la sesión
                std::string timestamp = instance->getTimestamp();
                
                // Encabezado de sesión para instancia creada automáticamente
                instance->logFile << "\n\n" << timestamp << " [INFO] ========== HULK COMPILER LOGGING SESSION ===========" << std::endl;
                instance->logFile << timestamp << " [INFO] Log file: hulk_compiler.log" << std::endl;
                instance->logFile << timestamp << " [INFO] Log level: INFO" << std::endl;
                instance->logFile << timestamp << " [INFO] Initialized automatically by getInstance()" << std::endl;
                instance->logFile << timestamp << " [INFO] ==================================================" << std::endl;
            }
        }
    }
    return instance;
}

void Logger::initialize(const std::string& filename, LogLevel level, const std::string& component) {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Verificar si ya existe una instancia
    if (instance != nullptr) {
        // Si ya existe una instancia y es para el mismo archivo, solo registramos el componente
        if (instance->logFile.is_open()) {
            instance->info("Logger initialization requested from component: " + component);
        }
        return;
    }
    
    // Crear una nueva instancia si no existe
    instance = new Logger(filename, level);
    
    if (instance->logFile.is_open()) {
        // Obtener timestamp actual para la sesión
        std::string timestamp = instance->getTimestamp();
        
        // Encabezado de sesión mejorado y único
        instance->logFile << "\n\n" << timestamp << " [INFO] ========== HULK COMPILER LOGGING SESSION ===========" << std::endl;
        instance->logFile << timestamp << " [INFO] Log file: " << filename << std::endl;
        instance->logFile << timestamp << " [INFO] Log level: " << instance->levelToString(level) << std::endl;
        instance->logFile << timestamp << " [INFO] Initialized by component: " << component << std::endl;
        instance->logFile << timestamp << " [INFO] ==================================================" << std::endl;
    }
}

void Logger::close() {
    if (logFile.is_open()) {
        logFile << getTimestamp() << " [INFO] ========== LOGGING SESSION ENDED ==========\n" << std::endl;
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
