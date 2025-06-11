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
    
    // funciones matematicas
    std::vector<std::string> mathFuncs = {"sqrt_func", "sin_func", "cos_func", "tan_func", "log_func", "exp_func"};
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
    
    std::cout << "Standard library function declarations created." << std::endl;
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
    
    std::cout << "Main function created." << std::endl;
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
    
    std::cout << "LLVM and JIT initialized. Main function created." << std::endl;
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
}

void CodegenVisitor::visit(UnaryOpNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for UnaryOp Node: " << node->op << endl;
    
    node->node->accept(this, context);
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
        
        if (dynamic_cast<StringNode*>(node->argument) != nullptr) {
            strNode = dynamic_cast<StringNode*>(node->argument);
        }
        else if (auto nodeVector = dynamic_cast<ASTNodeVector*>(node->argument)) {
            if (!nodeVector->children.empty()) {
                strNode = dynamic_cast<StringNode*>(nodeVector->children[0]);
            }
        }
        
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
            return;
        } 
        else if (dynamic_cast<IDNode*>(node->argument) != nullptr) {
            IDNode* idNode = dynamic_cast<IDNode*>(node->argument);
            
            idNode->accept(this, context);
            llvm::Value* varValue = currentValue;
            
            llvm::Function* printDoubleFunc = TheModule->getFunction("print_double");
            if (printDoubleFunc && varValue && varValue->getType()->isDoubleTy()) {
                std::vector<llvm::Value*> args = {varValue};
                currentValue = Builder->CreateCall(printDoubleFunc, args, "printcall");
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
            }
            return;
        }
        else {
            node->argument->accept(this, context);
            llvm::Value* argValue = currentValue;
            
            if (argValue->getType()->isDoubleTy()) {
                llvm::Function* printDoubleFunc = TheModule->getFunction("print_double");
                if (printDoubleFunc) {
                    std::vector<llvm::Value*> args = {argValue};
                    currentValue = Builder->CreateCall(printDoubleFunc, args, "printcall");
                    return;
                }
            } else if (argValue->getType()->isIntegerTy(1)) {
                llvm::Function* printBoolFunc = TheModule->getFunction("print_bool");
                if (printBoolFunc) {
                    std::vector<llvm::Value*> args = {argValue};
                    currentValue = Builder->CreateCall(printBoolFunc, args, "printcall");
                    return;
                }
            }
            
            std::cerr << "Warning: Printing this value type is not implemented" << std::endl;
            return;
        }
    }
    else {
        llvm::Function* calledFunc = TheModule->getFunction(node->func_name);
        if (!calledFunc) {
            throw std::runtime_error("Unknown function referenced: " + node->func_name);
        }

        std::vector<llvm::Value*> args;
        if (node->argument != nullptr) {
            node->argument->accept(this, context);
            args.push_back(currentValue);
        }

        currentValue = Builder->CreateCall(calledFunc, args, "calltmp");
    }
}

void CodegenVisitor::visit(IDNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for ID Node: " << node->id_name << endl;
    
    if (node->id_name == "pi") {
        llvm::Function* getPiFunc = TheModule->getFunction("get_pi");
        if (getPiFunc) {
            currentValue = Builder->CreateCall(getPiFunc);
        } else {
            currentValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(3.14159265359));
            std::cerr << "Warning: get_pi function not found, using constant" << std::endl;
        }
    } else if (node->id_name == "e") {
        llvm::Function* getEFunc = TheModule->getFunction("get_e");
        if (getEFunc) {
            currentValue = Builder->CreateCall(getEFunc);
        } else {
            currentValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(2.71828182846));
            std::cerr << "Warning: get_e function not found, using constant" << std::endl;
        }
    } else if (node->id_name == "alpha") {
        currentValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.5));
    } else {
        auto it = NamedValues.find(node->id_name);
        if (it != NamedValues.end()) {
            llvm::Value* varValue = it->second;
            if (llvm::isa<llvm::AllocaInst>(varValue)) {
                currentValue = Builder->CreateLoad(llvm::Type::getDoubleTy(*TheContext), varValue, node->id_name);
                cout << "Loaded value from alloca for variable: " << node->id_name << endl;
            } else {
                currentValue = varValue;
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

    cout << "Generating code for ArgsList" << endl;
}

void CodegenVisitor::visit(AssignFuncNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for AssignFuncNode: " << node->func_name << endl;
    
    node->args->accept(this, context);
    node->body->accept(this, context);
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
        
        if (auto* idNode = dynamic_cast<IDNode*>(assign->value)) {
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
        llvm::AllocaInst* alloca = tmpBuilder.CreateAlloca(llvm::Type::getDoubleTy(*TheContext), nullptr, assign->var_id->id_name);
        
        Builder->CreateStore(currentValue, alloca);
        
        NamedValues[assign->var_id->id_name] = alloca;
        
        cout << "Let variable '" << assign->var_id->id_name << "' allocated and assigned" << endl;
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

    cout << "Generating code for VarAssignType: " << node->var_name << " := new " << node->id_type_name->id_name << endl;

    std::string typeName = node->id_type_name->id_name;
    
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

    llvm::PHINode* phiNode = Builder->CreatePHI(llvm::Type::getDoubleTy(*TheContext), 2, "iftmp");
    
    if (thenValue && thenValue->getType()->isDoubleTy()) {
        phiNode->addIncoming(thenValue, thenBB);
    } else {
        llvm::Value* defaultThen = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
        phiNode->addIncoming(defaultThen, thenBB);
    }
    
    if (elseValue && elseValue->getType()->isDoubleTy()) {
        phiNode->addIncoming(elseValue, elseBB);
    } else {
        llvm::Value* defaultElse = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
        phiNode->addIncoming(defaultElse, elseBB);
    }

    currentValue = phiNode;
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
    
    if (!node->parents.empty()) {
        std::string parentTypeName = node->parents[0];
        auto parentStructIt = typeStructMap.find(parentTypeName);
        auto parentAttributesIt = typeAttributeMap.find(parentTypeName);
        
        if (parentStructIt != typeStructMap.end() && parentAttributesIt != typeAttributeMap.end()) {
            llvm::StructType* parentStruct = parentStructIt->second;
            for (unsigned i = 0; i < parentStruct->getNumElements(); ++i) {
                structFields.push_back(parentStruct->getElementType(i));
            }
            
            attributes = parentAttributesIt->second;
            
            cout << "Inherited structure from " << parentTypeName << " with " << 
                    attributes.size() << " attributes" << endl;
        } else {
            cout << "Warning: Parent type " << parentTypeName << " not found for inheritance" << endl;
        }
    }
    
    for (auto* member : node->body) {
        if (auto* varAssign = dynamic_cast<VarAssign*>(member)) {
            auto it = std::find(attributes.begin(), attributes.end(), varAssign->var_id->id_name);
            if (it == attributes.end()) {
                structFields.push_back(llvm::Type::getDoubleTy(*TheContext));
                attributes.push_back(varAssign->var_id->id_name);
            }
        } else if (auto* funcNode = dynamic_cast<AssignFuncNode*>(member)) {
            methods.push_back(funcNode);
        }
    }
    
    llvm::StructType* structType = llvm::StructType::create(*TheContext, structFields, typeName);
    
    typeStructMap[typeName] = structType;
    typeAttributeMap[typeName] = attributes;
    
    cout << "DEBUG: Stored attributes for type " << typeName << ": ";
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
    
    static int objectCounter = 0;
    std::string globalObjName = "global_" + typeName + "_" + std::to_string(objectCounter++);
    
    llvm::GlobalVariable* globalObj = new llvm::GlobalVariable(
        *TheModule,
        structType,
        false,                                     
        llvm::GlobalValue::InternalLinkage,        
        llvm::Constant::getNullValue(structType),
        globalObjName
    );
    
    for (auto* member : node->body) {
        if (auto* varAssign = dynamic_cast<VarAssign*>(member)) {
            auto it = std::find(attributes.begin(), attributes.end(), varAssign->var_id->id_name);
            if (it != attributes.end()) {
                size_t fieldIndex = std::distance(attributes.begin(), it);
                llvm::Value* fieldPtr = constructorBuilder.CreateStructGEP(structType, globalObj, fieldIndex, "field_" + varAssign->var_id->id_name);
                
                llvm::Value* defaultVal = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
                if (auto* floatNode = dynamic_cast<FloatNode*>(varAssign->value)) {
                    defaultVal = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat((double)floatNode->value));
                }
                
                constructorBuilder.CreateStore(defaultVal, fieldPtr);
            }
        }
    }
    
    constructorBuilder.CreateRet(globalObj);
    
    for (auto* funcNode : methods) {
        generateMethodFunction(typeName, funcNode, structType, attributes);
    }
    
    if (!node->parents.empty()) {
        std::string parentTypeName = node->parents[0];
        
        std::vector<std::string> parentMethods;
        if (parentTypeName == "Point") {
            parentMethods = {"getX", "setX", "setY"};
        }
        
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

    llvm::legacy::PassManager passManager;

    passManager.add(llvm::createInstructionCombiningPass());
    passManager.add(llvm::createReassociatePass());
    passManager.add(llvm::createGVNPass());
    passManager.add(llvm::createCFGSimplificationPass());

    passManager.run(*TheModule);

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
    
    std::cout << "Generating code for AccessNode: " << node->var_name << " accessing member" << std::endl;
    
    auto it = NamedValues.find(node->var_name);
    if (it == NamedValues.end()) {
        throw std::runtime_error("Object not found: " + node->var_name);
    }
    
    llvm::Value* objectPtr = it->second;
    
    if (auto* allocaInst = llvm::dyn_cast<llvm::AllocaInst>(objectPtr)) {
        if (allocaInst->getAllocatedType()->isPointerTy()) {
            objectPtr = Builder->CreateLoad(allocaInst->getAllocatedType(), objectPtr, 
                                          node->var_name + "_loaded");
            std::cout << "Loaded object pointer from alloca for " << node->var_name << std::endl;
        }
    }
    
    currentObjectPtr = objectPtr;
    currentObjectName = node->var_name;
    
    node->member->accept(this, context);
    
    std::cout << "CodeGen: AccessNode processed for " << node->var_name << std::endl;
}

void CodegenVisitor::visit(TypeAssMember* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    std::cout << "Generating code for TypeAssMember: " << node->get_name() << std::endl;
    std::cout << "CodeGen: TypeAssMember processed (placeholder)" << std::endl;
}

void CodegenVisitor::visit(AttributeMember* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    std::cout << "Generating code for AttributeMember: " << node->name << std::endl;
    
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
    
    if (structIt == typeStructMap.end() || attrsIt == typeAttributeMap.end()) {
        throw std::runtime_error("Type information not found for: " + typeName);
    }
    
    llvm::StructType* structType = structIt->second;
    const std::vector<std::string>& attributes = attrsIt->second;
    
    auto it = std::find(attributes.begin(), attributes.end(), node->name);
    if (it == attributes.end()) {
        throw std::runtime_error("Attribute not found: " + node->name);
    }
    
    size_t index = std::distance(attributes.begin(), it);
    
    llvm::Value* fieldPtr = Builder->CreateStructGEP(structType, currentObjectPtr, index, "field_" + node->name);
    llvm::Value* fieldValue = Builder->CreateLoad(llvm::Type::getDoubleTy(*TheContext), fieldPtr, node->name);
    
    currentValue = fieldValue;
    
    std::cout << "CodeGen: AttributeMember '" << node->name << "' accessed successfully" << std::endl;
}

void CodegenVisitor::visit(MethodMember* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    std::cout << "Generating code for MethodMember: " << node->name << std::endl;
    
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
    
    std::vector<llvm::Value*> args;
    args.push_back(currentObjectPtr);
    
    if (!node->args.empty()) {
        for (ASTNode* argNode : node->args) {
            argNode->accept(this, context);
            args.push_back(currentValue);
        }
    }
    
    currentValue = Builder->CreateCall(methodFunc, args, "method_call");
    
    std::cout << "CodeGen: MethodMember '" << node->name << "' called successfully" << std::endl;
}

void CodegenVisitor::generateMethodFunction(const std::string& typeName, AssignFuncNode* method, 
                                           llvm::StructType* structType, const std::vector<std::string>& attributes) {
    std::string methodName = typeName + "_" + method->func_name;
    
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
        llvm::Type::getDoubleTy(*TheContext), 
        paramTypes,
        false
    );
    
    llvm::Function* methodFunc = llvm::Function::Create(
        methodType,
        llvm::Function::ExternalLinkage,
        methodName,
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
                size_t index = std::distance(attributes.begin(), it);
                llvm::Value* fieldPtr = methodBuilder.CreateStructGEP(structType, selfPtr, index, "field_" + attrName);
                llvm::Value* fieldValue = methodBuilder.CreateLoad(llvm::Type::getDoubleTy(*TheContext), fieldPtr, attrName);
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
    
    cout << "Generated method function: " << methodName << endl;
}

void CodegenVisitor::generateForwardingMethod(const std::string& childTypeName, const std::string& parentTypeName,
                                              const std::string& methodName, llvm::StructType* structType) {
    std::string childMethodName = childTypeName + "_" + methodName;
    std::string parentMethodName = parentTypeName + "_" + methodName;
    
    llvm::Function* parentMethod = TheModule->getFunction(parentMethodName);
    if (!parentMethod) {
        cout << "Warning: Parent method " << parentMethodName << " not found for forwarding" << endl;
        return;
    }
    
    llvm::FunctionType* forwardingType = parentMethod->getFunctionType();
    
    llvm::Function* forwardingFunc = llvm::Function::Create(
        forwardingType,
        llvm::Function::ExternalLinkage,
        childMethodName,
        TheModule.get()
    );
    
    llvm::BasicBlock* forwardingBB = llvm::BasicBlock::Create(*TheContext, "entry", forwardingFunc);
    llvm::IRBuilder<> forwardingBuilder(*TheContext);
    forwardingBuilder.SetInsertPoint(forwardingBB);
    
    std::vector<llvm::Value*> args;
    for (auto& arg : forwardingFunc->args()) {
        args.push_back(&arg);
    }
    
    llvm::Value* result = forwardingBuilder.CreateCall(parentMethod, args, "parent_call");
    forwardingBuilder.CreateRet(result);
    
    cout << "Generated forwarding method: " << childMethodName << " -> " << parentMethodName << endl;
}

static std::map<std::string, llvm::StructType*> typeStructMap;
static std::map<std::string, std::vector<std::string>> typeAttributeMap;
static std::map<std::string, llvm::Value*> objectInstances;

void CodegenVisitor::handleAssignment(BinOpNode* node, Context* context) {
    std::cout << "Handling assignment operation: " << node->op << std::endl;
    
    if (auto* accessNode = dynamic_cast<AccessNode*>(node->left)) {
        auto it = NamedValues.find(accessNode->var_name);
        if (it == NamedValues.end()) {
            throw std::runtime_error("Object not found: " + accessNode->var_name);
        }
        
        llvm::Value* objectPtr = it->second;
        auto* attrMember = dynamic_cast<AttributeMember*>(accessNode->member);
        if (!attrMember) {
            throw std::runtime_error("Assignment target must be an attribute");
        }
        
        node->right->accept(this, context);
        llvm::Value* valueToStore = currentValue;
        
        size_t index = (attrMember->name == "x") ? 0 : 1;
        
        std::string typeName = "Point";
        auto structIt = typeStructMap.find(typeName);
        if (structIt != typeStructMap.end()) {
            llvm::StructType* structType = structIt->second;
            llvm::Value* fieldPtr = Builder->CreateStructGEP(structType, objectPtr, index, "field_ptr");
            Builder->CreateStore(valueToStore, fieldPtr);
        }
        
        currentValue = valueToStore;
    } else {
        throw std::runtime_error("Unsupported assignment target");
    }
    
    std::cout << "Assignment operation completed" << std::endl;
}

