#include "visitor.h"
#include "../Ast/ast.hpp"
#include "context.h"
#include <iostream>

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
        node->inferredType = Context::intType; 
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
    
    if (node->op == "!" || node->op == "not") {
        if (node->node->inferredType && !context->canAssign(node->node->inferredType, Context::boolType)) {
            throw std::runtime_error("Type error: Boolean negation requires boolean operand, got " + 
                                   node->node->inferredType->name);
        }
        node->inferredType = Context::boolType;
        node->semanticValue = "bool_negation(" + node->node->semanticValue + ")";
    } else if (node->op == "-" || node->op == "+") {
        if (node->node->inferredType && !context->canAssign(node->node->inferredType, Context::intType)) {
            throw std::runtime_error("Type error: Arithmetic unary operation requires numeric operand, got " + 
                                   node->node->inferredType->name);
        }
        node->inferredType = Context::intType;
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
    
    node->left->accept(this, context);
    node->right->accept(this, context);
    
    if (node->op == "+" || node->op == "-" || node->op == "*" || node->op == "/" || node->op == "%" || node->op == "^") {
        checkTypeCompatibility(Context::intType, node->left->inferredType, "left operand of " + node->op);
        checkTypeCompatibility(Context::intType, node->right->inferredType, "right operand of " + node->op);
        
        node->inferredType = Context::intType;
        node->semanticValue = "arithmetic_op(" + node->left->semanticValue + ", " + node->op + ", " + node->right->semanticValue + ")";
        
    } else if (node->op == "==" || node->op == "!=" || node->op == "<" || node->op == ">" || node->op == "<=" || node->op == ">=") {
        if (node->left->inferredType && node->right->inferredType) {
            auto commonType = context->findCommonSupertype(node->left->inferredType, node->right->inferredType);
            if (!commonType || commonType->name == "void") {
                throw std::runtime_error("Type error: Incompatible types in comparison: " + 
                                       node->left->inferredType->name + " and " + node->right->inferredType->name);
            }
        }
        node->inferredType = Context::boolType;
        node->semanticValue = "comparison_op(" + node->left->semanticValue + ", " + node->op + ", " + node->right->semanticValue + ")";
        
    } else if (node->op == "&&" || node->op == "||" || node->op == "and" || node->op == "or") {
        checkTypeCompatibility(Context::boolType, node->left->inferredType, "left operand of " + node->op);
        checkTypeCompatibility(Context::boolType, node->right->inferredType, "right operand of " + node->op);
        
        node->inferredType = Context::boolType;
        node->semanticValue = "logical_op(" + node->left->semanticValue + ", " + node->op + ", " + node->right->semanticValue + ")";
        
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

    cout << "Visiting FunctionNode: " << node->func_name << endl;

    node->argument->accept(this, context);
    
    std::vector<std::shared_ptr<TypeInfo>> argTypes;
    if (node->argument->inferredType) {
        argTypes.push_back(node->argument->inferredType);
    }

    auto returnType = context->getFuncReturnType(node->func_name, argTypes);
    if (!returnType) {
        int argCount = 1;
        if (!context->isDefined(node->func_name, argCount)) {
            throw std::runtime_error("Semantic error: Function '" + node->func_name + 
                                   "' with compatible signature not found");
        }
        returnType = Context::voidType;
    }
    
    node->inferredType = returnType;
    node->semanticValue = "func_call(" + node->func_name + ", " + node->argument->semanticValue + ")";
    
    cout << "Semantic check: Function '" << node->func_name << "' returns " << returnType->name << endl;
}

void SemanticCheckerVisitor::visit(IDNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting IDNode: " << node->id_name << endl;
    
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
    
    if (context->isLocal(node->func_name, argCount)) {
        throw std::runtime_error("Semantic error: Function '" + node->func_name + 
                               "' with " + to_string(argCount) + " parameters already defined");
    }

    Context* functionContext = context->createChildContext();

    std::vector<std::shared_ptr<TypeInfo>> paramTypes;
    node->args->accept(this, context);
    
    for (auto* arg : node->args->children) {
        auto paramType = Context::intType;
        paramTypes.push_back(paramType);
        
        if (!functionContext->defineVar(arg->id_name, paramType)) {
            throw std::runtime_error("Semantic error: Failed to define parameter '" + arg->id_name + "'");
        }
    }

    node->body->accept(this, functionContext);
    auto returnType = node->body->inferredType ? node->body->inferredType : Context::voidType;

    if (!context->defineFunc(node->func_name, returnType, paramTypes)) {
        if (!context->define(node->func_name, argCount)) {
            throw std::runtime_error("Semantic error: Failed to define function '" + node->func_name + "'");
        }
    }
    
    node->inferredType = returnType;
    node->semanticValue = "func_def(" + node->func_name + ", params, " + node->body->semanticValue + ")";
    
    cout << "Function '" << node->func_name << "' defined with return type " << returnType->name << endl;
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
        
        assign->value->accept(this, letContext);
        auto varType = assign->value->inferredType ? 
                      assign->value->inferredType : 
                      std::make_shared<TypeInfo>("unknown", TypeKind::INFERRED);
        
        if (!letContext->defineVar(assign->var_id->id_name, varType)) {
            throw std::runtime_error("Semantic error: Failed to define variable '" + assign->var_id->id_name + "'");
        }
        
        assign->var_id->inferredType = varType;
        assign->var_id->semanticValue = "var_def(" + assign->var_id->id_name + ", " + assign->value->semanticValue + ")";
    }

    node->body->accept(this, letContext);
    
    node->inferredType = node->body->inferredType;
    node->semanticValue = "let_expr(bindings, " + node->body->semanticValue + ")";
    
    cout << "Let expression type: " << (node->inferredType ? node->inferredType->name : "void") << endl;
}

void SemanticCheckerVisitor::visit(VarAssign* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting VarAssign: " << node->var_id->id_name << endl;

    node->value->accept(this, context);
    
    if (!node->treated_as_type.empty() && node->treated_as_type != "none") {
        auto castType = context->getType(node->treated_as_type);
        if (!castType) {
            throw std::runtime_error("Semantic error: Type '" + node->treated_as_type + "' not found for cast");
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
	
	cout << "Semantic check: Boolean expression processed" << endl;
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

	if (!context->isDefined(node->id->id_name)) {
		throw std::runtime_error("Variable '" + node->id->id_name + "' is not defined");
	}

	this->visit(node->value, context);
}

void SemanticCheckerVisitor::visit(ForNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting ForNode with iterator: " << node->id->id_name << endl;

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

    cout << "Visiting TypeDeclNode: " << node->id->id_name << endl;

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
        element->accept(this, typeContext);
        
        if (auto* funcNode = dynamic_cast<AssignFuncNode*>(element)) {
            std::vector<std::shared_ptr<TypeInfo>> paramTypes;
            for (auto* param : funcNode->args->children) {
                paramTypes.push_back(Context::intType);
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