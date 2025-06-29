#include "Logger.h"

// Initialize static members
Logger* Logger::instance = nullptr;
std::mutex Logger::mutex;

Logger::Logger(const std::string& filename, LogLevel level)
    : logLevel(level), enabled(true), log_to_console(false) {
    // Abrir el archivo en modo de sobreescritura (truncate) en lugar de modo append
    // para que cada ejecución cree un nuevo log
    logFile.open(filename, std::ios::out | std::ios::trunc);
    
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
                instance->logFile << timestamp << " [INFO] ========== HULK COMPILER LOGGING SESSION ===========" << std::endl;
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
        instance->logFile << timestamp << " [INFO] ========== HULK COMPILER LOGGING SESSION ===========" << std::endl;
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

void Logger::setConsoleLogging(bool enable) {
    log_to_console = enable;
    if (enable) {
        info("Console logging enabled");
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (!enabled || level < logLevel) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex);
    
    std::string formatted_message = getTimestamp() + " [" + levelToString(level) + "] " + message;
    
    // Escribir al archivo de log si está abierto
    if (logFile.is_open()) {
        logFile << formatted_message << std::endl;
        logFile.flush(); // Ensure message is written immediately
    }
    
    // Si log_to_console está activado, imprimir también en consola
    if (log_to_console) {
        // Usar colores diferentes según el nivel de log
        switch (level) {
            case LogLevel::ERROR:
                std::cerr << "\033[1;31m" << formatted_message << "\033[0m" << std::endl; // Rojo brillante
                break;
            case LogLevel::WARNING:
                std::cout << "\033[1;33m" << formatted_message << "\033[0m" << std::endl; // Amarillo brillante
                break;
            case LogLevel::INFO:
                std::cout << "\033[1;32m" << formatted_message << "\033[0m" << std::endl; // Verde brillante
                break;
            case LogLevel::DEBUG:
                std::cout << "\033[1;36m" << formatted_message << "\033[0m" << std::endl; // Cian brillante
                break;
        }
    }
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

std::string Logger::generateUniqueLogFilename(const std::string& baseFilename) {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << baseFilename << "_";
    
    // Formato: YYYYMMDD_HHMMSS
    std::tm tm_now;
    
    #ifdef _WIN32
    localtime_s(&tm_now, &time_t_now);
    #else
    localtime_r(&time_t_now, &tm_now);
    #endif
    
    ss << std::put_time(&tm_now, "%Y%m%d_%H%M%S") << ".log";
    
    return ss.str();
}

void Logger::initializeWithUniqueFile(const std::string& baseFilename, LogLevel level, const std::string& component) {
    std::lock_guard<std::mutex> lock(mutex);
    
    // Generar un nombre de archivo único
    std::string uniqueFilename = generateUniqueLogFilename(baseFilename);
    
    // Si ya existe una instancia, cerrarla primero
    if (instance != nullptr) {
        delete instance;
        instance = nullptr;
    }
    
    // Crear una nueva instancia con el nombre de archivo único
    instance = new Logger(uniqueFilename, level);
    
    if (instance->logFile.is_open()) {
        std::string timestamp = instance->getTimestamp();
        
        instance->logFile << timestamp << " [INFO] ========== HULK COMPILER LOGGING SESSION ===========" << std::endl;
        instance->logFile << timestamp << " [INFO] Log file: " << uniqueFilename << std::endl;
        instance->logFile << timestamp << " [INFO] Log level: " << instance->levelToString(level) << std::endl;
        instance->logFile << timestamp << " [INFO] Component: " << component << std::endl;
        instance->logFile << timestamp << " [INFO] ==================================================" << std::endl;
    }
}

void Logger::Initialize(const LoggerConfig& config) {
    // Si se solicita un nombre de archivo único, usar initializeWithUniqueFile
    if (config.use_unique_filename) {
        initializeWithUniqueFile(config.logFilename, config.log_level, config.component);
    } else {
        // Si no, usar el initialize normal
        initialize(config.logFilename, config.log_level, config.component);
    }
    
    // Configurar logging a consola si está habilitado
    if (instance != nullptr) {
        instance->setConsoleLogging(config.log_to_console);
    }
}
