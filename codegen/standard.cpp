#include <iostream>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <limits>

extern "C" void print(const char* str) {
    std::cout << str << std::endl;
}

extern "C" void print_double(double value) {
    std::cout << value << std::endl;
}

extern "C" void print_bool(bool value) {
    std::cout << (value ? "true" : "false") << std::endl;
}

extern "C" double get_pi() {
    return M_PI;
}

extern "C" double get_e() {
    return M_E;
}

extern "C" double sqrt_func(double x) {
    return std::sqrt(x);
}

extern "C" double sin_func(double x) {
    return std::sin(x);
}

extern "C" double cos_func(double x) {
    return std::cos(x);
}

extern "C" double tan_func(double x) {
    return std::tan(x);
}

extern "C" double exp_func(double x) {
    return std::exp(x);
}

extern "C" double log_func(double base, double value) {
    if (base <= 0 || base == 1 || value <= 0) {
        return std::numeric_limits<double>::quiet_NaN();
    }
    return std::log(value) / std::log(base);
}

extern "C" double rand_func() {
    static bool initialized = false;
    if (!initialized) {
        std::srand(std::time(nullptr));
        initialized = true;
    }
    return static_cast<double>(std::rand()) / RAND_MAX;
}

extern "C" const char* double_to_string(double value) {
    std::string* result = new std::string(std::to_string(value));
    return result->c_str();
}