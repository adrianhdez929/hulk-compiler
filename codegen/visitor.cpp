#include "visitor.h"
#include "../Ast/ast.hpp"
#include "jit.h"
#include "errors.h"
#include <iostream>
#include <map>
#include <algorithm>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Constants.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/InstCombine/InstCombine.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <llvm/Transforms/Utils.h>

// contexto global de LLVM
static std::unique_ptr<llvm::LLVMContext> TheContext = std::make_unique<llvm::LLVMContext>();
static std::unique_ptr<llvm::Module> TheModule = std::make_unique<llvm::Module>("hulk_module", *TheContext);
static std::unique_ptr<llvm::IRBuilder<>> Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
static HulkJIT TheJIT;
static std::map<std::string, llvm::Value*> NamedValues;

// Declare the external print function to LLVM
static llvm::Function* createPrintFunction() {
    // Define the print function signature: void print(char*)
    llvm::FunctionType* printFuncType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(*TheContext),                        // Return type: void
        {llvm::Type::getInt8PtrTy(*TheContext)},                   // Parameter type: char*
        false                                                       // Not vararg
    );
    
    // Declare the external function in our module
    llvm::Function* printFunc = llvm::Function::Create(
        printFuncType,
        llvm::Function::ExternalLinkage,
        "print",
        TheModule.get()
    );
    
    return printFunc;
}

// Initialize the print function
static llvm::Function* PrintFunc = createPrintFunction();

// declaraciones de la lib standard
void CodegenVisitor::createStandardLibraryDeclarations() {
    llvm::FunctionType* printDoubleFuncType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(*TheContext),                       
        {llvm::Type::getDoubleTy(*TheContext)},                   
        false                                                     
    );
    llvm::Function::Create(
        printDoubleFuncType,
        llvm::Function::ExternalLinkage,
        "print_double",
        TheModule.get()
    );
    
    llvm::FunctionType* printBoolFuncType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(*TheContext),                        
        {llvm::Type::getInt1Ty(*TheContext)},                      
        false                                                      
    );
    llvm::Function::Create(
        printBoolFuncType,
        llvm::Function::ExternalLinkage,
        "print_bool",
        TheModule.get()
    );
    
    llvm::FunctionType* getPiFuncType = llvm::FunctionType::get(
        llvm::Type::getDoubleTy(*TheContext),                      
        false                                                      
    );
    llvm::Function::Create(
        getPiFuncType,
        llvm::Function::ExternalLinkage,
        "get_pi",
        TheModule.get()
    );
    
    llvm::FunctionType* getEFuncType = llvm::FunctionType::get(
        llvm::Type::getDoubleTy(*TheContext),                     
        false                                                     
    );
    llvm::Function::Create(
        getEFuncType,
        llvm::Function::ExternalLinkage,
        "get_e",
        TheModule.get()
    );
    
    // Mathematical functions
    std::vector<std::string> mathFuncs = {"sqrt_func", "sin_func", "cos_func", "tan_func", "exp_func"};
    for (const auto& funcName : mathFuncs) {
        llvm::FunctionType* mathFuncType = llvm::FunctionType::get(
            llvm::Type::getDoubleTy(*TheContext),                 
            {llvm::Type::getDoubleTy(*TheContext)},               
            false                                                 
        );
        llvm::Function::Create(
            mathFuncType,
            llvm::Function::ExternalLinkage,
            funcName,
            TheModule.get()
        );
    }
    
    // Two-parameter log function
    llvm::FunctionType* logFuncType = llvm::FunctionType::get(
        llvm::Type::getDoubleTy(*TheContext),
        {llvm::Type::getDoubleTy(*TheContext), llvm::Type::getDoubleTy(*TheContext)},
        false
    );
    llvm::Function::Create(
        logFuncType,
        llvm::Function::ExternalLinkage,
        "log_func",
        TheModule.get()
    );
    
    // Random function (no parameters)
    llvm::FunctionType* randFuncType = llvm::FunctionType::get(
        llvm::Type::getDoubleTy(*TheContext),
        {},
        false
    );
    llvm::Function::Create(
        randFuncType,
        llvm::Function::ExternalLinkage,
        "rand_func",
        TheModule.get()
    );
    
    // String manipulation functions
    llvm::FunctionType* doubleToStringType = llvm::FunctionType::get(
        llvm::Type::getInt8PtrTy(*TheContext),
        {llvm::Type::getDoubleTy(*TheContext)},
        false
    );
    llvm::Function::Create(
        doubleToStringType,
        llvm::Function::ExternalLinkage,
        "double_to_string",
        TheModule.get()
    );
    
    // Standard C string functions
    llvm::FunctionType* strcatType = llvm::FunctionType::get(
        llvm::Type::getInt8PtrTy(*TheContext),
        {llvm::Type::getInt8PtrTy(*TheContext), llvm::Type::getInt8PtrTy(*TheContext)},
        false
    );
    llvm::Function::Create(
        strcatType,
        llvm::Function::ExternalLinkage,
        "hulk_strcat",
        TheModule.get()
    );
    
    llvm::FunctionType* strlenType = llvm::FunctionType::get(
        llvm::Type::getInt64Ty(*TheContext),
        {llvm::Type::getInt8PtrTy(*TheContext)},
        false
    );
    llvm::Function::Create(
        strlenType,
        llvm::Function::ExternalLinkage,
        "hulk_strlen",
        TheModule.get()
    );
    
    llvm::FunctionType* strcpyType = llvm::FunctionType::get(
        llvm::Type::getInt8PtrTy(*TheContext),
        {llvm::Type::getInt8PtrTy(*TheContext), llvm::Type::getInt8PtrTy(*TheContext)},
        false
    );
    llvm::Function::Create(
        strcpyType,
        llvm::Function::ExternalLinkage,
        "hulk_strcpy",
        TheModule.get()
    );
    
    llvm::FunctionType* mallocType = llvm::FunctionType::get(
        llvm::Type::getInt8PtrTy(*TheContext),
        {llvm::Type::getInt64Ty(*TheContext)},
        false
    );
    llvm::Function::Create(
        mallocType,
        llvm::Function::ExternalLinkage,
        "hulk_malloc",
        TheModule.get()
    );
    
    // std::cout << "Standard library function declarations created." << std::endl;
}

// funcion main
llvm::Function* CodegenVisitor::createMainFunction() {
    llvm::FunctionType* mainFuncType = llvm::FunctionType::get(
        llvm::Type::getDoubleTy(*TheContext),                      
        false                                                      
    );
    
    llvm::Function* mainFunc = llvm::Function::Create(
        mainFuncType,
        llvm::Function::ExternalLinkage,
        "main",
        TheModule.get()
    );
    
    llvm::BasicBlock* BB = llvm::BasicBlock::Create(*TheContext, "entry", mainFunc);
    Builder->SetInsertPoint(BB);
    
    // std::cout << "Main function created." << std::endl;
    return mainFunc;
}

// initializar LLVM y funcion main
void CodegenVisitor::initialize() {
    // targets de LLVM
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    
    // declaraciones de la std lib
    createStandardLibraryDeclarations();
    
    // funcion main
    llvm::Function* mainFunc = createMainFunction();
    
    // std::cout << "LLVM and JIT initialized. Main function created." << std::endl;
}

void CodegenVisitor::visit(ASTNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    if (auto blockNode = dynamic_cast<BlockNode*>(node)) {
        visit(blockNode, context);
    } else if (auto floatNode = dynamic_cast<FloatNode*>(node)) {
        visit(floatNode, context);
    } else if (auto boolNode = dynamic_cast<BoolNode*>(node)) {
        visit(boolNode, context);
    } else if (auto stringNode = dynamic_cast<StringNode*>(node)) {
        visit(stringNode, context);
    } else if (auto unaryOpNode = dynamic_cast<UnaryOpNode*>(node)) {
        visit(unaryOpNode, context);
    } else if (auto binOpNode = dynamic_cast<BinOpNode*>(node)) {
        visit(binOpNode, context);
    } else if (auto functionCallNode = dynamic_cast<FunctionCallNode*>(node)) {
        visit(functionCallNode, context);
    } else if (auto idNode = dynamic_cast<IDNode*>(node)) {
        visit(idNode, context);
    } else if (auto argsListNode = dynamic_cast<ArgsList*>(node)) {
        visit(argsListNode, context);
    } else if (auto assignFuncNode = dynamic_cast<AssignFuncNode*>(node)) {
        visit(assignFuncNode, context);
    } else if (auto letAssignNode = dynamic_cast<LetAssign*>(node)) {
        visit(letAssignNode, context);
    } else if (auto varAssignNode = dynamic_cast<VarAssign*>(node)) {
        visit(varAssignNode, context);
    } else if (auto newTypeNode = dynamic_cast<NewTypeNode*>(node)) {
        visit(newTypeNode, context);
    } else if (auto varAssignTypeNode = dynamic_cast<VarAssignType*>(node)) {
        visit(varAssignTypeNode, context);
    } else if (auto varAssignListNode = dynamic_cast<VarAssignList*>(node)) {
        visit(varAssignListNode, context);
    } else if (auto conditionalNode = dynamic_cast<Conditional*>(node)) {
        visit(conditionalNode, context);
    } else if (auto boolExprNode = dynamic_cast<BoolExprNode*>(node)) {
        visit(boolExprNode, context);
    } else if (auto whileNode = dynamic_cast<WhileNode*>(node)) {
        visit(whileNode, context);
    } else if (auto varDesAssignNode = dynamic_cast<VarDesAssign*>(node)) {
        visit(varDesAssignNode, context);
    } else if (auto forNode = dynamic_cast<ForNode*>(node)) {
        visit(forNode, context);
    } else if (auto typeDeclNode = dynamic_cast<TypeDeclNode*>(node)) {
        visit(typeDeclNode, context);
    } else if (auto astNodeVector = dynamic_cast<ASTNodeVector*>(node)) {
        visit(astNodeVector, context);
    } else if (auto exprsList = dynamic_cast<ExprsList*>(node)) {
        visit(exprsList, context);
    } else if (auto programNode = dynamic_cast<ProgramNode*>(node)) {
        visit(programNode, context);
    } else if (auto accessNode = dynamic_cast<AccessNode*>(node)) {
        visit(accessNode, context);
    } else if (auto typeAssMemberNode = dynamic_cast<TypeAssMember*>(node)) {
        visit(typeAssMemberNode, context);
    } else if (auto attributeMemberNode = dynamic_cast<AttributeMember*>(node)) {
        visit(attributeMemberNode, context);
    } else if (auto methodMemberNode = dynamic_cast<MethodMember*>(node)) {
        visit(methodMemberNode, context);
    } else if (auto typeCastNode = dynamic_cast<TypeCastNode*>(node)) {
        visit(typeCastNode, context);
    } else {
        throw std::runtime_error("CodegenVisitor::visit(ASTNode*): Unhandled node type: " + std::string(typeid(*node).name()));
    }
}

void CodegenVisitor::visit(FloatNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for Float Node: " << node->value << endl;

    currentValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat((double)node->value));
}

void CodegenVisitor::visit(BoolNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    cout << "Generating code for Bool Node: " << node->value << endl;
    
    currentValue = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*TheContext), node->value ? 1 : 0);
}

void CodegenVisitor::visit(StringNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    cout << "Generating code for String Node: " << node->value << endl;
    
    // Create a global string constant
    std::string cleanStr = node->value;
    if (cleanStr.length() >= 2 && cleanStr.front() == '"' && cleanStr.back() == '"') {
        cleanStr = cleanStr.substr(1, cleanStr.length() - 2);
    }
    
    llvm::Constant* strConstant = llvm::ConstantDataArray::getString(*TheContext, cleanStr);
    
    llvm::GlobalVariable* globalStr = new llvm::GlobalVariable(
        *TheModule,
        strConstant->getType(),
        true,                                
        llvm::GlobalValue::PrivateLinkage,
        strConstant,
        ".str"                              
    );
    
    // Create a GEP instruction to get a pointer to the string
    llvm::Value* zero = llvm::ConstantInt::get(*TheContext, llvm::APInt(32, 0, true));
    std::vector<llvm::Value*> indices = {zero, zero};
    llvm::Value* strPtr = Builder->CreateInBoundsGEP(globalStr->getValueType(), globalStr, indices, "strptr");
    
    // Store the string pointer as the current value
    currentValue = strPtr;
}

void CodegenVisitor::visit(UnaryOpNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for UnaryOp Node: " << node->op << endl;
    
    node->node->accept(this, context);
    
    if (node->op == "-") {
        // Negate the value
        if (currentValue->getType()->isDoubleTy()) {
            currentValue = Builder->CreateFNeg(currentValue, "negtmp");
        } else if (currentValue->getType()->isIntegerTy()) {
            currentValue = Builder->CreateNeg(currentValue, "negtmp");
        }
    } else if (node->op == "!") {
        // Logical not
        if (currentValue->getType()->isIntegerTy(1)) {
            currentValue = Builder->CreateNot(currentValue, "nottmp");
        } else if (currentValue->getType()->isDoubleTy()) {
            // Convert to bool first
            llvm::Value* zero = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
            llvm::Value* cmp = Builder->CreateFCmpOEQ(currentValue, zero, "cmptmp");
            currentValue = cmp;
        }
    }
}

void CodegenVisitor::visit(BinOpNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for BinOp Node: " << node->op << endl;

    node->left->accept(this, context);
    llvm::Value* L = currentValue;
    node->right->accept(this, context);
    llvm::Value* R = currentValue;

    std::string str;
    llvm::raw_string_ostream rso(str);

    L->print(rso);
    std::string leftStr = rso.str();
    str.clear();

    R->print(rso);
    std::string rightStr = rso.str();

    cout << "Left Value: " << leftStr << ", Right Value: " << rightStr << endl;
    
    if (node->op == "+"){
        currentValue = Builder->CreateFAdd(L, R, "addtmp");
    } else if (node->op == "-") {
        currentValue = Builder->CreateFSub(L, R, "subtmp");
    } else if (node->op == "*") {
        currentValue = Builder->CreateFMul(L, R, "multmp");
    } else if (node->op == "/") {
        currentValue = Builder->CreateFDiv(L, R, "divtmp");
    } else if (node->op == "^") {
        llvm::Function* powFunc = llvm::Intrinsic::getDeclaration(
            TheModule.get(),
            llvm::Intrinsic::pow,
            {llvm::Type::getDoubleTy(*TheContext)}
        );
        
        // chequeo de tipos de los operandos
        if (!L->getType()->isDoubleTy()) {
            L = Builder->CreateFPCast(L, llvm::Type::getDoubleTy(*TheContext), "castL");
        }
        
        if (!R->getType()->isDoubleTy()) {
            R = Builder->CreateFPCast(R, llvm::Type::getDoubleTy(*TheContext), "castR");
        }
        
        std::vector<llvm::Value*> args = {L, R};
        currentValue = Builder->CreateCall(powFunc, args, "powtmp");
    } else if (node->op == ">") {
        currentValue = Builder->CreateFCmpOGT(L, R, "cmptmp");
    } else if (node->op == "<") {
        currentValue = Builder->CreateFCmpOLT(L, R, "cmptmp");
    } else if (node->op == ">=") {
        currentValue = Builder->CreateFCmpOGE(L, R, "cmptmp");
    } else if (node->op == "<=") {
        currentValue = Builder->CreateFCmpOLE(L, R, "cmptmp");
    } else if (node->op == "==") {
        currentValue = Builder->CreateFCmpOEQ(L, R, "cmptmp");
    } else if (node->op == "!=") {
        currentValue = Builder->CreateFCmpONE(L, R, "cmptmp");
    } else if (node->op == "@") {
        handleStringConcatenation(L, R, node, context, false);
    } else if (node->op == "@@") {
        handleStringConcatenation(L, R, node, context, true);
    } else if (node->op == "is") {
        auto* rightIdNode = dynamic_cast<IDNode*>(node->right);
        
        std::string targetTypeName = rightIdNode->id_name;
        
        // Check if left operand is an object reference we can track
        auto* leftIdNode = dynamic_cast<IDNode*>(node->left);
        if (leftIdNode) {
            std::string objName = leftIdNode->id_name;
            
            // Look up the object's actual type
            auto typeIt = objectTypes.find(objName);
            if (typeIt != objectTypes.end()) {
                std::string objType = typeIt->second;
                
                // Check type compatibility (exact match or inheritance)
                bool isCompatible = isSubtypeOf(objType, targetTypeName);
                
                currentValue = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*TheContext), 
                                                    isCompatible ? 1 : 0);
                                                    
                cout << "CodeGen: 'is' operator - checking if " << objType 
                     << " is " << targetTypeName << " => " << (isCompatible ? "true" : "false") << endl;
            } else {
                // Object type not found, assume false
                currentValue = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*TheContext), 0);
                cout << "CodeGen: 'is' operator - object type not found, returning false" << endl;
            }
        } else {
            // More complex left operand, assume false for now
            currentValue = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*TheContext), 0);
            cout << "CodeGen: 'is' operator - complex left operand, returning false" << endl;
        }
    } else if (node->op == "=" || node->op == ":=") {
        handleAssignment(node, context);
    } else {
        throw std::runtime_error("Unknown binary operator: " + node->op);
    }

    cout << "Generated code for BinOp Node: " << node->op << endl;
}

void CodegenVisitor::visit(FunctionCallNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for FunctionCall Node: " << node->func_name << endl;

    if (node->func_name == "print") {
        if (node->argument == nullptr) {
            throw std::runtime_error("Print function requires an argument");
        }
        
        StringNode* strNode = nullptr;
        
        // Process string literals directly
        if (dynamic_cast<StringNode*>(node->argument) != nullptr) {
            strNode = dynamic_cast<StringNode*>(node->argument);
        }
        // Process arguments in a node vector (e.g. from parsing expressions)
        else if (auto nodeVector = dynamic_cast<ASTNodeVector*>(node->argument)) {
            if (!nodeVector->children.empty()) {
                // First check for a direct string node
                strNode = dynamic_cast<StringNode*>(nodeVector->children[0]);
                
                // If not a string node, check if it's an ID node and handle accordingly
                if (!strNode && dynamic_cast<IDNode*>(nodeVector->children[0]) != nullptr) {
                    IDNode* idNode = dynamic_cast<IDNode*>(nodeVector->children[0]);
                    idNode->accept(this, context);
                    llvm::Value* varValue = currentValue;
                    
                    // Handle different variable types
                    if (varValue->getType()->isPointerTy()) {
                        // It's a string variable, use the print function directly
                        std::vector<llvm::Value*> args = {varValue};
                        currentValue = Builder->CreateCall(PrintFunc, args, "printcall");
                        cout << "Called print function for string pointer value" << endl;
                        return;
                    } else if (varValue->getType()->isDoubleTy()) {
                        // It's a double variable, use print_double
                        llvm::Function* printDoubleFunc = TheModule->getFunction("print_double");
                        if (printDoubleFunc) {
                            std::vector<llvm::Value*> args = {varValue};
                            currentValue = Builder->CreateCall(printDoubleFunc, args, "printcall");
                            cout << "Called print_double function for double value" << endl;
                            return;
                        }
                    } else if (varValue->getType()->isIntegerTy(1)) {
                        // It's a boolean variable, use print_bool
                        llvm::Function* printBoolFunc = TheModule->getFunction("print_bool");
                        if (printBoolFunc) {
                            std::vector<llvm::Value*> args = {varValue};
                            currentValue = Builder->CreateCall(printBoolFunc, args, "printcall");
                            cout << "Called print_bool function for boolean value" << endl;
                            return;
                        }
                    }
                    
                    // Debug type info when we're not sure how to handle it
                    std::string typeStr;
                    llvm::raw_string_ostream typeStream(typeStr);
                    varValue->getType()->print(typeStream);
                    cout << "DEBUG: ID node value type: " << typeStream.str() << endl;
                }
            }
        }
        
        // Handle string literals
        if (strNode != nullptr) {
            std::string cleanStr = strNode->value;
            if (cleanStr.length() >= 2 && cleanStr.front() == '"' && cleanStr.back() == '"') {
                cleanStr = cleanStr.substr(1, cleanStr.length() - 2);
            }
            
            llvm::Constant* strConstant = llvm::ConstantDataArray::getString(*TheContext, cleanStr);
            
            llvm::GlobalVariable* globalStr = new llvm::GlobalVariable(
                *TheModule,
                strConstant->getType(),
                true,                                
                llvm::GlobalValue::PrivateLinkage,
                strConstant,
                ".str"                              
            );
            
            llvm::Value* zero = llvm::ConstantInt::get(*TheContext, llvm::APInt(32, 0, true));
            std::vector<llvm::Value*> indices = {zero, zero};
            llvm::Value* strPtr = Builder->CreateInBoundsGEP(globalStr->getValueType(), globalStr, indices, "strptr");
            
            std::vector<llvm::Value*> args = {strPtr};
            currentValue = Builder->CreateCall(PrintFunc, args, "printcall");
            cout << "Called print function for string literal" << endl;
            return;
        } 
        // Handle ID nodes directly (variable references)
        else if (dynamic_cast<IDNode*>(node->argument) != nullptr) {
            IDNode* idNode = dynamic_cast<IDNode*>(node->argument);
            
            idNode->accept(this, context);
            llvm::Value* varValue = currentValue;
            
            // Debug the type of the value
            std::string typeStr;
            llvm::raw_string_ostream typeStream(typeStr);
            varValue->getType()->print(typeStream);
            cout << "DEBUG: Variable type for " << idNode->id_name << ": " << typeStream.str() << endl;
            
            if (varValue->getType()->isDoubleTy()) {
                llvm::Function* printDoubleFunc = TheModule->getFunction("print_double");
                if (printDoubleFunc) {
                    std::vector<llvm::Value*> args = {varValue};
                    currentValue = Builder->CreateCall(printDoubleFunc, args, "printcall");
                    cout << "Called print_double function for variable " << idNode->id_name << endl;
                    return;
                }
            } else if (varValue->getType()->isIntegerTy(1)) {
                llvm::Function* printBoolFunc = TheModule->getFunction("print_bool");
                if (printBoolFunc) {
                    std::vector<llvm::Value*> args = {varValue};
                    currentValue = Builder->CreateCall(printBoolFunc, args, "printcall");
                    cout << "Called print_bool function for variable " << idNode->id_name << endl;
                    return;
                }
            } else if (varValue->getType()->isPointerTy()) {
                // Handle string variables
                std::vector<llvm::Value*> args = {varValue};
                currentValue = Builder->CreateCall(PrintFunc, args, "printcall");
                cout << "Called print function for string pointer variable " << idNode->id_name << endl;
                return;
            } else {
                std::string idStr = idNode->id_name;
                llvm::Constant* strConstant = llvm::ConstantDataArray::getString(*TheContext, idStr);
                
                llvm::GlobalVariable* globalStr = new llvm::GlobalVariable(
                    *TheModule,
                    strConstant->getType(),
                    true,
                    llvm::GlobalValue::PrivateLinkage,
                    strConstant,
                    ".str"
                );
                
                llvm::Value* zero = llvm::ConstantInt::get(*TheContext, llvm::APInt(32, 0, true));
                std::vector<llvm::Value*> indices = {zero, zero};
                llvm::Value* strPtr = Builder->CreateInBoundsGEP(globalStr->getValueType(), globalStr, indices, "strptr");
                
                std::vector<llvm::Value*> args = {strPtr};
                currentValue = Builder->CreateCall(PrintFunc, args, "printcall");
                cout << "Called print function for variable name as string: " << idNode->id_name << endl;
            }
            return;
        }
        // Handle other expressions (method calls, field access, etc.)
        else {
            node->argument->accept(this, context);
            llvm::Value* argValue = currentValue;
            
            // Debug the type of the value
            std::string typeStr;
            llvm::raw_string_ostream typeStream(typeStr);
            argValue->getType()->print(typeStream);
            cout << "DEBUG: Expression result type: " << typeStream.str() << endl;
            
            if (argValue->getType()->isDoubleTy()) {
                llvm::Function* printDoubleFunc = TheModule->getFunction("print_double");
                if (printDoubleFunc) {
                    std::vector<llvm::Value*> args = {argValue};
                    currentValue = Builder->CreateCall(printDoubleFunc, args, "printcall");
                    cout << "Called print_double function for expression result" << endl;
                    return;
                }
            } else if (argValue->getType()->isIntegerTy(1)) {
                llvm::Function* printBoolFunc = TheModule->getFunction("print_bool");
                if (printBoolFunc) {
                    std::vector<llvm::Value*> args = {argValue};
                    currentValue = Builder->CreateCall(printBoolFunc, args, "printcall");
                    cout << "Called print_bool function for expression result" << endl;
                    return;
                }
            } else if (argValue->getType()->isPointerTy()) {
                // Handle string values returned from field access or methods
                std::vector<llvm::Value*> args = {argValue};
                currentValue = Builder->CreateCall(PrintFunc, args, "printcall");
                cout << "Called print function for string pointer expression result" << endl;
                return;
            }
            
            std::cerr << "Warning: Printing value type not implemented: " << typeStr << std::endl;
            return;
        }
    }
    // Handle standard library math functions
    else if (node->func_name == "sqrt" || node->func_name == "sin" || 
             node->func_name == "cos" || node->func_name == "exp") {
        // These functions take one argument
        if (node->argument == nullptr) {
            throw std::runtime_error("Function " + node->func_name + " requires one argument");
        }
        
        // Process the argument
        std::vector<llvm::Value*> args;
        if (auto* nodeVector = dynamic_cast<ASTNodeVector*>(node->argument)) {
            if (nodeVector->children.size() != 1) {
                throw std::runtime_error("Function " + node->func_name + " requires exactly one argument");
            }
            nodeVector->children[0]->accept(this, context);
            args.push_back(currentValue);
        } else {
            node->argument->accept(this, context);
            args.push_back(currentValue);
        }
        
        // Get the corresponding LLVM function
        std::string funcName = node->func_name + "_func";
        llvm::Function* mathFunc = TheModule->getFunction(funcName);
        if (!mathFunc) {
            throw std::runtime_error("Math function not found: " + funcName);
        }
        
        currentValue = Builder->CreateCall(mathFunc, args, node->func_name + "call");
        cout << "Called " << node->func_name << " function" << endl;
        return;
    }
    else if (node->func_name == "log") {
        // Log function takes two arguments: base and value
        if (node->argument == nullptr) {
            throw std::runtime_error("Function log requires two arguments: base and value");
        }
        
        std::vector<llvm::Value*> args;
        if (auto* nodeVector = dynamic_cast<ASTNodeVector*>(node->argument)) {
            if (nodeVector->children.size() != 2) {
                throw std::runtime_error("Function log requires exactly two arguments: base and value");
            }
            // Process base argument
            nodeVector->children[0]->accept(this, context);
            args.push_back(currentValue);
            // Process value argument
            nodeVector->children[1]->accept(this, context);
            args.push_back(currentValue);
        } else {
            throw std::runtime_error("Function log requires two arguments, but only one provided");
        }
        
        llvm::Function* logFunc = TheModule->getFunction("log_func");
        if (!logFunc) {
            throw std::runtime_error("Log function not found: log_func");
        }
        
        currentValue = Builder->CreateCall(logFunc, args, "logcall");
        cout << "Called log function with base and value" << endl;
        return;
    }
    else if (node->func_name == "rand") {
        // Rand function takes no arguments
        if (node->argument != nullptr) {
            // Check if it's an empty argument list
            if (auto* nodeVector = dynamic_cast<ASTNodeVector*>(node->argument)) {
                if (!nodeVector->children.empty()) {
                    throw std::runtime_error("Function rand takes no arguments");
                }
            } else {
                throw std::runtime_error("Function rand takes no arguments");
            }
        }
        
        llvm::Function* randFunc = TheModule->getFunction("rand_func");
        if (!randFunc) {
            throw std::runtime_error("Rand function not found: rand_func");
        }
        
        currentValue = Builder->CreateCall(randFunc, {}, "randcall");
        cout << "Called rand function" << endl;
        return;
    }
    else {
        llvm::Function* calledFunc = TheModule->getFunction(node->func_name);
        if (!calledFunc) {
            throw std::runtime_error("Unknown function referenced: " + node->func_name);
        }

        std::vector<llvm::Value*> args;
        if (node->argument != nullptr) {
            // Check if the argument is an ASTNodeVector (multiple arguments)
            if (auto* nodeVector = dynamic_cast<ASTNodeVector*>(node->argument)) {
                cout << "Processing " << nodeVector->children.size() << " arguments for function " << node->func_name << endl;
                for (auto* argNode : nodeVector->children) {
                    argNode->accept(this, context);
                    if (currentValue) {
                        args.push_back(currentValue);
                    }
                }
            } else {
                // Single argument
                node->argument->accept(this, context);
                if (currentValue) {
                    args.push_back(currentValue);
                }
            }
        }

        currentValue = Builder->CreateCall(calledFunc, args, "calltmp");
        cout << "Called user function '" << node->func_name << "' with " << args.size() << " arguments" << endl;
    }
}

void CodegenVisitor::visit(IDNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    if (node->id_name == "PI") {
        llvm::Function* getPiFunc = TheModule->getFunction("get_pi");
        if (getPiFunc) {
            currentValue = Builder->CreateCall(getPiFunc);
        } else {
            currentValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(3.14159265359));
            std::cerr << "Warning: get_pi function not found, using constant" << std::endl;
        }
    } else if (node->id_name == "E") {
        llvm::Function* getEFunc = TheModule->getFunction("get_e");
        if (getEFunc) {
            currentValue = Builder->CreateCall(getEFunc);
        } else {
            currentValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(2.71828182846));
            std::cerr << "Warning: get_e function not found, using constant" << std::endl;
        }
    } else {
        auto it = NamedValues.find(node->id_name);
        if (it != NamedValues.end()) {
            llvm::Value* varValue = it->second;
            if (llvm::isa<llvm::AllocaInst>(varValue)) {
                llvm::AllocaInst* allocaInst = llvm::cast<llvm::AllocaInst>(varValue);
                llvm::Type* allocaType = allocaInst->getAllocatedType();
                
                // Debug allocated type
                std::string typeStr;
                llvm::raw_string_ostream typeStream(typeStr);
                allocaType->print(typeStream);
                
                currentValue = Builder->CreateLoad(allocaType, varValue, node->id_name);
                
                // Debug loaded value type
                std::string valTypeStr;
                llvm::raw_string_ostream valTypeStream(valTypeStr);
                currentValue->getType()->print(valTypeStream);
                cout << "DEBUG: Loaded " << node->id_name << " with type: " << valTypeStream.str() << endl;
                
                cout << "Loaded value from alloca for variable: " << node->id_name << endl;
            } else {
                currentValue = varValue;
                
                std::string valTypeStr;
                llvm::raw_string_ostream valTypeStream(valTypeStr);
                currentValue->getType()->print(valTypeStream);
                valTypeStream.str(); // Flush the stream
                cout << "DEBUG: Using direct value for " << node->id_name << " with type: " << valTypeStr << endl;
            }
        } else {
            // In a real compiler, we would look up the variable in the symbol table
            // For now, just use a default value
            currentValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
            std::cerr << "Warning: Undefined variable '" << node->id_name << "', using 0.0" << std::endl;
        }
    }
}

void CodegenVisitor::visit(BlockNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for Block Node" << endl;
    
    for (auto child : node->children) {
        cout << "Visiting child node in Block" << endl;
        if (child == nullptr) {
            throw std::runtime_error("Child node is null");
        }
        child->accept(this, context);
    }
    
    cout << "Block Node: Completed visiting all children" << endl;
}

void CodegenVisitor::visit(ArgsList* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    for (auto* arg : node->children) {
        if (arg == nullptr) {
            throw std::runtime_error("Argument node is null");
        }
        arg->accept(this, context);
        
        // Store the current value in NamedValues for later use
        NamedValues[arg->id_name] = currentValue;
    }

    cout << "Generating code for ArgsList" << endl;
}

void CodegenVisitor::visit(AssignFuncNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for AssignFuncNode: " << node->func_name << endl;
    
    // Determine argument types based on their declared types
    std::vector<llvm::Type*> argTypes;
    for (auto* arg : node->args->children) {
        if (arg->id_type == "String") {
            argTypes.push_back(llvm::Type::getInt8PtrTy(*TheContext)); // String as char*
        } else if (arg->id_type == "Boolean") {
            argTypes.push_back(llvm::Type::getInt1Ty(*TheContext)); // Boolean as i1
        }
        else {
            argTypes.push_back(llvm::Type::getDoubleTy(*TheContext)); // Default to double for Number
        }
    }
    
    // Create function type (returns double for simplicity)
    llvm::FunctionType* funcType;
    if (node->inferredType->name == "String") {
        funcType = llvm::FunctionType::get(
            llvm::Type::getInt8PtrTy(*TheContext), // Return type for String
            argTypes,
            false
        );
    } else if (node->inferredType->name == "Boolean") {
        funcType = llvm::FunctionType::get(
            llvm::Type::getInt1Ty(*TheContext), // Return type for Boolean
            argTypes,
            false
        );
    } else {
        // Default to double return type
        funcType = llvm::FunctionType::get(
            llvm::Type::getDoubleTy(*TheContext),
            argTypes,
            false
        );
    }
    
    // Create the function
    llvm::Function* function = llvm::Function::Create(
        funcType,
        llvm::Function::ExternalLinkage,
        node->func_name,
        TheModule.get()
    );
    
    // Create basic block for function body
    llvm::BasicBlock* funcBB = llvm::BasicBlock::Create(*TheContext, "entry", function);
    
    // Save current builder state
    llvm::BasicBlock* prevBB = Builder->GetInsertBlock();
    Builder->SetInsertPoint(funcBB);
    
    // Save current named values and object types
    std::map<std::string, llvm::Value*> prevNamedValues = NamedValues;
    std::map<std::string, std::string> prevObjectTypes = objectTypes;
    
    // Set up function parameters
    auto argIt = function->arg_begin();
    for (size_t i = 0; i < node->args->children.size(); i++) {
        llvm::Value* arg = &(*argIt++);
        arg->setName(node->args->children[i]->id_name);
        
        // Create alloca for the argument based on its type
        llvm::AllocaInst* alloca = nullptr;
        if (node->args->children[i]->id_type == "String") {
            alloca = Builder->CreateAlloca(
                llvm::Type::getInt8PtrTy(*TheContext), 
                nullptr, 
                node->args->children[i]->id_name
            );
        } else {
            alloca = Builder->CreateAlloca(
                llvm::Type::getDoubleTy(*TheContext), 
                nullptr, 
                node->args->children[i]->id_name
            );
        }
        
        Builder->CreateStore(arg, alloca);
        NamedValues[node->args->children[i]->id_name] = alloca;
        
        // Store the parameter type in objectTypes map for proper type resolution
        if (!node->args->children[i]->id_type.empty() && 
            node->args->children[i]->id_type != "Number" && 
            node->args->children[i]->id_type != "Boolean" &&
            node->args->children[i]->id_type != "String") {
            objectTypes[node->args->children[i]->id_name] = node->args->children[i]->id_type;
            cout << "Stored parameter '" << node->args->children[i]->id_name 
                 << "' with object type '" << node->args->children[i]->id_type << "'" << endl;
        }
        
        cout << "Parameter '" << node->args->children[i]->id_name << "' of type " << 
                node->args->children[i]->id_type << " allocated" << endl;
    }
    
    // Generate function body
    node->body->accept(this, context);
    
    // Create return statement
    if (currentValue) {
        Builder->CreateRet(currentValue);
    } else {
        // Default return value if no explicit return
        Builder->CreateRet(llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0)));
    }
    
    // Restore previous state
    NamedValues = prevNamedValues;
    objectTypes = prevObjectTypes;
    if (prevBB) {
        Builder->SetInsertPoint(prevBB);
    }
    
    cout << "Function '" << node->func_name << "' generated successfully with " << 
            argTypes.size() << " parameters" << endl;
}

void CodegenVisitor::visit(LetAssign* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for LetAssign" << endl;

    std::map<std::string, llvm::Value*> prevValues;
    
    for (auto* assign : node->assigns) {
        auto it = NamedValues.find(assign->var_id->id_name);
        if (it != NamedValues.end()) {
            prevValues[assign->var_id->id_name] = it->second;
        }
        
        assign->value->accept(this, context);
        
        // Handle assignment from a NewTypeNode
        if (auto* newTypeNode = dynamic_cast<NewTypeNode*>(assign->value)) {
            std::string typeName = newTypeNode->id_type_name;
            objectInstances[assign->var_id->id_name] = currentValue;
            objectTypes[assign->var_id->id_name] = typeName;
            
            llvm::Function* function = Builder->GetInsertBlock()->getParent();
            llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
            
            llvm::AllocaInst* alloca = tmpBuilder.CreateAlloca(
                llvm::PointerType::get(llvm::Type::getInt8Ty(*TheContext), 0), 
                nullptr, 
                assign->var_id->id_name
            );
            
            Builder->CreateStore(currentValue, alloca);
            NamedValues[assign->var_id->id_name] = alloca;
            
            cout << "Let variable '" << assign->var_id->id_name 
                 << "' allocated as new object of type '" << typeName << "'" << endl;
            continue;
        }
        // Handle assignment from a TypeCastNode
        else if (auto* typeCastNode = dynamic_cast<TypeCastNode*>(assign->value)) {
            // Generate the type cast
            assign->value->accept(this, context);
            
            llvm::Function* function = Builder->GetInsertBlock()->getParent();
            llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
            
            llvm::AllocaInst* alloca = tmpBuilder.CreateAlloca(currentValue->getType(), nullptr, assign->var_id->id_name);
            Builder->CreateStore(currentValue, alloca);
            NamedValues[assign->var_id->id_name] = alloca;
            
            // Record the cast target type in objectTypes
            objectTypes[assign->var_id->id_name] = typeCastNode->target_type;
            
            cout << "Let variable '" << assign->var_id->id_name 
                 << "' allocated as type cast result to type '" << typeCastNode->target_type << "'" << endl;
            continue;
        }
        // Handle assignment from an IDNode (reference to existing object)
        else if (auto* idNode = dynamic_cast<IDNode*>(assign->value)) {
            std::string sourceVarName = idNode->id_name;
            
            auto objIt = objectInstances.find(sourceVarName);
            if (objIt != objectInstances.end()) {
                objectInstances[assign->var_id->id_name] = objIt->second;
                
                llvm::Function* function = Builder->GetInsertBlock()->getParent();
                llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
                
                llvm::AllocaInst* alloca = tmpBuilder.CreateAlloca(
                    llvm::PointerType::get(llvm::Type::getInt8Ty(*TheContext), 0), 
                    nullptr, 
                    assign->var_id->id_name
                );
                
                Builder->CreateStore(objIt->second, alloca);
                NamedValues[assign->var_id->id_name] = alloca;
                
                if (assign->var_id->inferredType && assign->var_id->inferredType->name != "unknown") {
                    objectTypes[assign->var_id->id_name] = assign->var_id->inferredType->name;
                    cout << "Assigned object reference '" << assign->var_id->id_name 
                         << "' with type '" << assign->var_id->inferredType->name << "'" << endl;
                } else {
                    auto typeIt = objectTypes.find(sourceVarName);
                    if (typeIt != objectTypes.end()) {
                        objectTypes[assign->var_id->id_name] = typeIt->second;
                        cout << "Assigned object reference '" << assign->var_id->id_name 
                             << "' with inherited type '" << typeIt->second << "'" << endl;
                    }
                }
                
                cout << "Let variable '" << assign->var_id->id_name << "' allocated as object reference" << endl;
                continue;
            }
        }
        
        llvm::Function* function = Builder->GetInsertBlock()->getParent();
        llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
        
        // Debug current value type
        std::string typeStr;
        llvm::raw_string_ostream typeStream(typeStr);
        currentValue->getType()->print(typeStream);
        cout << "DEBUG: Variable " << assign->var_id->id_name << " assignment value type: " << typeStream.str() << endl;
        
        // Set the appropriate allocation type based on the value type
        llvm::Type* allocaType = llvm::Type::getDoubleTy(*TheContext);
        
        // Check if we're dealing with a string literal
        if (auto* strNode = dynamic_cast<StringNode*>(assign->value)) {
            allocaType = llvm::Type::getInt8PtrTy(*TheContext);
            cout << "DEBUG: String literal detected, using pointer type" << endl;
        }
        // Check if value is a pointer type (including struct string fields)
        else if (currentValue->getType()->isPointerTy()) {
            allocaType = currentValue->getType();
            cout << "DEBUG: Pointer type detected, using " << typeStream.str() << endl;
        }
        
        llvm::AllocaInst* alloca = tmpBuilder.CreateAlloca(allocaType, nullptr, assign->var_id->id_name);
        
        Builder->CreateStore(currentValue, alloca);
        
        NamedValues[assign->var_id->id_name] = alloca;
        
        if (allocaType->isPointerTy() && assign->var_id->inferredType && 
            assign->var_id->inferredType->name == "String") {
            cout << "DEBUG: Marking " << assign->var_id->id_name << " as String type" << endl;
            // Can track string variables in a separate map if needed
        }
        
        cout << "Let variable '" << assign->var_id->id_name << "' allocated and assigned with type " 
             << typeStream.str() << endl;
    }

    node->body->accept(this, context);

    for (auto* assign : node->assigns) {
        auto prevIt = prevValues.find(assign->var_id->id_name);
        if (prevIt != prevValues.end()) {
            NamedValues[assign->var_id->id_name] = prevIt->second;
        } else {
            NamedValues.erase(assign->var_id->id_name);
        }
    }
}

void CodegenVisitor::visit(VarAssign* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for VarAssign: " << node->var_id->id_name << endl;

    node->value->accept(this, context);
    
    // The currentValue now contains the result of evaluating the value expression
    // In a full implementation, we might want to create an alloca and store instruction
    // For now, we'll just keep track of the value in our NamedValues map
    
    cout << "Variable '" << node->var_id->id_name << "' assigned value" << endl;
}

void CodegenVisitor::visit(NewTypeNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for NewTypeNode: new " << node->id_type_name << endl;

    std::string typeName = node->id_type_name;
    
    // Find the constructor function for this type
    std::string constructorName = "new_" + typeName;
    llvm::Function* constructorFunc = TheModule->getFunction(constructorName);
    
    if (!constructorFunc) {
        throw std::runtime_error("Constructor function not found: " + constructorName);
    }
    
    // Process constructor arguments (if any)
    std::vector<llvm::Value*> args;
    for (auto* expr : node->expr_list) {
        expr->accept(this, context);
        if (currentValue) {
            args.push_back(currentValue);
        }
    }
    
    // Call the constructor function to create new instance
    llvm::Value* newInstance = Builder->CreateCall(constructorFunc, args, "new_instance");
    
    // Set the current value to the newly created instance
    currentValue = newInstance;
    
    cout << "Created new instance of type " << typeName << " with " << args.size() << " arguments" << endl;
}

void CodegenVisitor::visit(VarAssignList* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for VarAssignList" << endl;

    for (auto* assign : node->assigns) {
        assign->accept(this, context);
    }
}

void CodegenVisitor::visit(VarAssignType* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for VarAssignType: " << node->var_name << " := new " << node->new_type->id_type_name << endl;

    std::string typeName = node->new_type->id_type_name;
    
    std::string constructorName = "new_" + typeName;
    llvm::Function* constructorFunc = TheModule->getFunction(constructorName);
    
    if (!constructorFunc) {
        throw std::runtime_error("Constructor function not found: " + constructorName);
    }
    
    llvm::Value* newInstance = Builder->CreateCall(constructorFunc, {}, "new_" + node->var_name);
    
    NamedValues[node->var_name] = newInstance;
    objectInstances[node->var_name] = newInstance;
    objectTypes[node->var_name] = typeName;
    
    cout << "Created new instance of " << typeName << " assigned to " << node->var_name << endl;
    
    node->body->accept(this, context);
    
    cout << "VarAssignType: Object instantiation and body execution completed" << endl;
}

void CodegenVisitor::visit(Conditional* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for Conditional" << endl;

    node->bool_expr->accept(this, context);
    llvm::Value* condValue = currentValue;

    llvm::Function* function = Builder->GetInsertBlock()->getParent();

    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*TheContext, "then", function);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(*TheContext, "else");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*TheContext, "ifcont");

    Builder->CreateCondBr(condValue, thenBB, elseBB);

    Builder->SetInsertPoint(thenBB);
    node->if_body->accept(this, context);
    llvm::Value* thenValue = currentValue;
    
    if (!Builder->GetInsertBlock()->getTerminator()) {
        Builder->CreateBr(mergeBB);
    }
    thenBB = Builder->GetInsertBlock();

    function->getBasicBlockList().push_back(elseBB);
    Builder->SetInsertPoint(elseBB);
    node->else_body->accept(this, context);
    llvm::Value* elseValue = currentValue;
    
    if (!Builder->GetInsertBlock()->getTerminator()) {
        Builder->CreateBr(mergeBB);
    }
    elseBB = Builder->GetInsertBlock();

    function->getBasicBlockList().push_back(mergeBB);
    Builder->SetInsertPoint(mergeBB);

    // Determine the type for the PHI node based on the branch values
    llvm::Type* phiType = nullptr;
    if (thenValue && elseValue) {
        if (thenValue->getType() == elseValue->getType()) {
            phiType = thenValue->getType();
        } else if (thenValue->getType()->isPointerTy() && elseValue->getType()->isPointerTy()) {
            // Both are pointers (strings), use the common pointer type
            phiType = llvm::Type::getInt8PtrTy(*TheContext);
        } else {
            // Default to double for mixed types
            phiType = llvm::Type::getDoubleTy(*TheContext);
        }
    } else {
        // Default to double if we can't determine type
        phiType = llvm::Type::getDoubleTy(*TheContext);
    }

    // Special case: if both branches return void, don't create a PHI node
    if (phiType && phiType->isVoidTy()) {
        // Both branches return void, just set currentValue to a dummy value
        currentValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
        cout << "Conditional: Both branches return void, using dummy value" << endl;
        return;
    }

    llvm::PHINode* phiNode = Builder->CreatePHI(phiType, 2, "iftmp");
    
    // Add incoming values with type conversion if needed
    if (thenValue) {
        if (thenValue->getType() == phiType) {
            phiNode->addIncoming(thenValue, thenBB);
        } else if (phiType->isPointerTy() && thenValue->getType()->isPointerTy()) {
            // Cast pointer types if needed
            llvm::Value* castedThen = Builder->CreateBitCast(thenValue, phiType, "then_cast");
            phiNode->addIncoming(castedThen, thenBB);
        } else {
            // Default conversion
            llvm::Value* defaultThen = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
            phiNode->addIncoming(defaultThen, thenBB);
        }
    } else {
        llvm::Value* defaultThen = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
        phiNode->addIncoming(defaultThen, thenBB);
    }
    
    if (elseValue) {
        if (elseValue->getType() == phiType) {
            phiNode->addIncoming(elseValue, elseBB);
        } else if (phiType->isPointerTy() && elseValue->getType()->isPointerTy()) {
            // Cast pointer types if needed
            llvm::Value* castedElse = Builder->CreateBitCast(elseValue, phiType, "else_cast");
            phiNode->addIncoming(castedElse, elseBB);
        } else {
            // Default conversion
            llvm::Value* defaultElse = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
            phiNode->addIncoming(defaultElse, elseBB);
        }
    } else {
        llvm::Value* defaultElse = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
        phiNode->addIncoming(defaultElse, elseBB);
    }

    currentValue = phiNode;
    
    cout << "Conditional: PHI node created with type ";
    std::string typeStr;
    llvm::raw_string_ostream typeStream(typeStr);
    phiType->print(typeStream);
    cout << typeStream.str() << endl;
}

void CodegenVisitor::visit(BoolExprNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for BoolExprNode" << endl;

    node->expr->accept(this, context);
    
    if (!currentValue) {
        throw std::runtime_error("BoolExprNode: Expression evaluation failed");
    }
    
    cout << "BoolExprNode: Expression evaluated, checking type..." << endl;
    
    if (currentValue->getType()->isIntegerTy(1)) {
        cout << "BoolExprNode: Expression is already boolean type" << endl;
    } else if (currentValue->getType()->isDoubleTy()) {
        llvm::Value* zero = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
        currentValue = Builder->CreateFCmpONE(currentValue, zero, "booltmp");
        cout << "BoolExprNode: Converted double to boolean" << endl;
    } else {
        std::string typeStr;
        llvm::raw_string_ostream typeStream(typeStr);
        currentValue->getType()->print(typeStream);
        throw std::runtime_error("BoolExprNode: Unsupported expression type: " + typeStream.str());
    }
    
    cout << "BoolExprNode: Successfully processed" << endl;
}

void CodegenVisitor::visit(WhileNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for WhileNode" << endl;

    llvm::Function* function = Builder->GetInsertBlock()->getParent();
    if (!function) {
        throw std::runtime_error("WhileNode: No current function found");
    }
    
    llvm::BasicBlock* condBB = llvm::BasicBlock::Create(*TheContext, "while.cond", function);
    llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(*TheContext, "while.body", function);
    llvm::BasicBlock* endBB = llvm::BasicBlock::Create(*TheContext, "while.end", function);
    
    Builder->CreateBr(condBB);
    
    Builder->SetInsertPoint(condBB);
    node->bool_expr->accept(this, context);
    llvm::Value* condValue = currentValue;
    
    if (!condValue) {
        throw std::runtime_error("WhileNode: Condition evaluation failed");
    }
    
    Builder->CreateCondBr(condValue, bodyBB, endBB);
    
    Builder->SetInsertPoint(bodyBB);
    node->body->accept(this, context);
    Builder->CreateBr(condBB);
    
    Builder->SetInsertPoint(endBB);
    currentValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
    
    cout << "WhileNode: Successfully generated (simplified version)" << endl;
}

void CodegenVisitor::visit(VarDesAssign* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for VarDesAssign: " << node->id->id_name << endl;

    node->value->accept(this, context);
    llvm::Value* assignValue = currentValue;

    auto it = NamedValues.find(node->id->id_name);
    if (it != NamedValues.end() && llvm::isa<llvm::AllocaInst>(it->second)) {
        Builder->CreateStore(assignValue, it->second);
        currentValue = assignValue;
    } else {
        throw std::runtime_error("Variable '" + node->id->id_name + "' not found or not assignable");
    }
}

void CodegenVisitor::visit(ForNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for ForNode: " << node->id->id_name << endl;
    
    // For now, provide a basic implementation
    // A full implementation would need to handle iteration over collections
    
    // Visit the group/collection
    node->group->accept(this, context);
    
    llvm::Function* function = Builder->GetInsertBlock()->getParent();
    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(*TheContext, "for.loop", function);
    llvm::BasicBlock* endBB = llvm::BasicBlock::Create(*TheContext, "for.end", function);
    
    Builder->CreateBr(loopBB);
    
    Builder->SetInsertPoint(loopBB);
    node->body->accept(this, context);
    
    Builder->CreateBr(endBB);
    
    Builder->SetInsertPoint(endBB);
    currentValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
    
    cout << "ForNode: Basic structure generated" << endl;
}

void CodegenVisitor::visit(TypeDeclNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for TypeDeclNode: " << node->id->id_name << endl;
        
    std::string typeName = node->id->id_name;
    
    std::vector<llvm::Type*> structFields;
    
    std::vector<std::string> attributes;
    std::vector<AssignFuncNode*> methods;
    
    // Use the member variable from visitor.h for attribute types
    AttributeTypeMap attrTypeMap;
    
    if (!node->parents.empty()) {
        std::string parentTypeName = node->parents[0];
        cout << "Type " << typeName << " inherits from " << parentTypeName << endl;
        auto parentStructIt = typeStructMap.find(parentTypeName);
        auto parentAttributesIt = typeAttributeMap.find(parentTypeName);
        auto parentAttrTypeIt = typeAttributeTypeMap.find(parentTypeName);
        
        if (parentStructIt != typeStructMap.end() && parentAttributesIt != typeAttributeMap.end() && parentAttrTypeIt != typeAttributeTypeMap.end()) {
            llvm::StructType* parentStruct = parentStructIt->second;
            for (unsigned i = 0; i < parentStruct->getNumElements(); ++i) {
                structFields.push_back(parentStruct->getElementType(i));
            }
            
            attributes = parentAttributesIt->second;
            
            // CRITICAL FIX: Copy the parent's attribute type map to inherit field types
            const AttributeTypeMap& parentAttrTypeMap = parentAttrTypeIt->second;
            attrTypeMap = parentAttrTypeMap;
            
            cout << "Inherited structure from " << parentTypeName << " with " << 
                    attributes.size() << " attributes and " << attrTypeMap.size() << " attribute types" << endl;
        } else {
            cout << "Warning: Parent type " << parentTypeName << " not found for inheritance" << endl;
        }
    }
    
    for (auto* member : node->body) {
        if (auto* varAssign = dynamic_cast<VarAssign*>(member)) {
            auto it = std::find(attributes.begin(), attributes.end(), varAssign->var_id->id_name);
            if (it == attributes.end()) {
                // Infer type
                llvm::Type* fieldType = llvm::Type::getDoubleTy(*TheContext);
                if (auto* strNode = dynamic_cast<StringNode*>(varAssign->value)) {
                    fieldType = llvm::Type::getInt8PtrTy(*TheContext);
                }
                structFields.push_back(fieldType);
                attributes.push_back(varAssign->var_id->id_name);
                attrTypeMap[varAssign->var_id->id_name] = fieldType;
            }
        } else if (auto* funcNode = dynamic_cast<AssignFuncNode*>(member)) {
            methods.push_back(funcNode);
        }
    }
    
    llvm::StructType* structType = llvm::StructType::create(*TheContext, structFields, typeName);
    
    typeStructMap[typeName] = structType;
    typeAttributeMap[typeName] = attributes;
    this->typeAttributeTypeMap[typeName] = attrTypeMap;
    
    // Track inheritance relationships
    if (!node->parents.empty()) {
        std::string parentTypeName = node->parents[0];
        typeInheritanceMap[typeName] = parentTypeName;
        cout << "Recorded inheritance: " << typeName << " inherits from " << parentTypeName << endl;
    }
    
    cout << "DEBUG: Stored attributes for type " << typeName << ": ";
    for (const auto& attr : attributes) {
        cout << attr << " ";
    }
    cout << endl;
    
    cout << "DEBUG: Stored attribute types for type " << typeName << ": ";
    for (const auto& pair : attrTypeMap) {
        const std::string& attrName = pair.first;
        llvm::Type* attrType = pair.second;
        std::string typeStr;
        llvm::raw_string_ostream typeStream(typeStr);
        attrType->print(typeStream);
        typeStream.str(); // Flush the stream
        cout << attrName << ": " << typeStr << ", ";
    }
    cout << endl;
    for (const auto& attr : attributes) {
        cout << attr << " ";
    }
    cout << endl;
    
    llvm::FunctionType* constructorType = llvm::FunctionType::get(
        llvm::PointerType::get(structType, 0),  
        {},                                     
        false                                   
    );
    
    llvm::Function* constructorFunc = llvm::Function::Create(
        constructorType,
        llvm::Function::ExternalLinkage,
        "new_" + typeName,
        TheModule.get()
    );
    
    llvm::BasicBlock* constructorBB = llvm::BasicBlock::Create(*TheContext, "entry", constructorFunc);
    llvm::IRBuilder<> constructorBuilder(*TheContext);
    constructorBuilder.SetInsertPoint(constructorBB);
    
    // Allocate memory for the new instance using hulk_malloc
    llvm::Function* mallocFunc = TheModule->getFunction("hulk_malloc");
    if (!mallocFunc) {
        // Declare hulk_malloc if not already declared
        llvm::FunctionType* mallocType = llvm::FunctionType::get(
            llvm::Type::getInt8PtrTy(*TheContext),
            {llvm::Type::getInt64Ty(*TheContext)},
            false
        );
        mallocFunc = llvm::Function::Create(
            mallocType,
            llvm::Function::ExternalLinkage,
            "hulk_malloc",
            TheModule.get()
        );
    }
    
    // Calculate the size of the struct
    llvm::DataLayout dataLayout(TheModule.get());
    uint64_t structSize = dataLayout.getTypeAllocSize(structType);
    llvm::Value* sizeValue = llvm::ConstantInt::get(llvm::Type::getInt64Ty(*TheContext), structSize);
    
    // Call malloc to allocate memory
    llvm::Value* mallocResult = constructorBuilder.CreateCall(mallocFunc, {sizeValue}, "malloc_result");
    
    // Cast the result to the correct struct type pointer
    llvm::Value* newInstance = constructorBuilder.CreateBitCast(
        mallocResult,
        llvm::PointerType::get(structType, 0),
        "new_instance"
    );
    
    // Set specific values for attributes defined in the body
    for (auto* member : node->body) {
        if (auto* varAssign = dynamic_cast<VarAssign*>(member)) {
            if (!varAssign->var_id || !varAssign->value) {
                std::cerr << "Warning: Invalid var assignment in type declaration" << std::endl;
                continue;
            }
            auto it = std::find(attributes.begin(), attributes.end(), varAssign->var_id->id_name);
            if (it != attributes.end()) {
                size_t fieldIndex = std::distance(attributes.begin(), it);
                llvm::Value* fieldPtr = constructorBuilder.CreateStructGEP(structType, newInstance, fieldIndex, "field_" + varAssign->var_id->id_name);
                llvm::Type* fieldType = attrTypeMap[varAssign->var_id->id_name];
                llvm::Value* defaultVal = nullptr;
                DefaultValue defaultValueData; // Store the raw value for inheritance
                
                if (fieldType->isDoubleTy()) {
                    double doubleValue = 0.0;
                    if (auto* floatNode = dynamic_cast<FloatNode*>(varAssign->value)) {
                        doubleValue = (double)floatNode->value;
                    }
                    defaultVal = llvm::ConstantFP::get(fieldType, llvm::APFloat(doubleValue));
                    defaultValueData = DefaultValue(doubleValue);
                } else if (fieldType->isPointerTy()) {
                    std::string cleanStr = "";
                    if (auto* strNode = dynamic_cast<StringNode*>(varAssign->value)) {
                        cleanStr = strNode->value;
                        if (cleanStr.length() >= 2 && cleanStr.front() == '"' && cleanStr.back() == '"') {
                            cleanStr = cleanStr.substr(1, cleanStr.length() - 2);
                        }
                    }
                    llvm::Constant* strConstant = llvm::ConstantDataArray::getString(*TheContext, cleanStr);
                    llvm::GlobalVariable* globalStr = new llvm::GlobalVariable(
                        *TheModule,
                        strConstant->getType(),
                        true,
                        llvm::GlobalValue::PrivateLinkage,
                        strConstant,
                        ".str"
                    );
                    llvm::Value* zero = llvm::ConstantInt::get(*TheContext, llvm::APInt(32, 0, true));
                    std::vector<llvm::Value*> indices = {zero, zero};
                    defaultVal = constructorBuilder.CreateInBoundsGEP(globalStr->getValueType(), globalStr, indices, "strptr");
                    defaultValueData = DefaultValue(cleanStr);
                }
                if (defaultVal) {
                    constructorBuilder.CreateStore(defaultVal, fieldPtr);
                    // Store the default value in our map for inheritance
                    typeDefaultValuesMap[typeName][varAssign->var_id->id_name] = defaultValueData;
                    cout << "Set and stored default value for field " << varAssign->var_id->id_name << " in type " << typeName << endl;
                }
            }
        }
    }
    
    // Handle inherited field initialization  
    if (!node->parents.empty()) {
        std::string parentTypeName = node->parents[0];
        
        // Set inherited field values (for fields not defined in current body)
        for (size_t i = 0; i < attributes.size(); ++i) {
            std::string attrName = attributes[i];
            
            // Check if this field is NOT defined in the current type's body
            bool definedInCurrentBody = false;
            for (auto* member : node->body) {
                if (auto* varAssign = dynamic_cast<VarAssign*>(member)) {
                    if (varAssign->var_id && varAssign->var_id->id_name == attrName) {
                        definedInCurrentBody = true;
                        break;
                    }
                }
            }
            
            // If not defined in current body, it's inherited - initialize with actual default value from parent
            if (!definedInCurrentBody) {
                llvm::Value* fieldPtr = constructorBuilder.CreateStructGEP(structType, newInstance, i, "inherited_field_" + attrName);
                
                // Try to get the actual default value from the parent type
                llvm::Value* defaultVal = nullptr;
                auto parentDefaultsIt = typeDefaultValuesMap.find(parentTypeName);
                if (parentDefaultsIt != typeDefaultValuesMap.end()) {
                    auto attrDefaultIt = parentDefaultsIt->second.find(attrName);
                    if (attrDefaultIt != parentDefaultsIt->second.end()) {
                        // Found the actual default value from parent, recreate it in current context
                        const DefaultValue& parentDefault = attrDefaultIt->second;
                        
                        // Get field type for current context
                        auto attrTypeIt = typeAttributeTypeMap.find(typeName);
                        if (attrTypeIt != typeAttributeTypeMap.end()) {
                            const AttributeTypeMap& attrTypeMap = attrTypeIt->second;
                            auto fieldTypeIt = attrTypeMap.find(attrName);
                            if (fieldTypeIt != attrTypeMap.end()) {
                                llvm::Type* fieldType = fieldTypeIt->second;
                                
                                if (parentDefault.type == DefaultValue::DOUBLE && fieldType->isDoubleTy()) {
                                    defaultVal = llvm::ConstantFP::get(fieldType, llvm::APFloat(parentDefault.doubleVal));
                                } else if (parentDefault.type == DefaultValue::STRING && fieldType->isPointerTy()) {
                                    llvm::Constant* strConstant = llvm::ConstantDataArray::getString(*TheContext, parentDefault.stringVal);
                                    llvm::GlobalVariable* globalStr = new llvm::GlobalVariable(
                                        *TheModule,
                                        strConstant->getType(),
                                        true,
                                        llvm::GlobalValue::PrivateLinkage,
                                        strConstant,
                                        ".str"
                                    );
                                    llvm::Value* zero = llvm::ConstantInt::get(*TheContext, llvm::APInt(32, 0, true));
                                    std::vector<llvm::Value*> indices = {zero, zero};
                                    defaultVal = constructorBuilder.CreateInBoundsGEP(globalStr->getValueType(), globalStr, indices, "strptr");
                                }
                                
                                if (defaultVal) {
                                    constructorBuilder.CreateStore(defaultVal, fieldPtr);
                                    cout << "Set inherited field " << attrName << " with parent's actual default value (" 
                                         << (parentDefault.type == DefaultValue::DOUBLE ? std::to_string(parentDefault.doubleVal) : parentDefault.stringVal) 
                                         << ") in type " << typeName << endl;
                                    continue;
                                }
                            }
                        }
                    }
                }
                
                // Fallback: if parent default not found, use type-based generic defaults
                auto attrTypeIt = typeAttributeTypeMap.find(typeName);
                if (attrTypeIt != typeAttributeTypeMap.end()) {
                    const AttributeTypeMap& attrTypeMap = attrTypeIt->second;
                    auto fieldTypeIt = attrTypeMap.find(attrName);
                    if (fieldTypeIt != attrTypeMap.end()) {
                        llvm::Type* fieldType = fieldTypeIt->second;
                        
                        if (fieldType->isDoubleTy()) {
                            // For numeric fields, use 0.0 as default
                            defaultVal = llvm::ConstantFP::get(fieldType, llvm::APFloat(0.0));
                        } else if (fieldType->isPointerTy()) {
                            // For string fields, use empty string as default
                            llvm::Constant* strConstant = llvm::ConstantDataArray::getString(*TheContext, "");
                            llvm::GlobalVariable* globalStr = new llvm::GlobalVariable(
                                *TheModule,
                                strConstant->getType(),
                                true,
                                llvm::GlobalValue::PrivateLinkage,
                                strConstant,
                                ".str"
                            );
                            llvm::Value* zero = llvm::ConstantInt::get(*TheContext, llvm::APInt(32, 0, true));
                            std::vector<llvm::Value*> indices = {zero, zero};
                            defaultVal = constructorBuilder.CreateInBoundsGEP(globalStr->getValueType(), globalStr, indices, "inherited_strptr");
                        }
                        
                        if (defaultVal) {
                            constructorBuilder.CreateStore(defaultVal, fieldPtr);
                            cout << "Set inherited field " << attrName << " with generic default value in type " << typeName << " (parent default not found)" << endl;
                        }
                    }
                }
            }
        }
    }
    
    constructorBuilder.CreateRet(newInstance);
    
    // Generate methods for this type
    for (auto* funcNode : methods) {
        generateMethodFunction(typeName, funcNode, structType, attributes);
    }
    
    // Handle inheritance forwarding using LLVM module method discovery
    if (!node->parents.empty()) {
        std::string parentTypeName = node->parents[0];
        typeInheritanceMap[typeName] = parentTypeName; // Track inheritance relationship
        
        // Discover parent methods by examining what functions exist in the LLVM module
        std::vector<std::string> parentMethods;
        std::string parentPrefix = parentTypeName + "_";
        
        // Iterate through all functions in the module to find parent methods
        for (auto& func : *TheModule) {
            std::string funcName = func.getName().str();
            if (funcName.find(parentPrefix) == 0) {
                // Extract method name by removing the prefix
                std::string methodName = funcName.substr(parentPrefix.length());
                parentMethods.push_back(methodName);
            }
        }
        
        cout << "Type " << typeName << " inherits from " << parentTypeName << 
                " with " << parentMethods.size() << " parent methods discovered from LLVM module" << endl;
        
        for (const std::string& parentMethodName : parentMethods) {
            bool isOverridden = false;
            for (auto* funcNode : methods) {
                if (funcNode->func_name == parentMethodName) {
                    isOverridden = true;
                    break;
                }
            }
            
            if (!isOverridden) {
                generateForwardingMethod(typeName, parentTypeName, parentMethodName, structType);
                cout << "Generated forwarding method: " << typeName << "_" << parentMethodName << 
                        " -> " << parentTypeName << "_" << parentMethodName << endl;
            }
        }
    }
    
    cout << "TypeDeclNode: Generated struct type and methods for " << typeName << endl;
    currentValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
}

void CodegenVisitor::visit(ASTNodeVector* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for ASTNodeVector with " << node->children.size() << " elements" << endl;

    for (auto* child : node->children) {
        child->accept(this, context);
    }
    
    cout << "ASTNodeVector: All elements processed" << endl;
}

void CodegenVisitor::visit(ExprsList* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for ExprsList with " << node->children.size() << " expressions" << endl;

    for (auto* expr : node->children) {
        expr->accept(this, context);
    }
    
    cout << "ExprsList: All expressions processed" << endl;
}

void CodegenVisitor::visit(ProgramNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for ProgramNode (root)" << endl;

    if (node->getNode()) {
        node->getNode()->accept(this, context);
    } else {
        cout << "Warning: ProgramNode has no child node" << endl;
    }
    
    cout << "ProgramNode: Code generation completed" << endl;
}

void CodegenVisitor::optimize() {
    cout << "Optimizing module" << endl;

    // Temporarily disable optimization to debug string concatenation
    /*
    llvm::legacy::PassManager passManager;

    passManager.add(llvm::createInstructionCombiningPass());
    passManager.add(llvm::createReassociatePass());
    passManager.add(llvm::createGVNPass());
    passManager.add(llvm::createCFGSimplificationPass());

    passManager.run(*TheModule);
    */

    cout << "Optimization complete" << endl;
}

void CodegenVisitor::generateCode() {
    cout << "Generating code" << endl;

    try {
        moduleNode->accept(this, globalContext);

        llvm::Function* mainFunc = TheModule->getFunction("main");
        if (mainFunc) {
            llvm::BasicBlock* currentBB = Builder->GetInsertBlock();
            if (currentBB && !currentBB->getTerminator()) {
                llvm::Value* returnValue = currentValue;
                if (!returnValue || !returnValue->getType()->isDoubleTy()) {
                    returnValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
                }
                Builder->CreateRet(returnValue);
                cout << "Added return statement to main function" << endl;
            }
        }

        this->optimize();
        
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        llvm::InitializeNativeTargetAsmParser();
        
        cout << "Generated LLVM IR:" << endl;
        std::string s;
        llvm::raw_string_ostream os(s);
        TheModule->print(os, nullptr);
        cout << os.str() << endl;
        
        cout << "Adding module to JIT..." << endl;
        
        if (auto Err = TheJIT.addModule(std::move(TheModule))) {
            logAllErrors(std::move(Err));
            return;
        }
        
        cout << "Module successfully added to JIT" << endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception during code generation: " << e.what() << std::endl;
        return;
    }

    TheJIT.loadStandardLibrary();
    
    cout << "Executing JIT-compiled code..." << endl;
    double result = TheJIT.executeFunction("main");
    
    if (auto printDoubleSymbol = TheJIT.lookup("print_double")) {
        void (*printDoubleFunc)(double) = (void (*)(double))(intptr_t)printDoubleSymbol->getAddress();
        cout << "Calling print_double function with result:" << endl;
        printDoubleFunc(result);
    }
}

void CodegenVisitor::visit(AccessNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    // std::cout << "Generating code for AccessNode: " << node->var_name << " accessing member" << std::endl;
    
    auto it = NamedValues.find(node->var_name);
    if (it == NamedValues.end()) {
        throw std::runtime_error("Object not found: " + node->var_name);
    }
    
    llvm::Value* objectPtr = it->second;
    
    if (auto* allocaInst = llvm::dyn_cast<llvm::AllocaInst>(objectPtr)) {
        if (allocaInst->getAllocatedType()->isPointerTy()) {
            objectPtr = Builder->CreateLoad(allocaInst->getAllocatedType(), objectPtr, 
                                          node->var_name + "_loaded");
            // std::cout << "Loaded object pointer from alloca for " << node->var_name << std::endl;
        }
    }
    
    currentObjectPtr = objectPtr;
    currentObjectName = node->var_name;
    
    node->member->accept(this, context);
    
    // std::cout << "CodeGen: AccessNode processed for " << node->var_name << std::endl;
}

void CodegenVisitor::visit(TypeAssMember* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    // std::cout << "Generating code for TypeAssMember: " << node->get_name() << std::endl;
    // std::cout << "CodeGen: TypeAssMember processed (placeholder)" << std::endl;
}

void CodegenVisitor::visit(AttributeMember* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    // std::cout << "Generating code for AttributeMember: " << node->name << std::endl;
    
    if (!currentObjectPtr) {
        throw std::runtime_error("No current object for attribute access: " + node->name);
    }
    
    std::string typeName;
    auto typeIt = objectTypes.find(currentObjectName);
    if (typeIt != objectTypes.end()) {
        typeName = typeIt->second;
    } else {
        throw std::runtime_error("Could not determine type for object: " + currentObjectName);
    }
    
    cout << "DEBUG: Looking for attribute '" << node->name << "' in type '" << typeName << "'" << endl;
    
    auto structIt = typeStructMap.find(typeName);
    auto attrsIt = typeAttributeMap.find(typeName);
    auto attrTypeIt = typeAttributeTypeMap.find(typeName);
    if (structIt == typeStructMap.end() || attrsIt == typeAttributeMap.end() || attrTypeIt == typeAttributeTypeMap.end()) {
        throw std::runtime_error("Type information not found for: " + typeName);
    }
    llvm::StructType* structType = structIt->second;
    const std::vector<std::string>& attributes = attrsIt->second;
    const AttributeTypeMap& attrTypeMap = attrTypeIt->second;
    auto it = std::find(attributes.begin(), attributes.end(), node->name);
    if (it == attributes.end()) {
        throw std::runtime_error("Attribute not found: " + node->name);
    }
    size_t index = std::distance(attributes.begin(), it);
    llvm::Type* fieldType = attrTypeMap.at(node->name);
    llvm::Value* fieldPtr = Builder->CreateStructGEP(structType, currentObjectPtr, index, "field_" + node->name);
    llvm::Value* fieldValue = Builder->CreateLoad(fieldType, fieldPtr, node->name);
    
    currentValue = fieldValue;
    
    // std::cout << "CodeGen: AttributeMember '" << node->name << "' accessed successfully" << std::endl;
}

void CodegenVisitor::visit(MethodMember* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    // std::cout << "Generating code for MethodMember: " << node->name << std::endl;
    
    if (!currentObjectPtr) {
        throw std::runtime_error("No current object for method call: " + node->name);
    }
    
    std::string typeName;
    auto typeIt = objectTypes.find(currentObjectName);
    if (typeIt != objectTypes.end()) {
        typeName = typeIt->second;
    } else {
        throw std::runtime_error("Could not determine type for object: " + currentObjectName);
    }
    
    std::string methodFuncName = typeName + "_" + node->name;
    llvm::Function* methodFunc = TheModule->getFunction(methodFuncName);
    
    if (!methodFunc) {
        throw std::runtime_error("Method function not found: " + methodFuncName);
    }
    
    // Determine the return type of the method
    llvm::Type* returnType = methodFunc->getReturnType();
    std::string returnTypeStr;
    llvm::raw_string_ostream typeStream(returnTypeStr);
    returnType->print(typeStream);
    // std::cout << "DEBUG: Method '" << node->name << "' return type: " << typeStream.str() << std::endl;
    
    std::vector<llvm::Value*> args;
    args.push_back(currentObjectPtr);
    
    if (!node->args.empty()) {
        for (ASTNode* argNode : node->args) {
            argNode->accept(this, context);
            args.push_back(currentValue);
        }
    }
    
    currentValue = Builder->CreateCall(methodFunc, args, "method_call");
    
    // Store the method's return type if needed for future type checking
    if (returnType->isPointerTy()) {
        // std::cout << "DEBUG: Method returns a pointer type (likely a String)" << std::endl;
    }
    
    // std::cout << "CodeGen: MethodMember '" << node->name << "' called successfully" << std::endl;
}

void CodegenVisitor::generateMethodFunction(const std::string& typeName, AssignFuncNode* method, 
                                           llvm::StructType* structType, const std::vector<std::string>& attributes) {
    std::string methodName = typeName + "_" + method->func_name;
    
    // Default to double, but try to infer from semantic analysis
    llvm::Type* returnType = llvm::Type::getDoubleTy(*TheContext);
    
    // Check if the method returns a string based on semantic analysis
    if (method->body && method->body->inferredType) {
        if (method->body->inferredType->name == "String") {
            returnType = llvm::Type::getInt8PtrTy(*TheContext);
        } else if (method->body->inferredType->name == "Number") {
            returnType = llvm::Type::getDoubleTy(*TheContext);
        } else if (method->body->inferredType->name == "Boolean") {
            returnType = llvm::Type::getInt1Ty(*TheContext);
        }
        // Add more type mappings as needed
    }
    
    // If this is a getter, set the return type to the field type
    if (method->func_name.find("get") == 0 && method->args->children.empty()) {
        std::string attrName = method->func_name.substr(3);
        std::transform(attrName.begin(), attrName.end(), attrName.begin(), ::tolower);
        auto attrTypeIt = typeAttributeTypeMap.find(typeName);
        if (attrTypeIt != typeAttributeTypeMap.end()) {
            const AttributeTypeMap& attrTypeMap = attrTypeIt->second;
            auto it = attrTypeMap.find(attrName);
            if (it != attrTypeMap.end()) {
                returnType = it->second;
            }
        }
    }
    std::vector<llvm::Type*> paramTypes;
    paramTypes.push_back(llvm::PointerType::get(structType, 0)); 
    
    int argCount = 0;
    if (method->args && !method->args->children.empty()) {
        argCount = method->args->children.size();
    }
    
    for (int i = 0; i < argCount; i++) {
        paramTypes.push_back(llvm::Type::getDoubleTy(*TheContext));
    }
    llvm::FunctionType* methodType = llvm::FunctionType::get(
        returnType, 
        paramTypes,
        false
    );
    
    llvm::Function* methodFunc = llvm::Function::Create(
        methodType,
        llvm::Function::ExternalLinkage,
        typeName + "_" + method->func_name,
        TheModule.get()
    );
    
    llvm::BasicBlock* methodBB = llvm::BasicBlock::Create(*TheContext, "entry", methodFunc);
    llvm::IRBuilder<> methodBuilder(*TheContext);
    methodBuilder.SetInsertPoint(methodBB);
    
    auto argIt = methodFunc->arg_begin();
    llvm::Value* selfPtr = &(*argIt++);
    selfPtr->setName("self");
    
    std::vector<std::pair<std::string, llvm::Value*>> methodParams;
    if (argCount > 0 && method->args && !method->args->children.empty()) {
        for (int i = 0; i < argCount && i < method->args->children.size(); i++) {
            llvm::Value* paramValue = &(*argIt++);
            std::string paramName = method->args->children[i]->id_name;
            paramValue->setName(paramName);
            methodParams.push_back({paramName, paramValue});
        }
    }
    if (method->body) {
        llvm::Value* prevCurrentValue = currentValue;
        std::string prevObjectName = currentObjectName;
        llvm::Value* prevObjectPtr = currentObjectPtr;
        llvm::BasicBlock* prevInsertBlock = Builder->GetInsertBlock();
        
        Builder->SetInsertPoint(methodBB);
        
        currentObjectName = "self";
        currentObjectPtr = selfPtr;
        
        std::string prevSelfType;
        auto selfTypeIt = objectTypes.find("self");
        if (selfTypeIt != objectTypes.end()) {
            prevSelfType = selfTypeIt->second;
        }
        objectTypes["self"] = typeName; 

        llvm::Value* prevSelfValue = nullptr;
        if (NamedValues.find("self") != NamedValues.end()) {
            prevSelfValue = NamedValues["self"];
        }
        NamedValues["self"] = selfPtr;
        
        std::vector<std::pair<std::string, llvm::Value*>> prevParamValues;
        for (const auto& param : methodParams) {
            if (NamedValues.find(param.first) != NamedValues.end()) {
                prevParamValues.push_back({param.first, NamedValues[param.first]});
            }
            NamedValues[param.first] = param.second;
        }
        
        method->body->accept(this, nullptr);
        
        if (currentValue) {
            Builder->CreateRet(currentValue);
        } else {
            Builder->CreateRet(llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0)));
        }
        
        if (prevSelfValue) {
            NamedValues["self"] = prevSelfValue;
        } else {
            NamedValues.erase("self");
        }
        
        if (!prevSelfType.empty()) {
            objectTypes["self"] = prevSelfType;
        } else {
            objectTypes.erase("self");
        }
        
        for (const auto& param : methodParams) {
            NamedValues.erase(param.first);
        }
        for (const auto& prevParam : prevParamValues) {
            NamedValues[prevParam.first] = prevParam.second;
        }
        
        if (prevInsertBlock) {
            Builder->SetInsertPoint(prevInsertBlock);
        }
        currentValue = prevCurrentValue;
        currentObjectName = prevObjectName;
        currentObjectPtr = prevObjectPtr;
    } else {
        if (method->func_name.find("get") == 0) {
            std::string attrName = method->func_name.substr(3);
            std::transform(attrName.begin(), attrName.end(), attrName.begin(), ::tolower);
            auto it = std::find(attributes.begin(), attributes.end(), attrName);
            if (it != attributes.end()) {
                size_t fieldIndex = std::distance(attributes.begin(), it);
                auto attrTypeIt = typeAttributeTypeMap.find(typeName);
                llvm::Type* fieldType = llvm::Type::getDoubleTy(*TheContext);
                if (attrTypeIt != typeAttributeTypeMap.end()) {
                    const AttributeTypeMap& attrTypeMap = attrTypeIt->second;
                    auto typeIt = attrTypeMap.find(attrName);
                    if (typeIt != attrTypeMap.end()) fieldType = typeIt->second;
                }
                llvm::Value* fieldPtr = methodBuilder.CreateStructGEP(structType, selfPtr, fieldIndex, "field_" + attrName);
                llvm::Value* fieldValue = methodBuilder.CreateLoad(fieldType, fieldPtr, attrName);
                methodBuilder.CreateRet(fieldValue);
            } else {
                methodBuilder.CreateRet(llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0)));
            }
        } else if (method->func_name.find("set") == 0) {
            std::string attrName = method->func_name.substr(3);
            std::transform(attrName.begin(), attrName.end(), attrName.begin(), ::tolower);
            
            auto it = std::find(attributes.begin(), attributes.end(), attrName);
            if (it != attributes.end() && !methodParams.empty()) {
                size_t index = std::distance(attributes.begin(), it);
                llvm::Value* fieldPtr = methodBuilder.CreateStructGEP(structType, selfPtr, index, "field_" + attrName);
                methodBuilder.CreateStore(methodParams[0].second, fieldPtr);
                methodBuilder.CreateRet(methodParams[0].second);
            } else {
                methodBuilder.CreateRet(llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0)));
            }
        } else {
            methodBuilder.CreateRet(llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0)));
        }
    }
    cout << "Generated method function: " << typeName + "_" + method->func_name << endl;
}

void CodegenVisitor::generateForwardingMethod(const std::string& childTypeName, const std::string& parentTypeName,
                                 const std::string& methodName, llvm::StructType* structType) {
    // Create forwarding method that calls the parent method
    std::string childMethodName = childTypeName + "_" + methodName;
    std::string parentMethodName = parentTypeName + "_" + methodName;
    
    // Get the parent method
    llvm::Function* parentMethod = TheModule->getFunction(parentMethodName);
    if (!parentMethod) {
        throw std::runtime_error("Parent method not found: " + parentMethodName);
    }
    
    // Create the child method with the same signature
    llvm::FunctionType* parentFuncType = parentMethod->getFunctionType();
    
    // First parameter type should be a pointer to the child struct
    std::vector<llvm::Type*> paramTypes;
    paramTypes.push_back(llvm::PointerType::get(structType, 0)); // Child struct pointer
    
    // Copy the rest of the parameter types from the parent function
    for (unsigned i = 1; i < parentFuncType->getNumParams(); i++) {
        paramTypes.push_back(parentFuncType->getParamType(i));
    }
    
    llvm::FunctionType* childFuncType = llvm::FunctionType::get(
        parentFuncType->getReturnType(),
        paramTypes,
        parentFuncType->isVarArg()
    );
    
    llvm::Function* childMethod = llvm::Function::Create(
        childFuncType,
        llvm::Function::ExternalLinkage,
        childMethodName,
        TheModule.get()
    );
    
    // Set parameter names
    llvm::Function::arg_iterator childArgIt = childMethod->arg_begin();
    childArgIt->setName("self");
    
    std::vector<std::string> paramNames;
    llvm::Function::arg_iterator parentArgIt = parentMethod->arg_begin();
    ++parentArgIt; // Skip self
    
    for (unsigned i = 1; i < parentFuncType->getNumParams(); i++) {
        if (parentArgIt != parentMethod->arg_end()) {
            std::string paramName = parentArgIt->getName().str();
            if (paramName.empty()) {
                paramName = "arg" + std::to_string(i);
            }
            paramNames.push_back(paramName);
            ++parentArgIt;
        }
    }
    
    childArgIt++; // Move past "self"
    for (unsigned i = 0; i < paramNames.size() && childArgIt != childMethod->arg_end(); i++, ++childArgIt) {
        childArgIt->setName(paramNames[i]);
    }
    
    // Create the function body
    llvm::BasicBlock* entryBB = llvm::BasicBlock::Create(*TheContext, "entry", childMethod);
    llvm::IRBuilder<> builder(entryBB);
    
    // Prepare arguments for parent method call
    std::vector<llvm::Value*> parentArgs;
    
    // First argument is the child object pointer (self)
    llvm::Function::arg_iterator selfArg = childMethod->arg_begin();
    parentArgs.push_back(&(*selfArg));
    
    // Add the rest of the parameters
    llvm::Function::arg_iterator paramArg = childMethod->arg_begin();
    ++paramArg;
    
    while (paramArg != childMethod->arg_end()) {
        parentArgs.push_back(&(*paramArg));
        ++paramArg;
    }
    
    // Call parent method
    llvm::Value* result = builder.CreateCall(parentMethod, parentArgs, "call_parent");
    
    // Return the result
    builder.CreateRet(result);
    
    // std::cout << "Generated forwarding method: " << childMethodName << " -> " << parentMethodName << std::endl;
}

// Moved these maps to class members in visitor.h

void CodegenVisitor::handleAssignment(BinOpNode* node, Context* context) {
    // std::cout << "Handling assignment operation: " << node->op << std::endl;
    
    // Process the right side of the assignment first
    node->right->accept(this, context);
    llvm::Value* valueToStore = currentValue;
    
    // Handle different types of assignment targets
    if (auto* accessNode = dynamic_cast<AccessNode*>(node->left)) {
        // Handle attribute assignment (e.g., obj.attr := value)
        auto it = NamedValues.find(accessNode->var_name);
        if (it == NamedValues.end()) {
            throw std::runtime_error("Object not found: " + accessNode->var_name);
        }
        
        llvm::Value* objectPtr = it->second;
        auto* attrMember = dynamic_cast<AttributeMember*>(accessNode->member);
        if (!attrMember) {
            throw std::runtime_error("Assignment target must be an attribute");
        }
        
        // Find the type and attribute information
        std::string typeName;
        auto typeIt = objectTypes.find(accessNode->var_name);
        if (typeIt != objectTypes.end()) {
            typeName = typeIt->second;
        } else {
            throw std::runtime_error("Could not determine type for object: " + accessNode->var_name);
        }
        
        auto structIt = typeStructMap.find(typeName);
        auto attrsIt = typeAttributeMap.find(typeName);
        if (structIt != typeStructMap.end() && attrsIt != typeAttributeMap.end()) {
            llvm::StructType* structType = structIt->second;
            const std::vector<std::string>& attributes = attrsIt->second;
            
            // Find the attribute index
            auto it = std::find(attributes.begin(), attributes.end(), attrMember->name);
            if (it == attributes.end()) {
                throw std::runtime_error("Attribute not found: " + attrMember->name);
            }
            size_t index = std::distance(attributes.begin(), it);
            
            // Create the GEP instruction and store the value
            llvm::Value* fieldPtr = Builder->CreateStructGEP(structType, objectPtr, index, "field_" + attrMember->name);
            Builder->CreateStore(valueToStore, fieldPtr);
        }
        
        currentValue = valueToStore;
    } else if (auto* idNode = dynamic_cast<IDNode*>(node->left)) {
        // Handle simple variable reassignment (e.g., var := value)
        auto it = NamedValues.find(idNode->id_name);
        if (it != NamedValues.end() && llvm::isa<llvm::AllocaInst>(it->second)) {
            // Check types and perform conversion if necessary
            llvm::AllocaInst* allocaInst = llvm::cast<llvm::AllocaInst>(it->second);
            llvm::Type* allocaType = allocaInst->getAllocatedType();
            
            // If the value type doesn't match the variable type, try to convert it
            if (valueToStore->getType() != allocaType) {
                if (allocaType->isDoubleTy() && valueToStore->getType()->isIntegerTy()) {
                    valueToStore = Builder->CreateSIToFP(valueToStore, allocaType, "conv_to_double");
                }
                // Add other type conversions as needed
            }
            
            // Store the value in the variable
            Builder->CreateStore(valueToStore, it->second);
            currentValue = valueToStore;
            
            // std::cout << "Variable '" << idNode->id_name << "' reassigned" << std::endl;
        } else {
            throw std::runtime_error("Variable '" + idNode->id_name + "' not found or not assignable");
        }
    } else {
        throw std::runtime_error("Unsupported assignment target");
    }
    
    // std::cout << "Assignment operation completed" << std::endl;
}

void CodegenVisitor::handleStringConcatenation(llvm::Value* leftValue, llvm::Value* rightValue, BinOpNode* node, Context* context, bool space) {
    cout << "Handling string concatenation (@) or (@@) operators" << endl;
    
    // Ensure left operand is a string pointer
    if (!leftValue->getType()->isPointerTy()) {
        throw std::runtime_error("Left operand of @ or @@ must be a string");
    }
    
    // Handle right operand - convert to string if it's a number
    llvm::Value* rightStringValue = rightValue;
    if (rightValue->getType()->isDoubleTy()) {
        // Convert number to string using double_to_string function
        llvm::Function* doubleToStringFunc = TheModule->getFunction("double_to_string");
        if (!doubleToStringFunc) {
            // Declare the double_to_string function
            llvm::FunctionType* doubleToStringType = llvm::FunctionType::get(
                llvm::Type::getInt8PtrTy(*TheContext),
                {llvm::Type::getDoubleTy(*TheContext)},
                false
            );
            doubleToStringFunc = llvm::Function::Create(
                doubleToStringType,
                llvm::Function::ExternalLinkage,
                "double_to_string",
                TheModule.get()
            );
        }
        
        rightStringValue = Builder->CreateCall(doubleToStringFunc, {rightValue}, "num_to_str");
        cout << "Converted number to string for concatenation" << endl;
    } else if (!rightValue->getType()->isPointerTy()) {
        throw std::runtime_error("Right operand of @@ must be a string or number");
    }
    
    // Declare required functions for string concatenation
    
    // hulk_strlen function
    llvm::Function* strlenFunc = TheModule->getFunction("hulk_strlen");
    if (!strlenFunc) {
        llvm::FunctionType* strlenType = llvm::FunctionType::get(
            llvm::Type::getInt64Ty(*TheContext),
            {llvm::Type::getInt8PtrTy(*TheContext)},
            false
        );
        strlenFunc = llvm::Function::Create(
            strlenType,
            llvm::Function::ExternalLinkage,
            "hulk_strlen",
            TheModule.get()
        );
    }
    
    // hulk_malloc function
    llvm::Function* mallocFunc = TheModule->getFunction("hulk_malloc");
    if (!mallocFunc) {
        llvm::FunctionType* mallocType = llvm::FunctionType::get(
            llvm::Type::getInt8PtrTy(*TheContext),
            {llvm::Type::getInt64Ty(*TheContext)},
            false
        );
        mallocFunc = llvm::Function::Create(
            mallocType,
            llvm::Function::ExternalLinkage,
            "hulk_malloc",
            TheModule.get()
        );
    }
    
    // hulk_strcpy function
    llvm::Function* strcpyFunc = TheModule->getFunction("hulk_strcpy");
    if (!strcpyFunc) {
        llvm::FunctionType* strcpyType = llvm::FunctionType::get(
            llvm::Type::getInt8PtrTy(*TheContext),
            {llvm::Type::getInt8PtrTy(*TheContext), llvm::Type::getInt8PtrTy(*TheContext)},
            false
        );
        strcpyFunc = llvm::Function::Create(
            strcpyType,
            llvm::Function::ExternalLinkage,
            "hulk_strcpy",
            TheModule.get()
        );
    }
    
    // hulk_strcat function
    llvm::Function* strcatFunc = TheModule->getFunction("hulk_strcat");
    if (!strcatFunc) {
        llvm::FunctionType* strcatType = llvm::FunctionType::get(
            llvm::Type::getInt8PtrTy(*TheContext),
            {llvm::Type::getInt8PtrTy(*TheContext), llvm::Type::getInt8PtrTy(*TheContext)},
            false
        );
        strcatFunc = llvm::Function::Create(
            strcatType,
            llvm::Function::ExternalLinkage,
            "hulk_strcat",
            TheModule.get()
        );
    }
    
    // Calculate lengths of both strings
    llvm::Value* leftLen = Builder->CreateCall(strlenFunc, {leftValue}, "left_len");
    llvm::Value* rightLen = Builder->CreateCall(strlenFunc, {rightStringValue}, "right_len");
    
    // Calculate total length needed (left + right + 1 for null terminator, +1 for space if needed)
    llvm::Value* totalLen = Builder->CreateAdd(leftLen, rightLen, "total_len");
    if (space) {
        // Add 1 for the space character
        totalLen = Builder->CreateAdd(totalLen, 
            llvm::ConstantInt::get(llvm::Type::getInt64Ty(*TheContext), 1), "total_len_with_space");
    }
    llvm::Value* totalLenPlusOne = Builder->CreateAdd(totalLen, 
        llvm::ConstantInt::get(llvm::Type::getInt64Ty(*TheContext), 1), "total_len_plus_one");
    
    // Allocate memory for the concatenated string
    llvm::Value* newString = Builder->CreateCall(mallocFunc, {totalLenPlusOne}, "new_string");
    
    // Copy the left string to the new allocated memory
    Builder->CreateCall(strcpyFunc, {newString, leftValue}, "copy_left");
    
    if (space) {
        // Create a global string constant for the space character
        llvm::GlobalVariable* spaceGlobal = Builder->CreateGlobalString(" ", "space_str");
        llvm::Value* spacePtr = Builder->CreateBitCast(spaceGlobal, llvm::Type::getInt8PtrTy(*TheContext), "space_ptr");
        Builder->CreateCall(strcatFunc, {newString, spacePtr}, "add_space");
    }
    
    // Concatenate the right string
    currentValue = Builder->CreateCall(strcatFunc, {newString, rightStringValue}, "str_concat");
    
    cout << "String concatenation completed with proper memory allocation" << endl;
}

void CodegenVisitor::visit(TypeCastNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    // std::cout << "Generating code for TypeCastNode: casting to " << node->target_type << std::endl;
    
    // Generate code for the expression being cast
    node->expr->accept(this, context);
    llvm::Value* sourceValue = currentValue;
    
    // Find the target type structure if it exists
    auto structIt = typeStructMap.find(node->target_type);
    if (structIt != typeStructMap.end()) {
        // Target is a custom type - for inheritance, this might involve pointer casting
        llvm::StructType* targetStruct = structIt->second;
        
        // If the source is also a struct pointer, we can cast it
        if (sourceValue->getType()->isPointerTy()) {
            // Cast the pointer to the target type
            llvm::Type* targetPtrType = llvm::PointerType::get(targetStruct, 0);
            currentValue = Builder->CreateBitCast(sourceValue, targetPtrType, "type_cast");
            // std::cout << "Generated pointer cast for inheritance" << std::endl;
        } else {
            throw std::runtime_error("CodeGen error: Cannot cast non-pointer value to custom type");
        }
    } else {
        // Built-in type casting (Number, String, etc.)
        if (node->target_type == "Number" || node->target_type == "number") {
            if (sourceValue->getType()->isDoubleTy()) {
                currentValue = sourceValue; // Already a number
            } else if (sourceValue->getType()->isIntegerTy()) {
                currentValue = Builder->CreateSIToFP(sourceValue, llvm::Type::getDoubleTy(*TheContext), "int_to_double");
            } else {
                throw std::runtime_error("CodeGen error: Cannot cast to Number from this type");
            }
        } else if (node->target_type == "String" || node->target_type == "string") {
            if (sourceValue->getType()->isPointerTy()) {
                currentValue = sourceValue; // Already a string pointer
            } else {
                throw std::runtime_error("CodeGen error: Cannot cast to String from this type");
            }
        } else {
            // Default case - assume the cast is valid and pass through
            currentValue = sourceValue;
            // std::cout << "Generated identity cast for type " << node->target_type << std::endl;
        }
    }
    
    // std::cout << "Type cast code generation completed" << std::endl;
}

bool CodegenVisitor::isSubtypeOf(const std::string& childType, const std::string& parentType) {
    // Direct match
    if (childType == parentType) {
        return true;
    }
    
    // Check inheritance chain
    auto it = typeInheritanceMap.find(childType);
    while (it != typeInheritanceMap.end()) {
        std::string currentParent = it->second;
        if (currentParent == parentType) {
            return true;
        }
        // Move up the inheritance chain
        it = typeInheritanceMap.find(currentParent);
    }
    
    // Everything inherits from Object (if not explicitly stated)
    if (parentType == "Object") {
        return true;
    }
    
    return false;
}

