#include "visitor.h"
#include "../Ast/ast.hpp"
#include "context.h"
#include <iostream>
#include <typeinfo>

using namespace std;

void SemanticCheckerVisitor::visit(ASTNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
        return;
    }

    node->accept(this, context);
}

void SemanticCheckerVisitor::visit(FloatNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
        return;
    }

    // cout << "Visiting Float Node: " << node->value << endl;
}

void SemanticCheckerVisitor::visit(BoolNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
        return;
    }
    
    // cout << "Visiting Bool Node: " << node->value << endl;
    node->semanticValue = node->value ? "true" : "false";
}

void SemanticCheckerVisitor::visit(StringNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
        return;
    }
    
    // cout << "Visiting String Node: " << node->value << endl;
    node->semanticValue = "\"" + node->value + "\"";
}

void SemanticCheckerVisitor::visit(UnaryOpNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
        return;
    }

    // cout << "Visiting UnaryOp Node: " << node->op << endl;
    
    node->node->accept(this, context);
    
    if (node->op == "!") {
        if (node->node->inferredType && !context->canAssign(node->node->inferredType, Context::boolType)) {
            addError("Type error in line " + std::to_string(node->line) + " Boolean negation requires boolean operand, got " + 
                    node->node->inferredType->name);
            return;
        }
        node->inferredType = Context::boolType;
        node->semanticValue = "bool_negation(" + node->node->semanticValue + ")";
    } else if (node->op == "-") {
        if (node->node->inferredType && !context->canAssign(node->node->inferredType, Context::numberType)) {
            addError("Type error in line " + std::to_string(node->line) + ": Arithmetic unary operation requires numeric operand, got " + 
                    node->node->inferredType->name);
            return;
        }
        node->inferredType = Context::numberType;
        node->semanticValue = "arithmetic_unary(" + node->op + ", " + node->node->semanticValue + ")";
    } else {
        addError("Semantic error in line " + std::to_string(node->line) + ": Unknown unary operator: " + node->op);
        return;
    }
}

void SemanticCheckerVisitor::visit(BinOpNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
        return;
    }

    // cout << "Visiting BinOp Node: " << node->op << endl;
    
    // Handle 'is' operator specially since the right operand is a type name, not a variable
    if (node->op == "is") {
        // Type checking operator: returns true if left operand is of type specified by right operand
        // Left operand should be an expression, right operand should be a type name (IDNode)
        
        // Visit left operand normally
        node->left->accept(this, context);
        
        if (!node->left->inferredType) {
            addError("Semantic error in line " + std::to_string(node->line) + ": Cannot determine type of left operand in 'is' expression");
            return;
        }
        
        // Right operand should be a type name (IDNode) - don't visit it as a variable
        auto* rightIdNode = dynamic_cast<IDNode*>(node->right);
        if (!rightIdNode) {
            addError("Semantic error in line " + std::to_string(node->line) + " : Right operand of 'is' must be a type name");
            return;
        }
        
        // Check if the right operand is a valid type
        auto targetType = context->getType(rightIdNode->id_name);
        if (!targetType) {
            addError("Semantic error in line " + std::to_string(node->line) + " : Type '" + rightIdNode->id_name + "' not found in 'is' expression");
            return;
        }
        
        // Set the type for the right operand manually (it's a type reference)
        rightIdNode->semanticValue = "type_ref(" + rightIdNode->id_name + ")";
        
        // The 'is' operator always returns a boolean
        node->semanticValue = "type_check(" + node->left->semanticValue + " is " + rightIdNode->id_name + ")";
        
        // cout << "Semantic check: 'is' operator - checking if " << 
                // (node->left->inferredType ? node->left->inferredType->name : "unknown") << 
                // " is " << rightIdNode->id_name << endl;
        return;
    }
    
    // For all other operators, visit both operands normally
    node->left->accept(this, context);
    node->right->accept(this, context);
    
    if (node->op == "+" || node->op == "-" || node->op == "*" || node->op == "/" || node->op == "%" || node->op == "^") {
        checkTypeCompatibility(Context::numberType, node->left->inferredType, "left operand of " + node->op, node->line);
        checkTypeCompatibility(Context::numberType, node->right->inferredType, "right operand of " + node->op, node->line);
        
        node->semanticValue = "arithmetic_op(" + node->left->semanticValue + ", " + node->op + ", " + node->right->semanticValue + ")";
        
    } else if (node->op == "==" || node->op == "!=" || node->op == "<" || node->op == ">" || node->op == "<=" || node->op == ">=") {
        checkTypeCompatibility(node->left->inferredType, node->right->inferredType, "comparison operands", node->line);
        if (!node->left->inferredType || !node->right->inferredType) {
            addError("Type error in line " + std::to_string(node->line) + " : Cannot compare untyped expressions");
            return;
        }
        node->semanticValue = "comparison_op(" + node->left->semanticValue + ", " + node->op + ", " + node->right->semanticValue + ")";
    } else if (node->op == "&" || node->op == "|") {
        checkTypeCompatibility(Context::boolType, node->left->inferredType, "left operand of " + node->op, node->line);
        checkTypeCompatibility(Context::boolType, node->right->inferredType, "right operand of " + node->op, node->line);
        
        node->semanticValue = "logical_op(" + node->left->semanticValue + ", " + node->op + ", " + node->right->semanticValue + ")";
        
    } else if (node->op == "@") {
        // String concatenation operator
        // Left operand should be a string, right operand can be string or number
        if (node->left->inferredType && !context->canAssign(node->left->inferredType, Context::stringType)) {
            addError("Type error in line " + std::to_string(node->line) + ": Left operand of @ must be a string, got " + 
                                   node->left->inferredType->name);
            return;
        }
        if (node->right->inferredType && 
            !context->canAssign(node->right->inferredType, Context::stringType) &&
            !context->canAssign(node->right->inferredType, Context::numberType)) {
            addError("Type error in line " + std::to_string(node->line) + " : Right operand of @ must be a string or number, got " + 
                                   node->right->inferredType->name);
            return;
        }
        node->semanticValue = "string_concat(" + node->left->semanticValue + ", " + node->right->semanticValue + ")";
    } else if (node->op == "@@") {
        // String concatenation operator
        // Left operand should be a string, right operand can be string or number
        if (node->left->inferredType && !context->canAssign(node->left->inferredType, Context::stringType)) {
            addError("Type error in line " + std::to_string(node->line) + " : Left operand of @@ must be a string, got " + 
                                   node->left->inferredType->name);
            return;
        }
        if (node->right->inferredType && 
            !context->canAssign(node->right->inferredType, Context::stringType) &&
            !context->canAssign(node->right->inferredType, Context::numberType)) {
            addError("Type error in line " + std::to_string(node->line) + ": Right operand of @@ must be a string or number, got " + 
                                   node->right->inferredType->name);
            return;
        }
        node->semanticValue = "string_double_concat(" + node->left->semanticValue + ", " + node->right->semanticValue + ")";
    } else if (node->op == "=" || node->op == ":=") {
        if (node->left->inferredType && node->right->inferredType) {
            if (!context->canAssign(node->right->inferredType, node->left->inferredType)) {
                addError("Type error in line " + std::to_string(node->line) + " : Cannot assign " + node->right->inferredType->name + 
                                       " to " + node->left->inferredType->name);
                return;
            }
        }
        node->semanticValue = "assignment(" + node->left->semanticValue + ", " + node->right->semanticValue + ")";
        
    } else {
        addError("Semantic error in line " + std::to_string(node->line) + " : Unknown binary operator: " + node->op);
        return;
    }
}

void SemanticCheckerVisitor::visit(FunctionCallNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
        return;
    }

    // cout << "Visiting FunctionCallNode: " << node->func_name << endl;

    // Handle built-in functions with special semantics
    if (node->func_name == "print") {
        node->argument->accept(this, context);
        node->semanticValue = "print_call(" + node->argument->semanticValue + ")";
        // cout << "Built-in function 'print' called" << endl;
        return;
    } else if (node->func_name == "sin") {
        node->argument->accept(this, context);
        if (node->argument->inferredType && !context->canAssign(node->argument->inferredType, Context::numberType)) {
            addError("Type error in line " + std::to_string(node->line) + " : Argument for 'sin' must be a number, got " + 
                                   node->argument->inferredType->name);
            return;
        }
        node->semanticValue = "sin_call(" + node->argument->semanticValue + ")";
        node->inferredType = Context::numberType; // sin returns a number
        return;
    } else if (node->func_name == "cos") {
        node->argument->accept(this, context);
        if (node->argument->inferredType && !context->canAssign(node->argument->inferredType, Context::numberType)) {
            addError("Type error in line " + std::to_string(node->line) + " : Argument for 'cos' must be a number, got " + 
                                   node->argument->inferredType->name);
            return;
        }
        node->semanticValue = "cos_call(" + node->argument->semanticValue + ")";
        node->inferredType = Context::numberType; // cos returns a number
        return;
    } else if (node->func_name == "sqrt") {
        node->argument->accept(this, context);
        if (node->argument->inferredType && !context->canAssign(node->argument->inferredType, Context::numberType)) {
            addError("Type error in line " + std::to_string(node->line) + " : Argument for 'sqrt' must be a number, got " + 
                                   node->argument->inferredType->name);
            return;
        }
        node->semanticValue = "sqrt_call(" + node->argument->semanticValue + ")";
        node->inferredType = Context::numberType; // sqrt returns a number
        return;
    } else if (node->func_name == "exp") {
        node->argument->accept(this, context);
        if (node->argument->inferredType && !context->canAssign(node->argument->inferredType, Context::numberType)) {
            addError("Type error in line " + std::to_string(node->line) + " : Argument for 'exp' must be a number, got " + 
                                   node->argument->inferredType->name);
            return;
        }
        node->semanticValue = "exp_call(" + node->argument->semanticValue + ")";
        node->inferredType = Context::numberType; // exp returns a number
        return;
    } else if (node->func_name == "log") {
        node->argument->accept(this, context);
        if (node->argument->inferredType && !context->canAssign(node->argument->inferredType, Context::numberType)) {
            addError("Type error in line " + std::to_string(node->line) + " : Argument for 'log' must be a number, got " + 
                                   node->argument->inferredType->name);
            return;
        }
        node->semanticValue = "log_call(" + node->argument->semanticValue + ")";
        node->inferredType = Context::numberType; // log returns a number
        return;
    } else if (node->func_name == "rand") {
        node->argument->accept(this, context);
        // Check if rand has arguments (it shouldn't)
        if (node->argument) {
            // Cast to ASTNodeVector to check if it's an empty argument list
            if (auto* nodeVector = dynamic_cast<ASTNodeVector*>(node->argument)) {
                if (!nodeVector->children.empty()) {
                    addError("Semantic error in line " + std::to_string(node->line) + " : 'rand' doesn't accept arguments");
                    return;
                }
            } else {
            // Single argument case
            addError("Semantic error in line " + std::to_string(node->line) + " : 'rand' doesn't accept arguments");
            return;
            }
        }
        node->semanticValue = "rand_call()";
        node->inferredType = Context::numberType; // rand returns a number
        return;
    }


    // Visit and type-check function arguments
    node->argument->accept(this, context);
    
    std::vector<std::shared_ptr<TypeInfo>> argTypes;
    
    // Handle different argument structures
    if (auto* nodeVector = dynamic_cast<ASTNodeVector*>(node->argument)) {
        // Multiple arguments case
        for (auto* arg : nodeVector->children) {
            if (arg->inferredType) {
                argTypes.push_back(arg->inferredType);
            } else {
                // Fallback to number type for untyped arguments
                argTypes.push_back(Context::numberType);
                // cout << "Warning: Argument has no inferred type, assuming number" << endl;
            }
        }
    } else {
        // Single argument case
        if (node->argument->inferredType) {
            argTypes.push_back(node->argument->inferredType);
        } else {
            argTypes.push_back(Context::numberType);
            // cout << "Warning: Single argument has no inferred type, assuming number" << endl;
        }
    }

    // First, try to resolve as a global function with exact parameter type matching
    auto returnType = context->getFuncReturnType(node->func_name, argTypes);
    bool isMethodCall = false;
    
    // If not found as global function and we're inside a type, check if it's a method of the current type
    if (!returnType && context->currentType) {
        cout << "Function '" << node->func_name << "' not found as global function, checking current type methods" << endl;
        
        // Check if the function is a method of the current type
        auto collectedMethods = context->getMethodsForType(context->currentType->name, true);
        
        for (const auto& method : collectedMethods) {
            if (method.name == node->func_name) {
                cout << "Found method '" << node->func_name << "' in type '" << context->currentType->name << "'" << endl;
                
                // Check if parameter types match
                if (method.paramTypes.size() == argTypes.size()) {
                    bool typesMatch = true;
                    for (size_t i = 0; i < argTypes.size(); i++) {
                        if (!context->canAssign(argTypes[i], method.paramTypes[i])) {
                            typesMatch = false;
                            break;
                        }
                    }
                    
                    if (typesMatch) {
                        returnType = method.returnType;
                        isMethodCall = true;
                        cout << "Method call '" << node->func_name << "' resolved with return type: " 
                             << (returnType ? returnType->name : "void") << endl;
                        
                        // Generate error: methods should be called with self. prefix
                        addError("Semantic error in line " + std::to_string(node->line) + 
                                " : Method '" + node->func_name + "' cannot be called directly. " +
                                "Use 'self." + node->func_name + "()' instead.");
                        break;
                    }
                }
            }
        }
    }
    
    if (!returnType) {
        // Try with type coercion for numeric types for global functions
        std::vector<std::shared_ptr<TypeInfo>> coercedTypes;
        bool canCoerce = true;
        
        for (auto& argType : argTypes) {
            if (argType->name == "Number" || argType->name == "number" || 
                context->canAssign(argType, Context::numberType)) {
                coercedTypes.push_back(Context::numberType);
            } else {
                coercedTypes.push_back(argType);
            }
        }
        
        if (coercedTypes != argTypes) {
            returnType = context->getFuncReturnType(node->func_name, coercedTypes);
            if (returnType) {
                // cout << "Function call resolved with type coercion" << endl;
            }
        }
    }
    
    if (!returnType) {
        // Final fallback: check if function exists with any parameter count
        int argCount = argTypes.size();
        if (!context->isDefined(node->func_name, argCount)) {
            // Provide more helpful error message if it's a method that should use self.
            if (context->currentType) {
                auto collectedMethods = context->getMethodsForType(context->currentType->name, true);
                bool isMethodWithDifferentArity = false;
                
                for (const auto& method : collectedMethods) {
                    if (method.name == node->func_name) {
                        isMethodWithDifferentArity = true;
                        addError("Semantic error in line " + std::to_string(node->line) + 
                                " : Method '" + node->func_name + "' exists in type '" + 
                                context->currentType->name + "' but with different parameter count. " +
                                "Expected " + to_string(method.paramTypes.size()) + " parameter(s), got " + 
                                to_string(argCount) + ". Use 'self." + node->func_name + "()' to call the method.");
                        return;
                    }
                }
                
                if (!isMethodWithDifferentArity) {
                    addError("Semantic error in line " + std::to_string(node->line) + 
                            " : Function '" + node->func_name + "' with " + to_string(argCount) + 
                            " parameter(s) not found as global function or method of type '" + 
                            context->currentType->name + "'.");
                }
            } else {
                addError("Semantic error in line " + std::to_string(node->line) + 
                        " : Function '" + node->func_name + "' with " + to_string(argCount) + 
                        " parameter(s) not found. Available overloads don't match the provided argument types.");
            }
            return;
        }
        
        // Function exists but types don't match exactly - this should be an error, not a fallback
        // Let's get the function info to provide better error message
        FuncInfo funcInfo;
        context->getLocal(node->func_name, argCount, funcInfo);
        
        string expectedTypes = "";
        for (size_t i = 0; i < funcInfo.paramTypes.size(); i++) {
            if (i > 0) expectedTypes += ", ";
            expectedTypes += funcInfo.paramTypes[i]->name;
        }
        
        string actualTypes = "";
        for (size_t i = 0; i < argTypes.size(); i++) {
            if (i > 0) actualTypes += ", ";
            actualTypes += argTypes[i]->name;
        }
        
        addError("Semantic error in line " + std::to_string(node->line) + " : Function '" + node->func_name + 
                               "' expects parameter types (" + expectedTypes + 
                               ") but got (" + actualTypes + ")");
        return;
    }
    
    // Build semantic value with proper argument representation
    string argsValue = "";
    if (argTypes.size() == 1) {
        argsValue = node->argument->semanticValue;
    } else {
        argsValue = "args(" + to_string(argTypes.size()) + ")";
    }
    
    // Set the inferred type for the function call
    node->inferredType = returnType;
    
    // Build semantic value indicating whether it's a method call or function call
    if (isMethodCall) {
        node->semanticValue = "method_call(" + node->func_name + ", " + argsValue + ")";
    } else {
        node->semanticValue = "func_call(" + node->func_name + ", " + argsValue + ")";
    }
    
    // cout << "Semantic check: Function '" << node->func_name << "' with " << 
            // argTypes.size() << " argument(s) returns " << returnType->name << endl;
}

void SemanticCheckerVisitor::visit(IDNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
        return;
    }


    if (node->id_name == "PI" || node->id_name == "E") {
        // Handle built-in constants
        node->semanticValue = node->id_name;
        node->inferredType = Context::numberType;
        // cout << "Visiting IDNode: Built-in constant " << node->id_name << endl;
        return;
    }

    // cout << "Visiting IDNode: " << node->id_name << endl;
    
    // Check if we're in a type member context (inside a method)
    if (context->currentType) {
        cout << "inside type: " << context->currentType->name << endl;
        // If we're inside a type, we need to check if the ID is a member variable or method
        // First check if it's a local variable/parameter in the current function scope
        auto localVarType = context->getVarType(node->id_name);
        bool isLocalVariable = false;
        
        // Check if it's defined in the immediate function context (not inherited from parent contexts)
        if (localVarType && context->isLocal(node->id_name)) {
            isLocalVariable = true;
            cout << "IDNode '" << node->id_name << "' is a local variable in function context" << endl;
            node->inferredType = localVarType;
            node->semanticValue = "var_ref(" + node->id_name + ")";
            return;
        }
        
        // If it's not a local variable, check if it's a member variable of the current type
        if (!isLocalVariable) {
            cout << "IDNode '" << node->id_name << "' is not a local variable, checking type members" << endl;
            
            // Check if it's a member variable using the Context's collected data
            auto collectedAttrs = context->getAttributesForType(context->currentType->name, true);
            bool isMemberVariable = false;
            
            for (const auto& attr : collectedAttrs) {
                cout << attr.name << " is an attribute of type " << context->currentType->name << endl;
                if (attr.name == node->id_name) {
                    cout << "IDNode '" << node->id_name << "' is an attribute of type '" << context->currentType->name << "'" << endl;
                    node->semanticValue = "attr_ref(" + context->currentType->name + ", " + node->id_name + ")";
                    node->inferredType = attr.type;
                    isMemberVariable = true;
                    addError("Semantic error in line " + std::to_string(node->line) + " : Member variable '" + node->id_name + 
                                               "' cannot be accessed directly in method. Use 'self." + 
                                               node->id_name + "' instead.");
                    break;
                }
            }
            
            // Check if it's a method name used without 'self.'
            if (!isMemberVariable) {
                auto collectedMethods = context->getMethodsForType(context->currentType->name, true);
                for (const auto& method : collectedMethods) {
                    if (method.name == node->id_name) {
                        cout << "IDNode '" << node->id_name << "' is a method of type '" << context->currentType->name << "'" << endl;
                        addError("Semantic error in line " + std::to_string(node->line) + " : Method '" + node->id_name + 
                                               "' cannot be called directly in method. Use 'self." + 
                                               node->id_name + "()' instead.");
                        break;
                    }
                }
            }
        }
    }
    
    if (!context->isDefined(node->id_name)) {
        addError("Semantic error in line " + std::to_string(node->line) + " : Variable '" + node->id_name + "' is not defined");
        return;
    }
    
    // Get the variable type and set the inferred type
    auto varType = context->getVarType(node->id_name);
    if (varType) {
        node->inferredType = varType;
    }
    
    node->semanticValue = "var_ref(" + node->id_name + ")";
    
    // cout << "Semantic check: Variable '" << node->id_name << "' has type " << varType->name << endl;
}

void SemanticCheckerVisitor::visit(BlockNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
		return;
	}

	// cout << "Visiting BlockNode" << endl;
	
	Context* blockContext = context->createChildContext();
	
	for (auto child : node->children) {
		child->accept(this, blockContext);
	}
	
	// cout << "Semantic check: Block processed with " << node->children.size() << " statements" << endl;
}

void SemanticCheckerVisitor::visit(ArgsList* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
		return;
	}

	// cout << "Visiting ArgsList with " << node->children.size() << " arguments" << endl;

	for (size_t i = 0; i < node->children.size(); i++) {
		for (size_t j = i + 1; j < node->children.size(); j++) {
			if (node->children[i]->id_name == node->children[j]->id_name) {
				addError("Semantic error in line " + std::to_string(node->line) + " : Duplicate parameter name '" + node->children[i]->id_name + "'");
				return;
			}
		}
	}

	for (auto child : node->children) {
		// cout << "Processing argument: " << child->id_name << endl;
        child->accept(this, context);
	}
	
	// cout << "Semantic check: All argument names are unique" << endl;
}

void SemanticCheckerVisitor::visit(AssignFuncNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
        return;
    }



    Context* functionContext = context->createChildContext();
    
    // If we're inside a type, this is a method - define 'self'
    if (context->currentType) {
        // Create a TypeInfo for the current type and define 'self'
        auto selfType = std::make_shared<TypeInfo>(context->currentType->name, TypeKind::CLASS);
        selfType->typeDef = context->currentType->typeDef;
        functionContext->defineVar("self", selfType);
        
        // cout << "Method '" << node->func_name << "' in type '" << context->currentType->name << "' - 'self' defined" << endl;
    }
    
    // Process function parameters in the function context
    for (auto* arg : node->args->children) {
        // Define the parameter in the function context FIRST
        std::shared_ptr<TypeInfo> paramType = Context::numberType; // Default
        if (!arg->id_type.empty() && arg->id_type != "none") {
            auto explicitType = context->getType(arg->id_type);
            if (explicitType) {
                paramType = explicitType;
            }
        }
        functionContext->defineVar(arg->id_name, paramType);
        
        // Then process the argument for validation
        arg->accept(this, functionContext);
    }
    
    // Process function body in the function context
    node->body->accept(this, functionContext);
    
    // cout << "Function '" << node->func_name << "' defined successfully:" << endl;
    // cout << "  - Parameters: " << node->args->children.size() << endl;
    // cout << "  - Body type: " << (node->body->inferredType ? node->body->inferredType->name : "Expression") << endl;
}

void SemanticCheckerVisitor::visit(LetAssign* node, Context* context) {
    if (node == nullptr) {
        addError("Node is null");
        return;
    }

    // cout << "Visiting LetAssign with " << node->assigns.size() << " assignments" << endl;

    Context* letContext = context->createChildContext();

    for (auto* assign : node->assigns) {        
        assign->accept(this, letContext);        
        assign->var_id->semanticValue = "var_def(" + assign->var_id->id_name + ", " + assign->value->semanticValue + ")";
    }

    // cout << "Entering LetAssign body" << endl;
    node->body->accept(this, letContext);
    
    node->semanticValue = "let_expr(bindings, " + node->body->semanticValue + ")";
    
    // cout << "Let expression type: " << (node->inferredType ? node->inferredType->name : "Expression") << endl;
}

void SemanticCheckerVisitor::visit(VarAssign* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
        return;
    }

    // cout << "Visiting VarAssign: " << node->var_id->id_name << " static type " << node->var_id->id_type << endl;

    node->value->accept(this, context);    
    
    // cout << "Semantic check: Variable assignment '" << node->var_id->id_name << "' with type " << 
            // (node->inferredType ? node->inferredType->name : "unknown") << endl;
}

void SemanticCheckerVisitor::visit(NewTypeNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
		return;
    }

    // Visit constructor arguments
    for (auto* expr : node->expr_list) {
        expr->accept(this, context);
    }

    node->semanticValue = "new_instance(" + node->id_type_name + ", args:" + to_string(node->expr_list.size()) + ")";
    
    // cout << "Semantic check: Object instantiation of type '" << node->id_type_name << "' is valid" << endl;
}

void SemanticCheckerVisitor::visit(VarAssignList* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
		return;
	}

	// cout << "Visiting VarAssignList with " << node->assigns.size() << " assignments" << endl;

	for (size_t i = 0; i < node->assigns.size(); i++) {
		for (size_t j = i + 1; j < node->assigns.size(); j++) {
			if (node->assigns[i]->var_id == node->assigns[j]->var_id) {
				addError("Semantic error in line " + std::to_string(node->line) + " : Duplicate variable assignment '" + node->assigns[i]->var_id->id_name + "' in the same list");
			}
		}
	}

	for (auto* assign : node->assigns) {
		assign->accept(this, context);
	}
	
	// cout << "Semantic check: All variable assignments in list are unique and valid" << endl;
}

void SemanticCheckerVisitor::visit(VarAssignType* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
		return;
    }

    // cout << "Visiting VarAssignType: " << node->var_name << " := new " << node->new_type->id_type_name << endl;

    node->body->accept(this, context);
    
    // cout << "Semantic check: Type instantiation for '" << node->var_name << "' of type '" << instanceType->name << "' is valid" << endl;
}

void SemanticCheckerVisitor::visit(Conditional* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
		return;
	}

	// cout << "Visiting Conditional (if-else)" << endl;

	node->bool_expr->accept(this, context);

	Context* ifContext = context->createChildContext();
	Context* elseContext = context->createChildContext();

	node->if_body->accept(this, ifContext);

	node->else_body->accept(this, elseContext);
	
	// cout << "Semantic check: Conditional statement processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(BoolExprNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
		return;
	}

	// cout << "Visiting BoolExprNode" << endl;

	node->expr->accept(this, context);
	
	// BoolExprNode should always have boolean type regardless of inner expression
	node->semanticValue = "bool_expr(" + node->expr->semanticValue + ")";
	
	// cout << "Semantic check: Boolean expression processed with type: " << 
	        // (node->inferredType ? node->inferredType->name : "unknown") << endl;
}

void SemanticCheckerVisitor::visit(WhileNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
		return;
	}

	// cout << "Visiting WhileNode" << endl;

	node->bool_expr->accept(this, context);

	Context* whileContext = context->createChildContext();

	node->body->accept(this, whileContext);
	
	// cout << "Semantic check: While loop processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(VarDesAssign* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
		return;
	}

	// cout << "Visiting VarDesAssign: " << node->id->id_name << endl;

	
	// Type check the assigned value
	node->value->accept(this, context);
	
	
	node->semanticValue = "var_reassign(" + node->id->id_name + ", " + node->value->semanticValue + ")";
	
	// cout << "Semantic check: Variable '" << node->id->id_name << "' reassigned with compatible type " << 
			// valueType->name << " -> " << varType->name << endl;
}

void SemanticCheckerVisitor::visit(ForNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
		return;
	}

	// cout << "DEBUG: Visiting ForNode with iterator: " << node->id->id_name << endl;

	node->group->accept(this, context);

	Context* forContext = context->createChildContext();

	if (!forContext->define(node->id->id_name)) {
        addError("Semantic error in line: " + std::to_string(node->line) + " : Failed to define iterator variable '" + node->id->id_name + "'");
	}

	node->body->accept(this, forContext);
	
	// cout << "Semantic check: For loop processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(TypeDeclNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
		return;
    }

    // cout << "DEBUG: Visiting TypeDeclNode: " << node->id->id_name << endl;

    // Get the type info instead of type def for currentType
    auto typeInfo = context->getType(node->id->id_name);
    context->currentType = typeInfo;

    node->args->accept(this, context);

    for (auto* element : node->body) {
        element->accept(this, context);
    }

    context->currentType = nullptr;

    // node->semanticValue = "type_def(" + node->id->id_name + 
    //                      ", properties:" + to_string(typeDef->properties.size()) +
    //                      ", methods:" + to_string(typeDef->methods.size()) + ")";
    
    // cout << "Type '" << node->id->id_name << "' defined with " << 
            // typeDef->properties.size() << " properties and " <<
            // typeDef->methods.size() << " methods" << endl;
}

void SemanticCheckerVisitor::visit(ASTNodeVector* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
		return;
	}

	// cout << "Visiting ASTNodeVector with " << node->children.size() << " elements" << endl;

	for (auto* child : node->children) {
		child->accept(this, context);
	}
	
	// cout << "Semantic check: ASTNodeVector processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(ExprsList* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
		return;
	}

	// cout << "Visiting ExprsList with " << node->children.size() << " expressions" << endl;

	for (auto* expr : node->children) {
		expr->accept(this, context);
	}
	
	// cout << "Semantic check: ExprsList processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(ProgramNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
		return;
	}

	// cout << "Visiting ProgramNode (root)" << endl;

	if (node->getNode()) {
		node->getNode()->accept(this, context);
	} else {
		// cout << "Warning: ProgramNode has no child node" << endl;
	}
	
	// cout << "Semantic check: Program processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(AccessNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
		return;
    }
    
    // std::cout << "Visiting AccessNode: " << node->var_name << " accessing member" << std::endl;

    // Check if the object being accessed exists and get its type
    if (!context->isDefined(node->var_name)) {
        addError("Semantic error in line " + std::to_string(node->line) + " : Variable '" + node->var_name + "' is not defined");
        return;
    }
    
    auto objectType = context->getVarType(node->var_name);
    if (!objectType) {
        addError("Semantic error in line " + std::to_string(node->line) + " : Cannot determine type of '" + node->var_name + "'");
        return;
    }
    
    // Get the member name being accessed
    std::string memberName = node->member->get_name();
    std::string objectTypeName = objectType->name;
    
    // Special handling for 'self' access in methods
    if (node->var_name == "self") {
        if (!context->currentType) {
            addError("Semantic error in line " + std::to_string(node->line) + " : 'self' can only be used inside type methods");
            return;
        }
        objectTypeName = context->currentType->name;
    }
    
    // Validate that the member exists in the object's type
    bool memberFound = false;
    
    // Check if it's an attribute
    auto collectedAttributes = context->getAttributesForType(objectTypeName, true);
    for (const auto& attr : collectedAttributes) {
        if (attr.name == memberName) {
            memberFound = true;
            // Set the inferred type for the access based on the attribute type
            node->inferredType = attr.type;
            break;
        }
    }

    // If not found as attribute, check if it's a method
    if (!memberFound) {
        auto collectedMethods = context->getMethodsForType(objectTypeName, true);
        for (const auto& method : collectedMethods) {
            if (method.name == memberName) {
                memberFound = true;
                // For method access, the type would be the method's return type
                node->inferredType = method.returnType;
                break;
            }
        }
    }
    
    if (!memberFound) {
        addError("Semantic error in line " + std::to_string(node->line) + " : Member '" + memberName + 
                "' not found in type '" + objectTypeName + "'");
        return;
    }
    
    // Process the member access
    node->member->accept(this, context);
      
    // std::cout << "Member access resolved to type: " << 
                // (node->inferredType ? node->inferredType->name : "unknown") << std::endl;
}

void SemanticCheckerVisitor::visit(TypeAssMember* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
		return;
    }
    // std::cout << "Visiting TypeAssMember: " << node->get_name() << std::endl;
    // std::cout << "Semantic check: TypeAssMember processed" << std::endl;
}

void SemanticCheckerVisitor::visit(AttributeMember* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
		return;
    }
    // std::cout << "Visiting AttributeMember: " << node->name << std::endl;
    
    if (!node->inferredType) {
        node->inferredType = std::make_shared<TypeInfo>("unknown", TypeKind::INFERRED);
    }
    node->semanticValue = "attribute(" + node->name + ")";
    
    // std::cout << "AttributeMember '" << node->name << "' processed" << std::endl;
}

void SemanticCheckerVisitor::visit(MethodMember* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
		return;
    }
    // std::cout << "Visiting MethodMember: " << node->name << std::endl;
	
    for (size_t i = 0; i < node->args.size(); i++) {
        node->args[i]->accept(this, context);
    }
    
    // std::cout << "MethodMember '" << node->name << "' with " << argTypes.size() << " arguments" << std::endl;
}

void SemanticCheckerVisitor::visit(TypeCastNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
		return;
    }
    
    // std::cout << "Visiting TypeCastNode: casting to " << node->target_type << std::endl;
    
    // Visit the expression being cast
    node->expr->accept(this, context);
    
    
    node->semanticValue = "type_cast(" + node->expr->semanticValue + ", " + node->target_type + ")";
    
    // std::cout << "Type cast from '" << sourceType->name << "' to '" << targetType->name << "' is valid" << std::endl;
}

// Type inference helper methods
std::shared_ptr<TypeInfo> SemanticCheckerVisitor::inferReturnType(ASTNode* body, Context* functionContext) {
    if (!body || !body->inferredType) {
        return Context::voidType;
    }
    
    // Handle different types of function bodies
    if (auto* blockNode = dynamic_cast<BlockNode*>(body)) {
        return inferBlockReturnType(blockNode, functionContext);
    } else if (auto* boolExprNode = dynamic_cast<BoolExprNode*>(body)) {
        return Context::boolType;
    } else if (auto* binOpNode = dynamic_cast<BinOpNode*>(body)) {
        return inferBinaryOperationType(binOpNode->op, 
                                      binOpNode->left->inferredType, 
                                      binOpNode->right->inferredType);
    } else if (auto* conditional = dynamic_cast<Conditional*>(body)) {
        return inferConditionalReturnType(conditional, functionContext);
    } else if (auto* letAssign = dynamic_cast<LetAssign*>(body)) {
        // For let expressions, return type is the type of the body
        return letAssign->body ? letAssign->body->inferredType : Context::voidType;
    } else if (auto* whileNode = dynamic_cast<WhileNode*>(body)) {
        // While loops typically return void
        return Context::voidType;
    } else if (auto* funcCall = dynamic_cast<FunctionCallNode*>(body)) {
        // Function calls return their inferred type
        return funcCall->inferredType ? funcCall->inferredType : Context::voidType;
    } else {
        // For simple expressions, use the inferred type
        return body->inferredType;
    }
}

std::shared_ptr<TypeInfo> SemanticCheckerVisitor::inferBlockReturnType(BlockNode* block, Context* functionContext) {
    if (!block || block->children.empty()) {
        return Context::voidType;
    }
    
    // The return type of a block is the type of its last expression
    ASTNode* lastExpr = block->children.back();
    if (lastExpr && lastExpr->inferredType) {
        // Recursively infer the type of the last expression
        return inferReturnType(lastExpr, functionContext);
    }
    
    return Context::voidType;
}

std::shared_ptr<TypeInfo> SemanticCheckerVisitor::inferConditionalReturnType(Conditional* cond, Context* functionContext) {
    if (!cond) return Context::voidType;
    
    auto ifType = cond->if_body ? inferReturnType(cond->if_body, functionContext) : Context::voidType;
    auto elseType = cond->else_body ? inferReturnType(cond->else_body, functionContext) : Context::voidType;
    
    // Try to find a common supertype
    if (ifType && elseType) {
        auto commonType = globalContext->findCommonSupertype(ifType, elseType);
        if (commonType && commonType->name != "Expression") {
            return commonType;
        }
    }
    
    // If both branches have the same type, use it
    if (ifType && elseType && ifType->name == elseType->name) {
        return ifType;
    }
    
    // If one branch is void, use the other (for partial conditionals)
    if (ifType && ifType->name != "Expression" && ifType->name != "unknown") return ifType;
    if (elseType && elseType->name != "Expression" && elseType->name != "unknown") return elseType;
    
    return Context::voidType;
}