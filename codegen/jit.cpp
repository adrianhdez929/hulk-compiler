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

Error HulkJIT::addModule(std::unique_ptr<Module> M) {
    std::string errorMsg;
    if (sys::DynamicLibrary::LoadLibraryPermanently(nullptr, &errorMsg)) {
        return createStringError(inconvertibleErrorCode(), 
                               "Could not load process symbols: " + errorMsg);
    }
    
    // Try to load the standard library from hulk directory or current directory
    if (sys::DynamicLibrary::LoadLibraryPermanently("./hulk/libstandard.so", &errorMsg) &&
        sys::DynamicLibrary::LoadLibraryPermanently("./libstandard.so", &errorMsg)) {
        std::cerr << "Warning: Could not load libstandard.so: " << errorMsg << std::endl;
    }

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

Expected<JITEvaluatedSymbol> HulkJIT::lookup(StringRef name) {
    if (!engine) {
        return createStringError(inconvertibleErrorCode(), "No module added to JIT");
    }
    
    uint64_t addr = engine->getFunctionAddress(std::string(name));
    if (!addr) {
        return createStringError(inconvertibleErrorCode(), 
                                "Could not find symbol: " + name.str());
    }
    
    return JITEvaluatedSymbol(addr, JITSymbolFlags::None);
}

double HulkJIT::executeFunction(const std::string& name) {
    if (!engine) {
        std::cerr << "Error: No module added to JIT" << std::endl;
        return 0.0;
    }
    
    auto symbolOrErr = lookup(name);
    if (!symbolOrErr) {
        logAllErrors(symbolOrErr.takeError());
        return 0.0;
    }
    
    double (*func)() = (double (*)())(intptr_t)symbolOrErr->getAddress();
    
    if (!func) {
        std::cerr << "Error: Could not cast function pointer" << std::endl;
        return 0.0;
    }
    
    try {
        return func();
    } catch (const std::exception& e) {
        std::cerr << "Error executing function: " << e.what() << std::endl;
        return 0.0;
    }
}

void HulkJIT::loadStandardLibrary() {
    std::cout << "Loading standard library functions..." << std::endl;
}
