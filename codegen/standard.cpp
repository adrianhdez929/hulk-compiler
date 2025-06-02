#include <iostream>
#include <string>
#include <cmath>

// Standard library functions for the Hulk compiler JIT

// Print function for strings
extern "C" void print(const char* str) {
    std::cout << str << std::endl;
}

// Print function for numbers (double)
extern "C" void print_double(double value) {
    std::cout << value << std::endl;
}

// Print function for booleans
extern "C" void print_bool(bool value) {
    std::cout << (value ? "true" : "false") << std::endl;
}

// Math constants
extern "C" double get_pi() {
    return M_PI;
}

extern "C" double get_e() {
    return M_E;
}

// Basic math functions
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

extern "C" double log_func(double x) {
    return std::log(x);
}

extern "C" double exp_func(double x) {
    return std::exp(x);
}

// Conversion functions
extern "C" const char* double_to_string(double value) {
    std::string* result = new std::string(std::to_string(value));
    return result->c_str();
}