#ifndef CODEGEN_JIT_H
#define CODEGEN_JIT_H

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/Error.h>
#include <memory>

// A simple JIT wrapper around LLVM's ExecutionEngine
class HulkJIT {
public:
    HulkJIT() {}
    
    // Add a module to the JIT
    llvm::Error addModule(std::unique_ptr<llvm::Module> M);
    
    // Look up a symbol in the JIT
    llvm::Expected<llvm::JITEvaluatedSymbol> lookup(llvm::StringRef name);
    
    // Execute a function from the JIT and return its result
    double executeFunction(const std::string& name);
    
    // Load standard library functions
    void loadStandardLibrary();

private:
    std::unique_ptr<llvm::ExecutionEngine> engine;
};

#endif