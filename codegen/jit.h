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
    
    llvm::Error addModule(std::unique_ptr<llvm::Module> M);
    
    llvm::Expected<llvm::JITEvaluatedSymbol> lookup(llvm::StringRef name);
    
    double executeFunction(const std::string& name);
    
    void loadStandardLibrary();

private:
    std::unique_ptr<llvm::ExecutionEngine> engine;
};

#endif