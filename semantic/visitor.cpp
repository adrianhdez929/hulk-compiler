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
        if (node->node->inferredType && node->node->inferredType->name == Context::boolType->name) {
            addError("Type error in line " + std::to_string(node->line) + " Boolean negation requires boolean operand, got " + 
                    node->node->inferredType->name);
            return;
        }
        node->inferredType = Context::boolType;
        node->semanticValue = "bool_negation(" + node->node->semanticValue + ")";
    } else if (node->op == "-") {
        if (node->node->inferredType && node->node->inferredType->name == Context::numberType->name) {
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
    
    if (node->op == "is") {
        node->left->accept(this, context);
        
        if (!node->left->inferredType) {
            addError("Semantic error in line " + std::to_string(node->line) + ": Cannot determine type of left operand in 'is' expression");
            return;
        }
        
        auto* rightIdNode = dynamic_cast<IDNode*>(node->right);
        if (!rightIdNode) {
            addError("Semantic error in line " + std::to_string(node->line) + " : Right operand of 'is' must be a type name");
            return;
        }
        
        auto targetType = context->getType(rightIdNode->id_name);
        if (!targetType) {
            addError("Semantic error in line " + std::to_string(node->line) + " : Type '" + rightIdNode->id_name + "' not found in 'is' expression");
            return;
        }
        return;
    }
    
    node->left->accept(this, context);
    node->right->accept(this, context);
    
    if (node->op == "+" || node->op == "-" || node->op == "*" || node->op == "/" || node->op == "%" || node->op == "^") {
        checkTypeCompatibility(Context::numberType, node->left->inferredType, "left operand of " + node->op, node->line);
        checkTypeCompatibility(Context::numberType, node->right->inferredType, "right operand of " + node->op, node->line);
    } else if (node->op == "==" || node->op == "!=" || node->op == "<" || node->op == ">" || node->op == "<=" || node->op == ">=") {
        checkTypeCompatibility(node->left->inferredType, node->right->inferredType, "comparison operands", node->line);
        if (!node->left->inferredType || !node->right->inferredType) {
            addError("Type error in line " + std::to_string(node->line) + " : Cannot compare untyped expressions");
            return;
        }
    } else if (node->op == "&" || node->op == "|") {
        checkTypeCompatibility(Context::boolType, node->left->inferredType, "left operand of " + node->op, node->line);
        checkTypeCompatibility(Context::boolType, node->right->inferredType, "right operand of " + node->op, node->line);
    } else if (node->op == "@") {
        if (node->left->inferredType && node->left->inferredType->name == Context::stringType->name) {
            addError("Type error in line " + std::to_string(node->line) + ": Left operand of @ must be a string, got " + 
                                   node->left->inferredType->name);
            return;
        }
        if (node->right->inferredType && 
            node->right->inferredType->name != Context::stringType->name &&
            node->right->inferredType->name != Context::numberType->name) {
            addError("Type error in line " + std::to_string(node->line) + " : Right operand of @ must be a string or number, got " + 
                                   node->right->inferredType->name);
            return;
        }
    } else if (node->op == "@@") {
        if (node->left->inferredType && node->left->inferredType->name != Context::stringType->name) {
            addError("Type error in line " + std::to_string(node->line) + " : Left operand of @@ must be a string, got " + 
                                   node->left->inferredType->name);
            return;
        }
        if (node->right->inferredType && 
            node->right->inferredType->name != Context::stringType->name &&
            node->right->inferredType->name != Context::numberType->name) {
            addError("Type error in line " + std::to_string(node->line) + ": Right operand of @@ must be a string or number, got " + 
                                   node->right->inferredType->name);
            return;
        }
    } else if (node->op == "=" || node->op == ":=") {
        if (node->left->inferredType && node->right->inferredType) {
            if (node->right->inferredType->name != node->left->inferredType->name) {
                addError("Type error in line " + std::to_string(node->line) + " : Cannot assign " + node->right->inferredType->name + 
                                       " to " + node->left->inferredType->name);
                return;
            }
        }
        
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

    if (node->func_name == "print") {
        node->argument->accept(this, context);
        node->semanticValue = "print_call(" + node->argument->semanticValue + ")";
        // cout << "Built-in function 'print' called" << endl;
        return;
    } else if (node->func_name == "sin") {
        node->argument->accept(this, context);
        auto* arg = dynamic_cast<ASTNodeVector*>(node->argument);
        if (arg && arg->children[0]->inferredType && arg->children[0]->inferredType->name != Context::numberType->name) {
            addError("Type error in line " + std::to_string(node->line) + " : Argument for 'sin' must be a number, got " + 
                                   arg->children[0]->inferredType->name);
            return;
        }
        node->inferredType = Context::numberType; 
        return;
    } else if (node->func_name == "cos") {
        node->argument->accept(this, context);
        auto* arg = dynamic_cast<ASTNodeVector*>(node->argument);
        if (arg && arg->children[0]->inferredType && arg->children[0]->inferredType->name != Context::numberType->name) {
            addError("Type error in line " + std::to_string(node->line) + " : Argument for 'cos' must be a number, got " + 
                                   arg->children[0]->inferredType->name);
            return;
        }
        node->inferredType = Context::numberType;
        return;
    } else if (node->func_name == "sqrt") {
        node->argument->accept(this, context);
        auto* arg = dynamic_cast<ASTNodeVector*>(node->argument);
        if (arg && arg->children[0]->inferredType && arg->children[0]->inferredType->name != Context::numberType->name) {
            addError("Type error in line " + std::to_string(node->line) + " : Argument for 'sqrt' must be a number, got " + 
                                   arg->children[0]->inferredType->name);
            return;
        }
        node->inferredType = Context::numberType;
        return;
    } else if (node->func_name == "exp") {
        node->argument->accept(this, context);
        auto* arg = dynamic_cast<ASTNodeVector*>(node->argument);
        if (arg && arg->children[0]->inferredType && arg->children[0]->inferredType->name != Context::numberType->name) {
            addError("Type error in line " + std::to_string(node->line) + " : Argument for 'exp' must be a number, got " + 
                                   arg->children[0]->inferredType->name);
            return;
        }
        node->inferredType = Context::numberType;
        return;
    } else if (node->func_name == "log") {
        node->argument->accept(this, context);
        auto* arg = dynamic_cast<ASTNodeVector*>(node->argument);
        if (arg && arg->children[0]->inferredType && arg->children[0]->inferredType->name != Context::numberType->name) {
            addError("Type error in line " + std::to_string(node->line) + " : Argument for 'log' must be a number, got " + 
                                   arg->children[0]->inferredType->name);
            return;
        }
        node->inferredType = Context::numberType;
        return;
    } else if (node->func_name == "rand") {
        node->argument->accept(this, context);
        if (node->argument) {
            if (auto* nodeVector = dynamic_cast<ASTNodeVector*>(node->argument)) {
                if (!nodeVector->children.empty()) {
                    addError("Semantic error in line " + std::to_string(node->line) + " : 'rand' doesn't accept arguments");
                    return;
                }
            } else {
            addError("Semantic error in line " + std::to_string(node->line) + " : 'rand' doesn't accept arguments");
            return;
            }
        }
        node->semanticValue = "rand_call()";
        node->inferredType = Context::numberType; 
        return;
    }


    node->argument->accept(this, context);
    
    std::vector<std::shared_ptr<TypeInfo>> argTypes;
    
    if (auto* nodeVector = dynamic_cast<ASTNodeVector*>(node->argument)) {
        for (auto* arg : nodeVector->children) {
            if (arg->inferredType) {
                argTypes.push_back(arg->inferredType);
            } else {
                argTypes.push_back(Context::numberType);
                // cout << "Warning: Argument has no inferred type, assuming number" << endl;
            }
        }
    } else {
        if (node->argument->inferredType) {
            argTypes.push_back(node->argument->inferredType);
        } else {
            argTypes.push_back(Context::numberType);
            // cout << "Warning: Single argument has no inferred type, assuming number" << endl;
        }
    }

    auto returnType = context->getFuncReturnType(node->func_name, argTypes);
    bool isMethodCall = false;
    
    if (!returnType && context->currentType) {
        // cout << "Function '" << node->func_name << "' not found as global function, checking current type methods" << endl;
        
        // auto collectedMethods = std::vector<MethodInfo>(); // context->getMethodsForType(context->currentType->name, true);
        
        // for (const auto& method : collectedMethods) {
        //     if (method.name == node->func_name) {
        //         // cout << "Found method '" << node->func_name << "' in type '" << context->currentType->name << "'" << endl;
                
        //         if (method.paramTypes.size() == argTypes.size()) {
        //             bool typesMatch = true;
        //             for (size_t i = 0; i < argTypes.size(); i++) {
        //                 if (argTypes[i]->name != method.paramTypes[i]->name) {
        //                     typesMatch = false;
        //                     break;
        //                 }
        //             }
                    
        //             if (typesMatch) {
        //                 returnType = method.returnType;
        //                 isMethodCall = true;
        //                 // cout << "Method call '" << node->func_name << "' resolved with return type: " 
        //                     //  << (returnType ? returnType->name : "void") << endl;
                        
        //                 addError("Semantic error in line " + std::to_string(node->line) + 
        //                         " : Method '" + node->func_name + "' cannot be called directly. " +
        //                         "Use 'self." + node->func_name + "()' instead.");
        //                 break;
        //             }
        //         }
        //     }
        // }
    }
    
    if (!returnType) {
        std::vector<std::shared_ptr<TypeInfo>> coercedTypes;
        bool canCoerce = true;
        
        for (auto& argType : argTypes) {
            if (argType->name == "Number" || argType->name == "number" || 
                argType->name == Context::numberType->name) {
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
        int argCount = argTypes.size();
        if (!context->isDefined(node->func_name, argCount)) {
            if (context->currentType) {
                // auto collectedMethods = std::vector<MethodInfo>(); //context->getMethodsForType(context->currentType->name, true);
                bool isMethodWithDifferentArity = false;
                
                // for (const auto& method : collectedMethods) {
                //     if (method.name == node->func_name) {
                //         isMethodWithDifferentArity = true;
                //         addError("Semantic error in line " + std::to_string(node->line) + 
                //                 " : Method '" + node->func_name + "' exists in type '" + 
                //                 context->currentType->name + "' but with different parameter count. " +
                //                 "Expected " + to_string(method.paramTypes.size()) + " parameter(s), got " + 
                //                 to_string(argCount) + ". Use 'self." + node->func_name + "()' to call the method.");
                //         return;
                //     }
                // }
                
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
        
        TypeMethod funcInfo = context->getFunc(node->func_name, argTypes);
        
        string expectedTypes = "";
        for (size_t i = 0; i < funcInfo.arguments.size(); i++) {
            if (i > 0) expectedTypes += ", ";
            expectedTypes += funcInfo.arguments[i].name;
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
    
    string argsValue = "";
    if (argTypes.size() == 1) {
        argsValue = node->argument->semanticValue;
    } else {
        argsValue = "args(" + to_string(argTypes.size()) + ")";
    }
    
    node->inferredType = returnType;
    
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

    if (!currentTypeName.empty()) {
        std::cout << "Entering id node for type " << currentTypeName << " with id " << node->id_name << std::endl;
        std::shared_ptr<TypeInfo> currentType = context->getType(currentTypeName);
        
        if (!currentFunctionName.empty()) {
            if (currentType->hasMethod(currentFunctionName)) {
                std::cout << "Inside method body for type " << currentTypeName << " with function " << currentFunctionName << std::endl;
                auto method = currentType->getMethod(currentFunctionName);

                for (auto attr: method.arguments) {
                    std::cout << "Method " << currentFunctionName << " argument: " << attr.name << " of type " << attr.type->name << std::endl;
                    if (attr.name == node->id_name) {
                        node->inferredType = attr.type;
                        return;
                    }
                }

                addError("Error in line " + std::to_string(node->line) + ": Variable '" + node->id_name + 
                            "' not found in method '" + currentFunctionName + "' of type '" + currentTypeName + "'");
            } else {
                addError("Error in line " + std::to_string(node->line) + ": Variable '" + node->id_name + " not found in method '" + currentFunctionName + "' of type '" + currentTypeName + "'");
            }
        }

        return;
    }

    if (context->isDefined(node->id_name)) {
        node->inferredType = context->getVarType(node->id_name);
    }

    if (node->id_name == "PI" || node->id_name == "E") {
        node->semanticValue = node->id_name;
        node->inferredType = Context::numberType;
        return;
    }

    if (!context->isDefined(node->id_name)) {
        addError("Semantic error in line " + std::to_string(node->line) + " : Variable '" + node->id_name + "' is not defined");
        return;
    }
    
    auto varType = context->getVarType(node->id_name);
    if (varType) {
        node->inferredType = varType;
    }
}

void SemanticCheckerVisitor::visit(BlockNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
		return;
	}

	// cout << "Visiting BlockNode" << endl;
	
	Context* blockContext = context->createChildContext();
	
	for (auto* child : node->children) {
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

	for (auto* child : node->children) {
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
    
    
    std::vector<TypeAttribute> argTypes = std::vector<TypeAttribute>();
    
    for (auto* arg : node->args->children) {
        if (arg->inferredType) {
            if (!context->getType(arg->inferredType->name)) {
                addError("Semantic error in line " + std::to_string(node->line) + " : Type '" + arg->inferredType->name + 
                                       "' is not defined for argument '" + arg->id_name + "'");
                continue;
            }
        } else {
            if (!context->getType(arg->id_type)) {
                addError("Semantic error in line " + std::to_string(node->line) + " : Type '" + arg->id_type + 
                                       "' is not defined for argument '" + arg->id_name + "'");
                continue;
            } else {
                arg->inferredType = context->getType(arg->id_type);
            }
        }
        
        functionContext->defineVar(arg->id_name, arg->inferredType ? arg->inferredType : Context::numberType); // Default to number type
        argTypes.push_back(TypeAttribute{arg->id_name, globalContext->getType(arg->inferredType->name)});
        
        arg->accept(this, functionContext);
    }

    if (context->currentType) {
        auto selfType = std::make_shared<TypeInfo>(context->currentType->name);
        selfType = context->currentType;
    } else {
       context->defineFunc(node->func_name, node->inferredType, argTypes);
    }
    
    currentFunctionName = node->func_name;
    node->body->accept(this, functionContext);
    currentFunctionName.clear();
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
    }

    // cout << "Entering LetAssign body" << endl;
    node->body->accept(this, letContext);
    
    // cout << "Let expression type: " << (node->inferredType ? node->inferredType->name : "Expression") << endl;
}

void SemanticCheckerVisitor::visit(VarAssign* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
        return;
    }

    // cout << "Visiting VarAssign: " << node->var_id->id_name << " static type " << node->var_id->id_type << endl;

    if (context->isDefined(node->var_id->id_name)) {
        auto existingType = context->getVarType(node->var_id->id_name);
        if (existingType && node->value->inferredType->name != existingType->name) {
            addError("Semantic error in line " + std::to_string(node->line) + " : Cannot assign " + 
                                   node->value->inferredType->name + " to existing variable " + 
                                   node->var_id->id_name + " of type " + existingType->name);
            return;
        }
    } else {
        // cout << "Defining new variable: " << node->var_id->id_name << " with type " << 
                // (node->inferredType ? node->inferredType->name : "unknown") << endl;
        context->defineVar(node->var_id->id_name, node->inferredType);
    }
    
    node->value->accept(this, context);
    
    // cout << "Semantic check: Variable assignment '" << node->var_id->id_name << "' with type " << 
            // (node->inferredType ? node->inferredType->name : "unknown") << endl;
}

void SemanticCheckerVisitor::visit(NewTypeNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
		return;
    }

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

	
	node->value->accept(this, context);
	
	
	node->semanticValue = "var_reassign(" + node->id->id_name + ", " + node->value->semanticValue + ")";
	
	// cout << "Semantic check: Variable '" << node->id->id_name << "' reassigned with compatible type " << 
			// valueType->name << " -> " << varType->name << endl;
}

void SemanticCheckerVisitor::visit(ForNode* node, Context* context) {
// 	if (node == nullptr) {
// 		throw std::runtime_error("Node is null");
// 		return;
// 	}

// 	// cout << "DEBUG: Visiting ForNode with iterator: " << node->id->id_name << endl;

// 	node->group->accept(this, context);

// 	Context* forContext = context->createChildContext();

// 	if (!forContext->define(node->id->id_name)) {
//         addError("Semantic error in line: " + std::to_string(node->line) + " : Failed to define iterator variable '" + node->id->id_name + "'");
// 	}

// 	node->body->accept(this, forContext);
	
// 	// cout << "Semantic check: For loop processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(TypeDeclNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
		return;
    }

    currentTypeName = node->id->id_name;

    auto typeInfo = context->getType(node->id->id_name);
    auto typeContext = context->createChildContext();
    context->currentType = typeInfo;

    typeContext->defineVar("self", typeInfo);

    node->args->accept(this, typeContext);

    for (auto* element : node->body) {
        element->accept(this, typeContext);
    }

    context->currentType = nullptr;
    currentTypeName.clear();
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

    if (node->var_name == "self") {
        return;
    }

    if (!context->isDefined(node->var_name)) {
        addError("Semantic error in line " + std::to_string(node->line) + " : Variable '" + node->var_name + "' is not defined");
        return;
    }
    
    auto objectType = context->getVarType(node->var_name);
    if (!objectType) {
        addError("Semantic error in line " + std::to_string(node->line) + " : Cannot determine type of '" + node->var_name + "'");
        return;
    }

    
    std::string memberName = node->member->get_name();
    std::string objectTypeName = objectType->name;
    
    if (node->get_form() == TypeAssMember::Form::Attribute) {
        if (!objectType->hasAttribute(memberName)) {
            addError("Semantic error in line " + std::to_string(node->line) + " : Attribute '" + memberName + 
                                   "' not found in type '" + objectTypeName + "'");
            return;
        }
        
        node->inferredType = objectType->getAttribute(memberName).type;
    } else {
        if (!objectType->hasMethod(memberName)) {
            addError("Semantic error in line " + std::to_string(node->line) + " : Method '" + memberName + 
                                   "' not found in type '" + objectTypeName + "'");
            return;
        }
        
        node->inferredType = objectType->getMethod(memberName).returnType;
    }
    
    node->member->accept(this, context);
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
        node->inferredType = std::make_shared<TypeInfo>("unknown");
    }
    
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
    
    node->expr->accept(this, context);
    
    
    node->semanticValue = "type_cast(" + node->expr->semanticValue + ", " + node->target_type + ")";
    
    // std::cout << "Type cast from '" << sourceType->name << "' to '" << targetType->name << "' is valid" << std::endl;
}
