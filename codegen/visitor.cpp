#include "visitor.h"
#include "../Ast/ast.hpp"
#include "jit.h"
#include "errors.h"
#include <iostream>
#include <map>

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

// Global or member variables for LLVM context
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

// Create standard library function declarations
void CodegenVisitor::createStandardLibraryDeclarations() {
    // Declare print_double function: void print_double(double)
    llvm::FunctionType* printDoubleFuncType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(*TheContext),                        // Return type: void
        {llvm::Type::getDoubleTy(*TheContext)},                    // Parameter type: double
        false                                                       // Not vararg
    );
    llvm::Function::Create(
        printDoubleFuncType,
        llvm::Function::ExternalLinkage,
        "print_double",
        TheModule.get()
    );
    
    // Declare print_bool function: void print_bool(bool)
    llvm::FunctionType* printBoolFuncType = llvm::FunctionType::get(
        llvm::Type::getVoidTy(*TheContext),                        // Return type: void
        {llvm::Type::getInt1Ty(*TheContext)},                      // Parameter type: bool (i1)
        false                                                       // Not vararg
    );
    llvm::Function::Create(
        printBoolFuncType,
        llvm::Function::ExternalLinkage,
        "print_bool",
        TheModule.get()
    );
    
    // Declare get_pi function: double get_pi()
    llvm::FunctionType* getPiFuncType = llvm::FunctionType::get(
        llvm::Type::getDoubleTy(*TheContext),                      // Return type: double
        false                                                       // Not vararg
    );
    llvm::Function::Create(
        getPiFuncType,
        llvm::Function::ExternalLinkage,
        "get_pi",
        TheModule.get()
    );
    
    // Declare get_e function: double get_e()
    llvm::FunctionType* getEFuncType = llvm::FunctionType::get(
        llvm::Type::getDoubleTy(*TheContext),                      // Return type: double
        false                                                       // Not vararg
    );
    llvm::Function::Create(
        getEFuncType,
        llvm::Function::ExternalLinkage,
        "get_e",
        TheModule.get()
    );
    
    // Declare mathematical functions
    std::vector<std::string> mathFuncs = {"sqrt_func", "sin_func", "cos_func", "tan_func", "log_func", "exp_func"};
    for (const auto& funcName : mathFuncs) {
        llvm::FunctionType* mathFuncType = llvm::FunctionType::get(
            llvm::Type::getDoubleTy(*TheContext),                  // Return type: double
            {llvm::Type::getDoubleTy(*TheContext)},                // Parameter type: double
            false                                                   // Not vararg
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

// Create the main function
llvm::Function* CodegenVisitor::createMainFunction() {
    // Define main function signature: double main()
    llvm::FunctionType* mainFuncType = llvm::FunctionType::get(
        llvm::Type::getDoubleTy(*TheContext),                      // Return type: double
        false                                                       // Not vararg
    );
    
    // Create the main function
    llvm::Function* mainFunc = llvm::Function::Create(
        mainFuncType,
        llvm::Function::ExternalLinkage,
        "main",
        TheModule.get()
    );
    
    // Create a basic block in the function
    llvm::BasicBlock* BB = llvm::BasicBlock::Create(*TheContext, "entry", mainFunc);
    Builder->SetInsertPoint(BB);
    
    std::cout << "Main function created." << std::endl;
    return mainFunc;
}

// Initialize LLVM and create a main function
void CodegenVisitor::initialize() {
    // Initialize LLVM targets
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    
    // Create standard library function declarations
    createStandardLibraryDeclarations();
    
    // Create main function
    llvm::Function* mainFunc = createMainFunction();
    
    // Print initialization info
    std::cout << "LLVM and JIT initialized. Main function created." << std::endl;
}

void CodegenVisitor::visit(ASTNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    // Use dynamic_cast to determine the actual type and call the appropriate visitor
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
    } else {
        throw std::runtime_error("CodegenVisitor::visit(ASTNode*): Unhandled node type: " + std::string(typeid(*node).name()));
    }
}

void CodegenVisitor::visit(FloatNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for Float Node: " << node->value << endl;

    // Use double type consistently
    currentValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat((double)node->value));
}

void CodegenVisitor::visit(BoolNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    cout << "Generating code for Bool Node: " << node->value << endl;
    
    // Create an appropriate LLVM value for a boolean
    currentValue = llvm::ConstantInt::get(llvm::Type::getInt1Ty(*TheContext), node->value ? 1 : 0);
}

void CodegenVisitor::visit(StringNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    cout << "Generating code for String Node: " << node->value << endl;
    
    // String handling will be done in the function call processing
    // Just keep a reference to the string node for now
    // The actual string allocation will happen when needed (in print function)
    // We don't set currentValue here because strings are handled differently depending on context
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

    // For the left value
    L->print(rso);
    std::string leftStr = rso.str();
    str.clear();  // Clear the string for reuse

    // For the right value
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
        // For power operation, we'll use the llvm.pow.f64 intrinsic for double precision
        llvm::Function* powFunc = llvm::Intrinsic::getDeclaration(
            TheModule.get(),
            llvm::Intrinsic::pow,
            {llvm::Type::getDoubleTy(*TheContext)}
        );
        
        // Ensure operands are of the right type (double)
        if (!L->getType()->isDoubleTy()) {
            L = Builder->CreateFPCast(L, llvm::Type::getDoubleTy(*TheContext), "castL");
        }
        
        if (!R->getType()->isDoubleTy()) {
            R = Builder->CreateFPCast(R, llvm::Type::getDoubleTy(*TheContext), "castR");
        }
        
        std::vector<llvm::Value*> args = {L, R};
        currentValue = Builder->CreateCall(powFunc, args, "powtmp");
    } else if (node->op == ">") {
        // Greater than comparison
        currentValue = Builder->CreateFCmpOGT(L, R, "cmptmp");
    } else if (node->op == "<") {
        // Less than comparison
        currentValue = Builder->CreateFCmpOLT(L, R, "cmptmp");
    } else if (node->op == ">=") {
        // Greater than or equal comparison
        currentValue = Builder->CreateFCmpOGE(L, R, "cmptmp");
    } else if (node->op == "<=") {
        // Less than or equal comparison
        currentValue = Builder->CreateFCmpOLE(L, R, "cmptmp");
    } else if (node->op == "==") {
        // Equal comparison
        currentValue = Builder->CreateFCmpOEQ(L, R, "cmptmp");
    } else if (node->op == "!=") {
        // Not equal comparison
        currentValue = Builder->CreateFCmpONE(L, R, "cmptmp");
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

    // Special handling for the print function
    if (node->func_name == "print") {
        // Make sure there's an argument
        if (node->argument == nullptr) {
            throw std::runtime_error("Print function requires an argument");
        }
        
        // Handle string literals
        if (dynamic_cast<StringNode*>(node->argument) != nullptr) {
            // String literals need to be created as global constant arrays
            StringNode* strNode = dynamic_cast<StringNode*>(node->argument);
            
            // Remove quotes from the string value if present
            std::string cleanStr = strNode->value;
            if (cleanStr.length() >= 2 && cleanStr.front() == '"' && cleanStr.back() == '"') {
                cleanStr = cleanStr.substr(1, cleanStr.length() - 2);
            }
            
            // Create a global string variable with the string content
            llvm::Constant* strConstant = llvm::ConstantDataArray::getString(*TheContext, cleanStr);
            
            // Create a global variable to store the string
            llvm::GlobalVariable* globalStr = new llvm::GlobalVariable(
                *TheModule,
                strConstant->getType(),
                true,                                // isConstant
                llvm::GlobalValue::PrivateLinkage,
                strConstant,
                ".str"                               // Name
            );
            
            // Get a pointer to the beginning of the string
            llvm::Value* zero = llvm::ConstantInt::get(*TheContext, llvm::APInt(32, 0, true));
            std::vector<llvm::Value*> indices = {zero, zero};
            llvm::Value* strPtr = Builder->CreateInBoundsGEP(globalStr->getValueType(), globalStr, indices, "strptr");
            
            // Call the print function with the string pointer
            std::vector<llvm::Value*> args = {strPtr};
            currentValue = Builder->CreateCall(PrintFunc, args, "printcall");
            return;
        } 
        else if (dynamic_cast<IDNode*>(node->argument) != nullptr) {
            // Handle variables - evaluate them and print their numeric values
            IDNode* idNode = dynamic_cast<IDNode*>(node->argument);
            
            // Visit the ID node to get its value
            idNode->accept(this, context);
            llvm::Value* varValue = currentValue;
            
            // Use print_double to print the numeric value
            llvm::Function* printDoubleFunc = TheModule->getFunction("print_double");
            if (printDoubleFunc && varValue && varValue->getType()->isDoubleTy()) {
                std::vector<llvm::Value*> args = {varValue};
                currentValue = Builder->CreateCall(printDoubleFunc, args, "printcall");
            } else {
                // Fallback: print the variable name as string
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
            // For other types, visit the argument and use appropriate print function
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
        // Standard function call handling for other functions
        llvm::Function* calledFunc = TheModule->getFunction(node->func_name);
        if (!calledFunc) {
            throw std::runtime_error("Unknown function referenced: " + node->func_name);
        }

        // Generate code for the argument (if any)
        std::vector<llvm::Value*> args;
        if (node->argument != nullptr) {
            node->argument->accept(this, context);
            args.push_back(currentValue);
        }

        // Create the function call instruction
        currentValue = Builder->CreateCall(calledFunc, args, "calltmp");
    }
}

void CodegenVisitor::visit(IDNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for ID Node: " << node->id_name << endl;
    
    // Handle special constants using external functions
    if (node->id_name == "pi") {
        // Call get_pi() function
        llvm::Function* getPiFunc = TheModule->getFunction("get_pi");
        if (getPiFunc) {
            currentValue = Builder->CreateCall(getPiFunc);
        } else {
            currentValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(3.14159265359));
            std::cerr << "Warning: get_pi function not found, using constant" << std::endl;
        }
    } else if (node->id_name == "e") {
        // Call get_e() function
        llvm::Function* getEFunc = TheModule->getFunction("get_e");
        if (getEFunc) {
            currentValue = Builder->CreateCall(getEFunc);
        } else {
            currentValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(2.71828182846));
            std::cerr << "Warning: get_e function not found, using constant" << std::endl;
        }
    } else if (node->id_name == "alpha") {
        // Use a predefined constant for alpha
        currentValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.5));
    } else {
        // Check if variable exists in named values (for user-defined variables)
        auto it = NamedValues.find(node->id_name);
        if (it != NamedValues.end()) {
            llvm::Value* varValue = it->second;
            // If it's an alloca, we need to load the value
            if (llvm::isa<llvm::AllocaInst>(varValue)) {
                currentValue = Builder->CreateLoad(llvm::Type::getDoubleTy(*TheContext), varValue, node->id_name);
                cout << "Loaded value from alloca for variable: " << node->id_name << endl;
            } else {
                // It's a direct value
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
    
    // Don't automatically add return statements in blocks
    // Let the caller decide when to add returns
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

    // Create a new scope for the let variables
    std::map<std::string, llvm::Value*> prevValues;
    
    // Process all variable assignments
    for (auto* assign : node->assigns) {
        // Save previous value if it exists
        auto it = NamedValues.find(assign->var_name);
        if (it != NamedValues.end()) {
            prevValues[assign->var_name] = it->second;
        }
        
        // Visit the assignment value to generate its code
        assign->value->accept(this, context);
        
        // Create an alloca for this variable so it can be reassigned later
        llvm::Function* function = Builder->GetInsertBlock()->getParent();
        llvm::IRBuilder<> tmpBuilder(&function->getEntryBlock(), function->getEntryBlock().begin());
        llvm::AllocaInst* alloca = tmpBuilder.CreateAlloca(llvm::Type::getDoubleTy(*TheContext), nullptr, assign->var_name);
        
        // Store the initial value
        Builder->CreateStore(currentValue, alloca);
        
        // Store the alloca in the named values map so we can load/store to it
        NamedValues[assign->var_name] = alloca;
        
        cout << "Let variable '" << assign->var_name << "' allocated and assigned" << endl;
    }

    // Visit the body with the new variables in scope
    node->body->accept(this, context);

    // Restore previous values (clean up scope)
    for (auto* assign : node->assigns) {
        auto prevIt = prevValues.find(assign->var_name);
        if (prevIt != prevValues.end()) {
            NamedValues[assign->var_name] = prevIt->second;
        } else {
            NamedValues.erase(assign->var_name);
        }
    }
}

void CodegenVisitor::visit(VarAssign* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for VarAssign: " << node->var_name << endl;

    // Visit the value expression to generate its code
    node->value->accept(this, context);
    
    // The currentValue now contains the result of evaluating the value expression
    // In a full implementation, we might want to create an alloca and store instruction
    // For now, we'll just keep track of the value in our NamedValues map
    
    cout << "Variable '" << node->var_name << "' assigned value" << endl;
}

void CodegenVisitor::visit(VarAssignList* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for VarAssignList" << endl;

    // Visit all variable assignments
    for (auto* assign : node->assigns) {
        assign->accept(this, context);
    }
}

void CodegenVisitor::visit(Conditional* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for Conditional" << endl;

    // Visit the boolean expression to get the condition
    node->bool_expr->accept(this, context);
    llvm::Value* condValue = currentValue;

    // Get the current function to create basic blocks
    llvm::Function* function = Builder->GetInsertBlock()->getParent();

    // Create basic blocks for then, else, and merge
    llvm::BasicBlock* thenBB = llvm::BasicBlock::Create(*TheContext, "then", function);
    llvm::BasicBlock* elseBB = llvm::BasicBlock::Create(*TheContext, "else");
    llvm::BasicBlock* mergeBB = llvm::BasicBlock::Create(*TheContext, "ifcont");

    // Create the conditional branch
    Builder->CreateCondBr(condValue, thenBB, elseBB);

    // Generate code for the then branch
    Builder->SetInsertPoint(thenBB);
    node->if_body->accept(this, context);
    llvm::Value* thenValue = currentValue;
    
    // Ensure the then block has a branch to merge (in case it doesn't have a terminator)
    if (!Builder->GetInsertBlock()->getTerminator()) {
        Builder->CreateBr(mergeBB);
    }
    thenBB = Builder->GetInsertBlock(); // Update thenBB in case the body changed the current block

    // Generate code for the else branch
    function->getBasicBlockList().push_back(elseBB);
    Builder->SetInsertPoint(elseBB);
    node->else_body->accept(this, context);
    llvm::Value* elseValue = currentValue;
    
    // Ensure the else block has a branch to merge (in case it doesn't have a terminator)
    if (!Builder->GetInsertBlock()->getTerminator()) {
        Builder->CreateBr(mergeBB);
    }
    elseBB = Builder->GetInsertBlock(); // Update elseBB in case the body changed the current block

    // Generate the merge block
    function->getBasicBlockList().push_back(mergeBB);
    Builder->SetInsertPoint(mergeBB);

    // Create a PHI node to merge the values from both branches
    llvm::PHINode* phiNode = Builder->CreatePHI(llvm::Type::getDoubleTy(*TheContext), 2, "iftmp");
    
    // Ensure both values are of the correct type
    if (thenValue && thenValue->getType()->isDoubleTy()) {
        phiNode->addIncoming(thenValue, thenBB);
    } else {
        // Default value if thenValue is not a double
        llvm::Value* defaultThen = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
        phiNode->addIncoming(defaultThen, thenBB);
    }
    
    if (elseValue && elseValue->getType()->isDoubleTy()) {
        phiNode->addIncoming(elseValue, elseBB);
    } else {
        // Default value if elseValue is not a double
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

    // Visit the inner expression (which should be a comparison)
    node->expr->accept(this, context);
    
    // The currentValue should now contain the result of the comparison
    if (!currentValue) {
        throw std::runtime_error("BoolExprNode: Expression evaluation failed");
    }
    
    cout << "BoolExprNode: Expression evaluated, checking type..." << endl;
    
    // Check the type and handle accordingly
    if (currentValue->getType()->isIntegerTy(1)) {
        // Already a boolean (i1), no conversion needed
        cout << "BoolExprNode: Expression is already boolean type" << endl;
    } else if (currentValue->getType()->isDoubleTy()) {
        // Convert double to boolean (non-zero is true)
        llvm::Value* zero = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
        currentValue = Builder->CreateFCmpONE(currentValue, zero, "booltmp");
        cout << "BoolExprNode: Converted double to boolean" << endl;
    } else {
        // Get type information for debugging
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

    // Get the current function
    llvm::Function* function = Builder->GetInsertBlock()->getParent();
    if (!function) {
        throw std::runtime_error("WhileNode: No current function found");
    }
    
    // Create basic blocks for proper while loop structure
    llvm::BasicBlock* condBB = llvm::BasicBlock::Create(*TheContext, "while.cond", function);
    llvm::BasicBlock* bodyBB = llvm::BasicBlock::Create(*TheContext, "while.body", function);
    llvm::BasicBlock* endBB = llvm::BasicBlock::Create(*TheContext, "while.end", function);
    
    // Jump to condition check
    Builder->CreateBr(condBB);
    
    // Generate condition evaluation
    Builder->SetInsertPoint(condBB);
    // Always reload variables from allocas in the condition to ensure we get the latest values
    node->bool_expr->accept(this, context);
    llvm::Value* condValue = currentValue;
    
    if (!condValue) {
        throw std::runtime_error("WhileNode: Condition evaluation failed");
    }
    
    // The condition should already be boolean (i1), don't convert
    // Branch based on condition
    Builder->CreateCondBr(condValue, bodyBB, endBB);
    
    // Generate body
    Builder->SetInsertPoint(bodyBB);
    node->body->accept(this, context);
    // After body execution, jump back to condition check
    Builder->CreateBr(condBB);
    
    // Continue with end
    Builder->SetInsertPoint(endBB);
    currentValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
    
    cout << "WhileNode: Successfully generated (simplified version)" << endl;
}

void CodegenVisitor::visit(VarDesAssign* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for VarDesAssign: " << node->id->id_name << endl;

    // Visit the value expression
    node->value->accept(this, context);
    llvm::Value* assignValue = currentValue;

    // Look up the variable in named values
    auto it = NamedValues.find(node->id->id_name);
    if (it != NamedValues.end() && llvm::isa<llvm::AllocaInst>(it->second)) {
        // Store the new value in the allocated variable
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
    
    // Create a simple loop structure (simplified)
    llvm::Function* function = Builder->GetInsertBlock()->getParent();
    llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(*TheContext, "for.loop", function);
    llvm::BasicBlock* endBB = llvm::BasicBlock::Create(*TheContext, "for.end", function);
    
    // Jump to loop
    Builder->CreateBr(loopBB);
    
    // Generate loop body
    Builder->SetInsertPoint(loopBB);
    node->body->accept(this, context);
    
    // For simplicity, just exit the loop (a real implementation would handle iteration)
    Builder->CreateBr(endBB);
    
    // Continue after loop
    Builder->SetInsertPoint(endBB);
    currentValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
    
    cout << "ForNode: Basic structure generated" << endl;
}

void CodegenVisitor::visit(TypeDeclNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for TypeDeclNode: " << node->id->id_name << endl;
    
    // Type declarations are typically handled at compile time for type checking
    // For code generation, we might create constructor functions or struct types
    // This is a simplified implementation
    
    cout << "TypeDeclNode: Type declarations not fully implemented in codegen" << endl;
    currentValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
}

void CodegenVisitor::visit(ASTNodeVector* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Generating code for ASTNodeVector with " << node->children.size() << " elements" << endl;

    // Visit all children
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

    // Visit all expressions
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

    // Visit the main program node using the public accessor
    if (node->getNode()) {
        node->getNode()->accept(this, context);
    } else {
        cout << "Warning: ProgramNode has no child node" << endl;
    }
    
    cout << "ProgramNode: Code generation completed" << endl;
}

void CodegenVisitor::optimize() {
    cout << "Optimizing module" << endl;

    // Run optimization passes on the module
    // The actual optimization passes would be more complex and numerous
    llvm::legacy::PassManager passManager;

    // Add a simple scalar optimization pass
    passManager.add(llvm::createInstructionCombiningPass());
    passManager.add(llvm::createReassociatePass());
    passManager.add(llvm::createGVNPass());
    passManager.add(llvm::createCFGSimplificationPass());

    // Run the passes over the module
    passManager.run(*TheModule);

    cout << "Optimization complete" << endl;
}

void CodegenVisitor::generateCode() {
    cout << "Generating code" << endl;

    try {
        // Generate the LLVM IR code
        moduleNode->accept(this, globalContext);

        // Ensure the main function has a proper return statement
        llvm::Function* mainFunc = TheModule->getFunction("main");
        if (mainFunc) {
            llvm::BasicBlock* currentBB = Builder->GetInsertBlock();
            if (currentBB && !currentBB->getTerminator()) {
                // Add a return statement with the current value or 0.0
                llvm::Value* returnValue = currentValue;
                if (!returnValue || !returnValue->getType()->isDoubleTy()) {
                    returnValue = llvm::ConstantFP::get(llvm::Type::getDoubleTy(*TheContext), llvm::APFloat(0.0));
                }
                Builder->CreateRet(returnValue);
                cout << "Added return statement to main function" << endl;
            }
        }

        // Optimize the generated code
        this->optimize();
        
        // Initialize LLVM targets for JIT
        llvm::InitializeNativeTarget();
        llvm::InitializeNativeTargetAsmPrinter();
        llvm::InitializeNativeTargetAsmParser();
        
        // Print the generated IR
        cout << "Generated LLVM IR:" << endl;
        std::string s;
        llvm::raw_string_ostream os(s);
        TheModule->print(os, nullptr);
        cout << os.str() << endl;
        
        cout << "Adding module to JIT..." << endl;
        
        // Add the module to the JIT compiler
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

    // Load the standard library functions
    TheJIT.loadStandardLibrary();
    
    // Execute the main function using our execute helper
    cout << "Executing JIT-compiled code..." << endl;
    double result = TheJIT.executeFunction("main");
    
    // Print the result properly
    // cout << "Result: " << result << endl;
    
    // We can also use the print_double function to show the result
    if (auto printDoubleSymbol = TheJIT.lookup("print_double")) {
        void (*printDoubleFunc)(double) = (void (*)(double))(intptr_t)printDoubleSymbol->getAddress();
        cout << "Calling print_double function with result:" << endl;
        printDoubleFunc(result);
    }
}

