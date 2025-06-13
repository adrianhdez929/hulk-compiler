#include "visitor.h"
#include "../Ast/ast.hpp"
#include "context.h"
#include <iostream>
#include <typeinfo>

using namespace std;

void SemanticCheckerVisitor::visit(ASTNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    node->accept(this, context);
}

void SemanticCheckerVisitor::visit(FloatNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting Float Node: " << node->value << endl;
    if (!node->inferredType) {
        node->inferredType = Context::numberType; 
    }
    node->semanticValue = "numeric_literal";
}

void SemanticCheckerVisitor::visit(BoolNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    cout << "Visiting Bool Node: " << node->value << endl;
    if (!node->inferredType) {
        node->inferredType = Context::boolType;
    }
    node->semanticValue = node->value ? "true" : "false";
}

void SemanticCheckerVisitor::visit(StringNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    cout << "Visiting String Node: " << node->value << endl;
    if (!node->inferredType) {
        node->inferredType = Context::stringType;
    }
    node->semanticValue = "\"" + node->value + "\"";
}

void SemanticCheckerVisitor::visit(UnaryOpNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting UnaryOp Node: " << node->op << endl;
    
    node->node->accept(this, context);
    
    if (node->op == "!") {
        if (node->node->inferredType && !context->canAssign(node->node->inferredType, Context::boolType)) {
            throw std::runtime_error("Type error: Boolean negation requires boolean operand, got " + 
                                   node->node->inferredType->name);
        }
        node->inferredType = Context::boolType;
        node->semanticValue = "bool_negation(" + node->node->semanticValue + ")";
    } else if (node->op == "-") {
        if (node->node->inferredType && !context->canAssign(node->node->inferredType, Context::numberType)) {
            throw std::runtime_error("Type error: Arithmetic unary operation requires numeric operand, got " + 
                                   node->node->inferredType->name);
        }
        node->inferredType = Context::numberType;
        node->semanticValue = "arithmetic_unary(" + node->op + ", " + node->node->semanticValue + ")";
    } else {
        throw std::runtime_error("Semantic error: Unknown unary operator: " + node->op);
    }
}

void SemanticCheckerVisitor::visit(BinOpNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting BinOp Node: " << node->op << endl;
    
    // Handle 'is' operator specially since the right operand is a type name, not a variable
    if (node->op == "is") {
        // Type checking operator: returns true if left operand is of type specified by right operand
        // Left operand should be an expression, right operand should be a type name (IDNode)
        
        // Visit left operand normally
        node->left->accept(this, context);
        
        if (!node->left->inferredType) {
            throw std::runtime_error("Semantic error: Cannot determine type of left operand in 'is' expression");
        }
        
        // Right operand should be a type name (IDNode) - don't visit it as a variable
        auto* rightIdNode = dynamic_cast<IDNode*>(node->right);
        if (!rightIdNode) {
            throw std::runtime_error("Semantic error: Right operand of 'is' must be a type name");
        }
        
        // Check if the right operand is a valid type
        auto targetType = context->getType(rightIdNode->id_name);
        if (!targetType) {
            throw std::runtime_error("Semantic error: Type '" + rightIdNode->id_name + "' not found in 'is' expression");
        }
        
        // Set the type for the right operand manually (it's a type reference)
        rightIdNode->inferredType = targetType;
        rightIdNode->semanticValue = "type_ref(" + rightIdNode->id_name + ")";
        
        // The 'is' operator always returns a boolean
        node->inferredType = Context::boolType;
        node->semanticValue = "type_check(" + node->left->semanticValue + " is " + rightIdNode->id_name + ")";
        
        cout << "Semantic check: 'is' operator - checking if " << 
                (node->left->inferredType ? node->left->inferredType->name : "unknown") << 
                " is " << rightIdNode->id_name << endl;
        return;
    }
    
    // For all other operators, visit both operands normally
    node->left->accept(this, context);
    node->right->accept(this, context);
    
    if (node->op == "+" || node->op == "-" || node->op == "*" || node->op == "/" || node->op == "%" || node->op == "^") {
        checkTypeCompatibility(Context::numberType, node->left->inferredType, "left operand of " + node->op);
        checkTypeCompatibility(Context::numberType, node->right->inferredType, "right operand of " + node->op);
        
        node->inferredType = Context::numberType;
        node->semanticValue = "arithmetic_op(" + node->left->semanticValue + ", " + node->op + ", " + node->right->semanticValue + ")";
        
    } else if (node->op == "==" || node->op == "!=" || node->op == "<" || node->op == ">" || node->op == "<=" || node->op == ">=") {
        if (node->left->inferredType && node->right->inferredType) {
            auto commonType = context->findCommonSupertype(node->left->inferredType, node->right->inferredType);
            if (!commonType || commonType->name == "Expression") {
                throw std::runtime_error("Type error: Incompatible types in comparison: " + 
                                       node->left->inferredType->name + " and " + node->right->inferredType->name);
            }
        }
        node->inferredType = Context::boolType;
        node->semanticValue = "comparison_op(" + node->left->semanticValue + ", " + node->op + ", " + node->right->semanticValue + ")";
    } else if (node->op == "&" || node->op == "|") {
        checkTypeCompatibility(Context::boolType, node->left->inferredType, "left operand of " + node->op);
        checkTypeCompatibility(Context::boolType, node->right->inferredType, "right operand of " + node->op);
        
        node->inferredType = Context::boolType;
        node->semanticValue = "logical_op(" + node->left->semanticValue + ", " + node->op + ", " + node->right->semanticValue + ")";
        
    } else if (node->op == "@") {
        // String concatenation operator
        // Left operand should be a string, right operand can be string or number
        if (node->left->inferredType && !context->canAssign(node->left->inferredType, Context::stringType)) {
            throw std::runtime_error("Type error: Left operand of @ must be a string, got " + 
                                   node->left->inferredType->name);
        }
        if (node->right->inferredType && 
            !context->canAssign(node->right->inferredType, Context::stringType) &&
            !context->canAssign(node->right->inferredType, Context::numberType)) {
            throw std::runtime_error("Type error: Right operand of @ must be a string or number, got " + 
                                   node->right->inferredType->name);
        }
        node->inferredType = Context::stringType;
        node->semanticValue = "string_concat(" + node->left->semanticValue + ", " + node->right->semanticValue + ")";
    } else if (node->op == "@@") {
        // String concatenation operator
        // Left operand should be a string, right operand can be string or number
        if (node->left->inferredType && !context->canAssign(node->left->inferredType, Context::stringType)) {
            throw std::runtime_error("Type error: Left operand of @@ must be a string, got " + 
                                   node->left->inferredType->name);
        }
        if (node->right->inferredType && 
            !context->canAssign(node->right->inferredType, Context::stringType) &&
            !context->canAssign(node->right->inferredType, Context::numberType)) {
            throw std::runtime_error("Type error: Right operand of @@ must be a string or number, got " + 
                                   node->right->inferredType->name);
        }
        node->inferredType = Context::stringType;
        node->semanticValue = "string_double_concat(" + node->left->semanticValue + ", " + node->right->semanticValue + ")";
    } else if (node->op == "=" || node->op == ":=") {
        if (node->left->inferredType && node->right->inferredType) {
            if (!context->canAssign(node->right->inferredType, node->left->inferredType)) {
                throw std::runtime_error("Type error: Cannot assign " + node->right->inferredType->name + 
                                       " to " + node->left->inferredType->name);
            }
        }
        node->inferredType = node->left->inferredType;
        node->semanticValue = "assignment(" + node->left->semanticValue + ", " + node->right->semanticValue + ")";
        
    } else {
        throw std::runtime_error("Semantic error: Unknown binary operator: " + node->op);
    }
}

void SemanticCheckerVisitor::visit(FunctionCallNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting FunctionCallNode: " << node->func_name << endl;

    // Handle built-in functions with special semantics
    if (node->func_name == "print") {
        node->argument->accept(this, context);
        node->inferredType = Context::voidType;
        node->semanticValue = "print_call(" + node->argument->semanticValue + ")";
        cout << "Built-in function 'print' called" << endl;
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
                cout << "Warning: Argument has no inferred type, assuming number" << endl;
            }
        }
    } else {
        // Single argument case
        if (node->argument->inferredType) {
            argTypes.push_back(node->argument->inferredType);
        } else {
            argTypes.push_back(Context::numberType);
            cout << "Warning: Single argument has no inferred type, assuming number" << endl;
        }
    }

    // Try to resolve function with exact parameter type matching
    auto returnType = context->getFuncReturnType(node->func_name, argTypes);
    
    if (!returnType) {
        // Try with type coercion for numeric types
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
                cout << "Function call resolved with type coercion" << endl;
            }
        }
    }
    
    if (!returnType) {
        // Final fallback: check if function exists with any parameter count
        int argCount = argTypes.size();
        if (!context->isDefined(node->func_name, argCount)) {
            throw std::runtime_error("Semantic error: Function '" + node->func_name + 
                                   "' with " + to_string(argCount) + " parameter(s) not found. " +
                                   "Available overloads don't match the provided argument types.");
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
        
        throw std::runtime_error("Semantic error: Function '" + node->func_name + 
                               "' expects parameter types (" + expectedTypes + 
                               ") but got (" + actualTypes + ")");
    }
    
    node->inferredType = returnType;
    
    // Build semantic value with proper argument representation
    string argsValue = "";
    if (argTypes.size() == 1) {
        argsValue = node->argument->semanticValue;
    } else {
        argsValue = "args(" + to_string(argTypes.size()) + ")";
    }
    
    node->semanticValue = "func_call(" + node->func_name + ", " + argsValue + ")";
    
    cout << "Semantic check: Function '" << node->func_name << "' with " << 
            argTypes.size() << " argument(s) returns " << returnType->name << endl;
}

void SemanticCheckerVisitor::visit(IDNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting IDNode: " << node->id_name << endl;
    
    // Check if we're in a method context and the variable might be a member variable
    if (inMethodContext && currentTypeDef) {
        // First check if it's a local variable/parameter in the current function scope
        auto localVarType = context->getVarType(node->id_name);
        bool isLocalVariable = false;
        
        // Check if it's defined in the immediate function context (not inherited from parent contexts)
        if (localVarType && context->isLocal(node->id_name)) {
            isLocalVariable = true;
        }
        
        // If it's not a local variable, check if it's a member variable of the current type
        if (!isLocalVariable) {
            // Check if it's a member variable (property) of the current type
            if (currentTypeDef->hasProperty(node->id_name)) {
                throw std::runtime_error("Semantic error: Member variable '" + node->id_name + 
                                       "' cannot be accessed directly in method. Use 'self." + 
                                       node->id_name + "' instead.");
            }
        }
    }
    
    auto varType = context->getVarType(node->id_name);
    if (!varType) {
        if (!context->isDefined(node->id_name)) {
            throw std::runtime_error("Semantic error: Variable '" + node->id_name + "' is not defined");
        }
        varType = std::make_shared<TypeInfo>("unknown", TypeKind::INFERRED);
    }
    
    node->inferredType = varType;
    node->semanticValue = "var_ref(" + node->id_name + ")";
    
    cout << "Semantic check: Variable '" << node->id_name << "' has type " << varType->name << endl;
}

void SemanticCheckerVisitor::visit(BlockNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting BlockNode" << endl;
	
	Context* blockContext = context->createChildContext();
	
	for (auto child : node->children) {
		child->accept(this, blockContext);
        node->inferredType = this->inferReturnType(child, blockContext);
	}
	
	cout << "Semantic check: Block processed with " << node->children.size() << " statements" << endl;
}

void SemanticCheckerVisitor::visit(ArgsList* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting ArgsList with " << node->children.size() << " arguments" << endl;

	for (size_t i = 0; i < node->children.size(); i++) {
		for (size_t j = i + 1; j < node->children.size(); j++) {
			if (node->children[i]->id_name == node->children[j]->id_name) {
				throw std::runtime_error("Semantic error: Duplicate parameter name '" + node->children[i]->id_name + "'");
			}
		}
	}

	for (auto child : node->children) {
		cout << "Processing argument: " << child->id_name << endl;
	}
	
	cout << "Semantic check: All argument names are unique" << endl;
}

void SemanticCheckerVisitor::visit(AssignFuncNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting AssignFuncNode: " << node->func_name << endl;

    int argCount = node->args->children.size();
    
    // Check for function redefinition in the current scope
    if (context->isLocal(node->func_name, argCount)) {
        throw std::runtime_error("Semantic error: Function '" + node->func_name + 
                               "' with " + to_string(argCount) + " parameters already defined in current scope");
    }

    // Create a new context for the function body
    Context* functionContext = context->createChildContext();

    // Process function parameters with type checking
    std::vector<std::shared_ptr<TypeInfo>> paramTypes;
    node->args->accept(this, context);
    
    for (auto* arg : node->args->children) {
        // For now, we default to number type, but this could be enhanced
        // to support explicit type annotations in function parameters
        auto paramType = Context::numberType;
        
        // Check for type annotations if they exist
        if (!arg->id_type.empty() && arg->id_type != "none") {
            auto annotatedType = context->getType(arg->id_type);
            if (annotatedType) {
                paramType = annotatedType;
                cout << "Parameter '" << arg->id_name << "' has explicit type: " << paramType->name << endl;
            } else {
                cout << "Warning: Type '" << arg->id_type << "' not found, using default number type" << endl;
            }
        }
        
        paramTypes.push_back(paramType);
        
        // Define parameter in function context
        if (!functionContext->defineVar(arg->id_name, paramType)) {
            throw std::runtime_error("Semantic error: Failed to define parameter '" + arg->id_name + "'");
        }
        
        cout << "Parameter '" << arg->id_name << "' defined with type " << paramType->name << endl;
    }

    // Handle explicit return type annotation
    std::shared_ptr<TypeInfo> declaredReturnType = nullptr;
    if (!node->func_type.empty() && node->func_type != "none") {
        declaredReturnType = context->getType(node->func_type);
        if (!declaredReturnType) {
            throw std::runtime_error("Semantic error: Return type '" + node->func_type + 
                                   "' not found for function '" + node->func_name + "'");
        }
        cout << "Function '" << node->func_name << "' has explicit return type: " << declaredReturnType->name << endl;
    }

    // Type check the function body
    node->body->accept(this, functionContext);
    
    // Infer return type from function body
    auto inferredReturnType = inferReturnType(node->body, functionContext);
    
    // Determine final return type
    std::shared_ptr<TypeInfo> returnType;
    if (declaredReturnType) {
        // Validate that inferred type is compatible with declared type
        if (inferredReturnType && !context->canAssign(inferredReturnType, declaredReturnType)) {
            throw std::runtime_error("Semantic error: Function '" + node->func_name + 
                                   "' body returns type '" + inferredReturnType->name + 
                                   "' but declared return type is '" + declaredReturnType->name + "'");
        }
        returnType = declaredReturnType;
        cout << "Function return type validated: declared=" << declaredReturnType->name << 
                ", inferred=" << (inferredReturnType ? inferredReturnType->name : "Expression") << endl;
    } else {
        // Use inferred type
        returnType = inferredReturnType ? inferredReturnType : Context::voidType;
        
        // Validate return type consistency for common cases
        if (returnType->name == "unknown" || returnType->name == "Expression") {
            cout << "Warning: Function '" << node->func_name << "' has unclear return type, using Expression" << endl;
            returnType = Context::voidType;
        } else {
            cout << "Function '" << node->func_name << "' inferred return type: " << returnType->name << endl;
        }
    }

    // Check for recursive function calls - basic validation
    if (node->func_name != "main") {  // Allow main to be special
        // This is a placeholder for more sophisticated recursive call analysis
        cout << "Function '" << node->func_name << "' body type-checked successfully" << endl;
    }

    // Register the function in the context
    if (!context->defineFunc(node->func_name, returnType, paramTypes)) {
        // Fallback to simpler registration if the advanced one fails
        if (!context->define(node->func_name, argCount)) {
            throw std::runtime_error("Semantic error: Failed to define function '" + node->func_name + "'");
        }
        cout << "Function registered with basic definition system" << endl;
    } else {
        cout << "Function registered with full type information" << endl;
    }
    
    node->inferredType = returnType;
    
    // Build semantic value
    string paramInfo = "params(";
    for (size_t i = 0; i < paramTypes.size(); i++) {
        if (i > 0) paramInfo += ", ";
        paramInfo += paramTypes[i]->name;
    }
    paramInfo += ")";
    
    node->semanticValue = "func_def(" + node->func_name + ", " + paramInfo + ", " + node->body->semanticValue + ")";
    
    cout << "Function '" << node->func_name << "' defined successfully:" << endl;
    cout << "  - Parameters: " << argCount << " (" << paramInfo << ")" << endl;
    cout << "  - Return type: " << returnType->name << endl;
    cout << "  - Body type: " << (node->body->inferredType ? node->body->inferredType->name : "Expression") << endl;
}

void SemanticCheckerVisitor::visit(LetAssign* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting LetAssign with " << node->assigns.size() << " assignments" << endl;

    Context* letContext = context->createChildContext();

    for (auto* assign : node->assigns) {
        if (letContext->isLocal(assign->var_id->id_name)) {
            throw std::runtime_error("Semantic error: Variable '" + assign->var_id->id_name + 
                                   "' redefined in let block");
        }
        
        cout << "Visiting LetAssign assignment" << endl;
        
        assign->accept(this, letContext);

        auto staticType = assign->inferredType ? 
                          assign->inferredType : 
                          std::make_shared<TypeInfo>("none", TypeKind::INFERRED);
        
        auto varType = assign->value->inferredType ? 
                      assign->value->inferredType : 
                      std::make_shared<TypeInfo>("none", TypeKind::INFERRED);
        

        if (!letContext->defineVar(assign->var_id->id_name, varType)) {
            throw std::runtime_error("Semantic error: Failed to define variable '" + assign->var_id->id_name + "'");
        }
        
        assign->var_id->inferredType = varType;
        assign->var_id->semanticValue = "var_def(" + assign->var_id->id_name + ", " + assign->value->semanticValue + ")";
    }

    cout << "Entering LetAssign body" << endl;
    node->body->accept(this, letContext);
    
    node->inferredType = node->body->inferredType;
    node->semanticValue = "let_expr(bindings, " + node->body->semanticValue + ")";
    
    cout << "Let expression type: " << (node->inferredType ? node->inferredType->name : "Expression") << endl;
}

void SemanticCheckerVisitor::visit(VarAssign* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting VarAssign: " << node->var_id->id_name << " static type " << node->var_id->id_type << endl;

    node->value->accept(this, context);    
    
    if (!node->var_id->id_type.empty() && node->var_id->id_type != "none") {
        auto castType = context->getType(node->var_id->id_type);
        if (!castType) {
            throw std::runtime_error("Semantic error: Type '" + node->var_id->id_type + "' not found for cast");
        }
        
        auto valueType = node->value->inferredType;
        if (!valueType) {
            throw std::runtime_error("Semantic error: Cannot infer type of value for cast");
        }
        
        if (!context->canAssign(valueType, castType)) {
            throw std::runtime_error("Semantic error: Type '" + valueType->name + 
                                   "' does not conform to '" + castType->name + "' for cast");
        }
        
        node->var_id->inferredType = castType;
        node->inferredType = castType;
        
        cout << "Semantic check: Type cast from '" << valueType->name << "' to '" << 
                castType->name << "' is valid" << endl;
    } else {
        node->var_id->inferredType = node->value->inferredType;
        node->inferredType = node->value->inferredType;
    }
    
    if (!node->var_id->semanticValue.empty() || !node->value->semanticValue.empty()) {
        node->semanticValue = "var_assign(" + node->var_id->id_name + ", " + node->value->semanticValue + ")";
    }
    
    cout << "Semantic check: Variable assignment '" << node->var_id->id_name << "' with type " << 
            (node->inferredType ? node->inferredType->name : "unknown") << endl;
}

void SemanticCheckerVisitor::visit(NewTypeNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting NewTypeNode: new " << node->id_type_name << endl;

    // Check if the type exists
    auto typeInfo = context->getType(node->id_type_name);
    if (!typeInfo) {
        throw std::runtime_error("Semantic error: Type '" + node->id_type_name + "' is not defined");
    }

    // Visit constructor arguments
    for (auto* expr : node->expr_list) {
        expr->accept(this, context);
    }

    // For now, we assume all constructors are valid
    // In a more complete implementation, we would check constructor signatures
    node->inferredType = typeInfo;
    node->semanticValue = "new_instance(" + node->id_type_name + ", args:" + to_string(node->expr_list.size()) + ")";
    
    cout << "Semantic check: Object instantiation of type '" << node->id_type_name << "' is valid" << endl;
}

void SemanticCheckerVisitor::visit(VarAssignList* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting VarAssignList with " << node->assigns.size() << " assignments" << endl;

	for (size_t i = 0; i < node->assigns.size(); i++) {
		for (size_t j = i + 1; j < node->assigns.size(); j++) {
			if (node->assigns[i]->var_id == node->assigns[j]->var_id) {
				throw std::runtime_error("Semantic error: Duplicate variable assignment '" + node->assigns[i]->var_id->id_name + "' in the same list");
			}
		}
	}

	for (auto* assign : node->assigns) {
		assign->accept(this, context);
	}
	
	cout << "Semantic check: All variable assignments in list are unique and valid" << endl;
}

void SemanticCheckerVisitor::visit(VarAssignType* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting VarAssignType: " << node->var_name << " := new " << node->new_type->id_type_name << endl;

    auto instanceType = context->getType(node->new_type->id_type_name);
    if (!instanceType) {
        if (!context->isDefined(node->new_type->id_type_name, 0)) {
            throw std::runtime_error("Semantic error: Type '" + node->new_type->id_type_name + "' is not defined");
        }
        instanceType = std::make_shared<TypeInfo>(node->new_type->id_type_name, TypeKind::CLASS);
    }

    if (!context->getVarType(node->var_name)) {
        if (!context->defineVar(node->var_name, instanceType)) {
            throw std::runtime_error("Semantic error: Variable '" + node->var_name + "' is already defined in this scope");
        }
    }

    node->body->accept(this, context);
    node->inferredType = node->body->inferredType;
    
    node->semanticValue = "type_instantiation(" + node->var_name + ", " + instanceType->name + ", " + node->body->semanticValue + ")";
    
    cout << "Semantic check: Type instantiation for '" << node->var_name << "' of type '" << instanceType->name << "' is valid" << endl;
}

void SemanticCheckerVisitor::visit(Conditional* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting Conditional (if-else)" << endl;

	node->bool_expr->accept(this, context);

	Context* ifContext = context->createChildContext();
	Context* elseContext = context->createChildContext();

	node->if_body->accept(this, ifContext);

	node->else_body->accept(this, elseContext);
	
	cout << "Semantic check: Conditional statement processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(BoolExprNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting BoolExprNode" << endl;

	node->expr->accept(this, context);
	
	// BoolExprNode should always have boolean type regardless of inner expression
	node->inferredType = Context::boolType;
	node->semanticValue = "bool_expr(" + node->expr->semanticValue + ")";
	
	cout << "Semantic check: Boolean expression processed with type: " << 
	        (node->inferredType ? node->inferredType->name : "unknown") << endl;
}

void SemanticCheckerVisitor::visit(WhileNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting WhileNode" << endl;

	node->bool_expr->accept(this, context);

	Context* whileContext = context->createChildContext();

	node->body->accept(this, whileContext);
	
	cout << "Semantic check: While loop processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(VarDesAssign* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting VarDesAssign: " << node->id->id_name << endl;

	// Check if the variable is defined
	if (!context->isDefined(node->id->id_name)) {
		throw std::runtime_error("Semantic error: Variable '" + node->id->id_name + "' is not defined");
	}

	// Get the variable's current type
	auto varType = context->getVarType(node->id->id_name);
	if (!varType) {
		throw std::runtime_error("Semantic error: Could not determine type of variable '" + node->id->id_name + "'");
	}

	// Type check the assigned value
	node->value->accept(this, context);
	
	auto valueType = node->value->inferredType;
	if (!valueType) {
		throw std::runtime_error("Semantic error: Could not infer type of assigned value");
	}

	// Check type compatibility
	if (!context->canAssign(valueType, varType)) {
		throw std::runtime_error("Semantic error: Cannot assign value of type '" + valueType->name + 
								"' to variable '" + node->id->id_name + "' of type '" + varType->name + "'");
	}

	// Set types for the nodes
	node->id->inferredType = varType;
	node->inferredType = varType;
	node->semanticValue = "var_reassign(" + node->id->id_name + ", " + node->value->semanticValue + ")";
	
	cout << "Semantic check: Variable '" << node->id->id_name << "' reassigned with compatible type " << 
			valueType->name << " -> " << varType->name << endl;
}

void SemanticCheckerVisitor::visit(ForNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "DEBUG: Visiting ForNode with iterator: " << node->id->id_name << endl;

	node->group->accept(this, context);

	Context* forContext = context->createChildContext();

	if (!forContext->define(node->id->id_name)) {
		throw std::runtime_error("Semantic error: Failed to define iterator variable '" + node->id->id_name + "'");
	}

	node->body->accept(this, forContext);
	
	cout << "Semantic check: For loop processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(TypeDeclNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "DEBUG: Visiting TypeDeclNode: " << node->id->id_name << endl;

    auto typeDef = std::make_shared<TypeDef>(node->id->id_name);
    
    if (context->isTypeDefined(node->id->id_name)) {
        throw std::runtime_error("Semantic error: Type '" + node->id->id_name + "' already defined");
    }

    if (!node->parents.empty()) {
        const std::string& parentName = node->parents[0];
        auto parentTypeDef = context->getTypeDef(parentName);
        
        if (!parentTypeDef) {
            throw std::runtime_error("Semantic error: Parent type '" + parentName + "' not found for type '" + node->id->id_name + "'");
        }
        
        typeDef->parentType = std::make_shared<TypeInfo>(parentName, TypeKind::CLASS);
        typeDef->parentType->typeDef = parentTypeDef;
        
        for (const auto& prop : parentTypeDef->properties) {
            typeDef->properties.push_back(prop);
            cout << "Inherited property '" << prop.name << "' from parent type " << parentName << endl;
        }
        
        for (const auto& method : parentTypeDef->methods) {
            typeDef->methods.push_back(method);
            cout << "Inherited method '" << method.name << "' from parent type " << parentName << endl;
        }
        
        cout << "Type '" << node->id->id_name << "' inherits from '" << parentName << "'" << endl;
    } else {
        if (node->id->id_name != "Object") {
            typeDef->parentType = Context::objectType;
            cout << "Type '" << node->id->id_name << "' implicitly inherits from Object" << endl;
        }
    }

    if (!context->defineType(node->id->id_name, typeDef)) {
        throw std::runtime_error("Semantic error: Failed to define type '" + node->id->id_name + "'");
    }

    Context* typeContext = context->createChildContext();
    auto selfType = std::make_shared<TypeInfo>(node->id->id_name, TypeKind::CLASS);
    selfType->typeDef = typeDef;
    
    if (!typeContext->defineVar("self", selfType)) {
        throw std::runtime_error("Semantic error: Failed to define 'self'");
    }

    if (typeDef->parentType && typeDef->parentType->typeDef) {
        for (const auto& prop : typeDef->parentType->typeDef->properties) {
            if (!typeContext->defineVar(prop.name, prop.type)) {
                cout << "Warning: Could not define inherited property '" << prop.name << "' in type context" << endl;
            } else {
                cout << "Added inherited property '" << prop.name << "' to type context for " << node->id->id_name << endl;
            }
        }
    }

    node->args->accept(this, context);
    for (auto* arg : node->args->children) {
        typeDef->genericParams.push_back(arg->id_name);
        auto genericType = std::make_shared<TypeInfo>(arg->id_name, TypeKind::GENERIC);
        if (!typeContext->defineVar(arg->id_name, genericType)) {
            throw std::runtime_error("Semantic error: Failed to define type parameter '" + arg->id_name + "'");
        }
    }

    for (auto* element : node->body) {
        if (auto* varAssign = dynamic_cast<VarAssign*>(element)) {
            varAssign->value->accept(this, typeContext);
            auto memberType = varAssign->value->inferredType ? 
                            varAssign->value->inferredType : 
                            std::make_shared<TypeInfo>("unknown", TypeKind::INFERRED);
            
            if (!typeContext->defineVar(varAssign->var_id->id_name, memberType)) {
                throw std::runtime_error("Semantic error: Failed to define member variable '" + 
                                       varAssign->var_id->id_name + "' in type '" + node->id->id_name + "'");
            }
            
            PropertyMemberInfo property(varAssign->var_id->id_name, memberType);
            
            bool found = false;
            for (auto& existingProp : typeDef->properties) {
                if (existingProp.name == varAssign->var_id->id_name) {
                    existingProp = property;
                    found = true;
                    cout << "Overrode inherited property '" << varAssign->var_id->id_name << "' in type " << node->id->id_name << endl;
                    break;
                }
            }
            
            if (!found) {
                typeDef->properties.push_back(property);
                cout << "Added new member variable '" << varAssign->var_id->id_name << "' of type " << 
                        memberType->name << " to type " << node->id->id_name << endl;
            }
                    
        } else if (auto* varAssignType = dynamic_cast<VarAssignType*>(element)) {
            auto instanceType = context->getType(varAssignType->new_type->id_type_name);
            if (!instanceType) {
                instanceType = std::make_shared<TypeInfo>(varAssignType->new_type->id_type_name, TypeKind::CLASS);
            }
            
            if (!typeContext->defineVar(varAssignType->var_name, instanceType)) {
                throw std::runtime_error("Semantic error: Failed to define member variable '" + 
                                       varAssignType->var_name + "' in type '" + node->id->id_name + "'");
            }
            
            PropertyMemberInfo property(varAssignType->var_name, instanceType);
            
            bool found = false;
            for (auto& existingProp : typeDef->properties) {
                if (existingProp.name == varAssignType->var_name) {
                    existingProp = property;
                    found = true;
                    cout << "Overrode inherited typed property '" << varAssignType->var_name << "' in type " << node->id->id_name << endl;
                    break;
                }
            }
            
            if (!found) {
                typeDef->properties.push_back(property);
                cout << "Added new typed member variable '" << varAssignType->var_name << "' of type " << 
                        instanceType->name << " to type " << node->id->id_name << endl;
            }
        }
    }

    for (auto* element : node->body) {
        // Check if this is a method definition and set method context
        if (auto* funcNode = dynamic_cast<AssignFuncNode*>(element)) {
            // Set method context before processing the method
            bool prevInMethodContext = inMethodContext;
            auto prevCurrentTypeDef = currentTypeDef;
            
            inMethodContext = true;
            currentTypeDef = typeDef;
            
            // Process the method with method context set
            element->accept(this, typeContext);
            
            // Restore previous context
            inMethodContext = prevInMethodContext;
            currentTypeDef = prevCurrentTypeDef;
        } else {
            // Regular processing for non-method elements
            element->accept(this, typeContext);
        }
        
        if (auto* funcNode = dynamic_cast<AssignFuncNode*>(element)) {
            std::vector<std::shared_ptr<TypeInfo>> paramTypes;
            for (auto* param : funcNode->args->children) {
                paramTypes.push_back(Context::numberType);
            }
            auto returnType = funcNode->body->inferredType ? 
                            funcNode->body->inferredType : Context::voidType;
            
            MethodMemberInfo method(funcNode->func_name, returnType, paramTypes);
            
            bool found = false;
            for (auto& existingMethod : typeDef->methods) {
                if (existingMethod.name == funcNode->func_name && existingMethod.paramTypes.size() == paramTypes.size()) {
                    existingMethod = method;
                    found = true;
                    cout << "Overrode inherited method '" << funcNode->func_name << "' in type " << node->id->id_name << endl;
                    break;
                }
            }
            
            if (!found) {
                typeDef->methods.push_back(method);
                cout << "Added new method '" << funcNode->func_name << "' with return type " << 
                        returnType->name << " to type " << node->id->id_name << endl;
            }
        }
    }
    
    node->inferredType = selfType;
    node->semanticValue = "type_def(" + node->id->id_name + 
                         ", properties:" + to_string(typeDef->properties.size()) +
                         ", methods:" + to_string(typeDef->methods.size()) + ")";
    
    cout << "Type '" << node->id->id_name << "' defined with " << 
            typeDef->properties.size() << " properties and " <<
            typeDef->methods.size() << " methods" << endl;
}

void SemanticCheckerVisitor::visit(ASTNodeVector* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting ASTNodeVector with " << node->children.size() << " elements" << endl;

	for (auto* child : node->children) {
		child->accept(this, context);
	}
	
	cout << "Semantic check: ASTNodeVector processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(ExprsList* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting ExprsList with " << node->children.size() << " expressions" << endl;

	for (auto* expr : node->children) {
		expr->accept(this, context);
	}
	
	cout << "Semantic check: ExprsList processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(ProgramNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting ProgramNode (root)" << endl;

	if (node->getNode()) {
		node->getNode()->accept(this, context);
	} else {
		cout << "Warning: ProgramNode has no child node" << endl;
	}
	
	cout << "Semantic check: Program processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(AccessNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    std::cout << "Visiting AccessNode: " << node->var_name << " accessing member" << std::endl;
    
    std::shared_ptr<TypeInfo> objectType;
    
    if (node->var_name == "self") {
        objectType = context->getVarType("self");
        if (!objectType) {
            throw std::runtime_error("Semantic error: 'self' only valid in type context");
        }
    } else {
        objectType = context->getVarType(node->var_name);
        if (!objectType) {
            throw std::runtime_error("Semantic error: Variable '" + node->var_name + "' not defined");
        }
    }

    node->member->accept(this, context);
    
    if (objectType && objectType->typeDef) {
        if (auto* attrMember = dynamic_cast<AttributeMember*>(node->member)) {
            auto* prop = objectType->typeDef->findProperty(attrMember->name);
            if (prop) {
                node->inferredType = prop->type;
                node->semanticValue = "attr_access(" + node->var_name + "." + attrMember->name + ")";
            } else {
                throw std::runtime_error("Semantic error: Property '" + attrMember->name + 
                                       "' not found in type '" + objectType->name + "'");
            }
        } else if (auto* methodMember = dynamic_cast<MethodMember*>(node->member)) {
            std::vector<std::shared_ptr<TypeInfo>> argTypes;
            for (auto* arg : methodMember->args) {
                arg->accept(this, context);
                if (arg->inferredType) {
                    argTypes.push_back(arg->inferredType);
                }
            }
            
            auto* method = context->resolveMethod(objectType, methodMember->name, argTypes);
            if (method) {
                node->inferredType = method->returnType;
                node->semanticValue = "method_call(" + node->var_name + "." + methodMember->name + 
                                    ", args:" + to_string(argTypes.size()) + ")";
            } else {
                throw std::runtime_error("Semantic error: Method '" + methodMember->name + 
                                       "' not found or incompatible signature in type '" + objectType->name + "'");
            }
        }
    } else {
        node->inferredType = std::make_shared<TypeInfo>("unknown", TypeKind::INFERRED);
        node->semanticValue = "member_access(" + node->var_name + ".member)";
    }
    
    std::cout << "Member access resolved to type: " << 
                (node->inferredType ? node->inferredType->name : "unknown") << std::endl;
}

void SemanticCheckerVisitor::visit(TypeAssMember* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    std::cout << "Visiting TypeAssMember: " << node->get_name() << std::endl;
    std::cout << "Semantic check: TypeAssMember processed" << std::endl;
}

void SemanticCheckerVisitor::visit(AttributeMember* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    std::cout << "Visiting AttributeMember: " << node->name << std::endl;
    
    if (!node->inferredType) {
        node->inferredType = std::make_shared<TypeInfo>("unknown", TypeKind::INFERRED);
    }
    node->semanticValue = "attribute(" + node->name + ")";
    
    std::cout << "AttributeMember '" << node->name << "' processed" << std::endl;
}

void SemanticCheckerVisitor::visit(MethodMember* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    std::cout << "Visiting MethodMember: " << node->name << std::endl;
	
    std::vector<std::shared_ptr<TypeInfo>> argTypes;
    std::string argsSemanticValue = "args(";
    
    for (size_t i = 0; i < node->args.size(); i++) {
        node->args[i]->accept(this, context);
        if (node->args[i]->inferredType) {
            argTypes.push_back(node->args[i]->inferredType);
        }
        if (i > 0) argsSemanticValue += ", ";
        argsSemanticValue += node->args[i]->semanticValue;
    }
    argsSemanticValue += ")";
    
    if (!node->inferredType) {
        node->inferredType = std::make_shared<TypeInfo>("unknown", TypeKind::INFERRED);
    }
    node->semanticValue = "method(" + node->name + ", " + argsSemanticValue + ")";
    
    std::cout << "MethodMember '" << node->name << "' with " << argTypes.size() << " arguments" << std::endl;
}

void SemanticCheckerVisitor::visit(TypeCastNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    std::cout << "Visiting TypeCastNode: casting to " << node->target_type << std::endl;
    
    // Visit the expression being cast
    node->expr->accept(this, context);
    
    // Check if the target type exists
    auto targetType = context->getType(node->target_type);
    if (!targetType) {
        throw std::runtime_error("Semantic error: Target type '" + node->target_type + "' is not defined");
    }
    
    // Get the source type
    auto sourceType = node->expr->inferredType;
    if (!sourceType) {
        throw std::runtime_error("Semantic error: Cannot infer type of expression being cast");
    }
    
    // Check if cast is valid (for inheritance)
    if (!context->canAssign(sourceType, targetType) && !context->isSubtype(sourceType, targetType)) {
        throw std::runtime_error("Semantic error: Cannot cast from '" + sourceType->name + 
                               "' to '" + targetType->name + "'");
    }
    
    // Set the result type as the target type
    node->inferredType = targetType;
    node->semanticValue = "type_cast(" + node->expr->semanticValue + ", " + node->target_type + ")";
    
    std::cout << "Type cast from '" << sourceType->name << "' to '" << targetType->name << "' is valid" << std::endl;
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