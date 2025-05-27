#include "visitor.h"
#include "../Ast/ast.hpp"
#include <iostream>
#include <map>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Constants.h>
#include <llvm/Support/raw_ostream.h>

// Global or member variables for LLVM context
// static std::unique_ptr<llvm::LLVMContext> TheContext;
static llvm::LLVMContext TheContext;
static std::unique_ptr<llvm::Module> TheModule;
static std::unique_ptr<llvm::IRBuilder<>> Builder;
// static std::map<std::string, Value*> NamedValues;

void CodegenVisitor::visit(ASTNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    node->accept(this, context);
}

void CodegenVisitor::visit(FloatNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    // Generate an LLVM constant float
    llvm::Value* val = llvm::ConstantFP::get(llvm::Type::getDoubleTy(TheContext), node->value);
    // Optionally, store or use 'val' as needed
    llvm::outs() << "Generated LLVM IR for FloatNode: ";
    val->print(llvm::outs());
    llvm::outs() << "\n";
}

void CodegenVisitor::visit(BoolNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    cout << "Generating code for Bool Node: " << node->value << endl;
}

void CodegenVisitor::visit(StringNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    cout << "Generating code for String Node: " << node->value << endl;
}

void CodegenVisitor::visit(UnaryOpNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for UnaryOp Node: " << node->op << endl;
    
    this->visit(node->node, context);
}

void CodegenVisitor::visit(BinOpNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for BinOp Node: " << node->op << endl;
    
    this->visit(node->left, context);
    this->visit(node->right, context);
}