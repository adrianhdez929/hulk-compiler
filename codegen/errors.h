// Error handling utilities for JIT compiler
#include <llvm/Support/Error.h>
#include <iostream>

// Use our own error converter
inline std::string errorToString(const llvm::Error &E) {
    std::string ErrMsg;
    llvm::raw_string_ostream OS(ErrMsg);
    OS << E;
    return OS.str();
}

inline void logAllErrors(llvm::Error E) {
    if (E) {
        std::cerr << "JIT Error: " << errorToString(E) << std::endl;
    }
}
