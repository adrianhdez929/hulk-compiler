#include "jit.h"
#include "errors.h"
#include <iostream>
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/MCJIT.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/DynamicLibrary.h>

// External print function from standard.cpp
extern "C" void print(const char* str);

using namespace llvm;

// Add a module to the JIT
Error HulkJIT::addModule(std::unique_ptr<Module> M) {
    // First, load the current process symbols so our print function can be found
    std::string errorMsg;
    if (sys::DynamicLibrary::LoadLibraryPermanently(nullptr, &errorMsg)) {
        return createStringError(inconvertibleErrorCode(), 
                               "Could not load process symbols: " + errorMsg);
    }
    
    // Also load our standard library
    if (sys::DynamicLibrary::LoadLibraryPermanently("./libstandard.so", &errorMsg)) {
        std::cerr << "Warning: Could not load libstandard.so: " << errorMsg << std::endl;
        // Continue anyway - the function might be available in the process
    }

    // Create JIT engine for module
    engine.reset(EngineBuilder(std::move(M))
        .setEngineKind(EngineKind::JIT)
        .setErrorStr(&errorMsg)
        .create());
        
    if (!engine) {
        return createStringError(inconvertibleErrorCode(), 
                                "Could not create ExecutionEngine: " + errorMsg);
    }
    
    return Error::success();
}

// Look up a symbol in the JIT
Expected<JITEvaluatedSymbol> HulkJIT::lookup(StringRef name) {
    if (!engine) {
        return createStringError(inconvertibleErrorCode(), "No module added to JIT");
    }
    
    // Find the symbol in the JIT
    uint64_t addr = engine->getFunctionAddress(std::string(name));
    if (!addr) {
        return createStringError(inconvertibleErrorCode(), 
                                "Could not find symbol: " + name.str());
    }
    
    // Return the symbol
    return JITEvaluatedSymbol(addr, JITSymbolFlags::None);
}

// Execute a function from the JIT and return its result
double HulkJIT::executeFunction(const std::string& name) {
    if (!engine) {
        std::cerr << "Error: No module added to JIT" << std::endl;
        return 0.0;
    }
    
    // Look up the function
    auto symbolOrErr = lookup(name);
    if (!symbolOrErr) {
        logAllErrors(symbolOrErr.takeError());
        return 0.0;
    }
    
    // Cast the function pointer
    double (*func)() = (double (*)())(intptr_t)symbolOrErr->getAddress();
    
    if (!func) {
        std::cerr << "Error: Could not cast function pointer" << std::endl;
        return 0.0;
    }
    
    // Execute the function and return its result
    try {
        return func();
    } catch (const std::exception& e) {
        std::cerr << "Error executing function: " << e.what() << std::endl;
        return 0.0;
    }
}

// Load standard library functions
void HulkJIT::loadStandardLibrary() {
    // Standard library functions are loaded in addModule via LoadLibraryPermanently
    std::cout << "Loading standard library functions..." << std::endl;
}
