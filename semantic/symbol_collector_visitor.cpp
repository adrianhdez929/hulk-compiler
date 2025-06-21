#include "symbol_collector_visitor.h"
#include "context.h"
#include "../Ast/ast.hpp"
#include <iostream>
#include <algorithm>


SymbolCollectorVisitor::SymbolCollectorVisitor(Context* globalCtx) 
    : globalContext(globalCtx), typeCollector(nullptr), typeCollectorVisitor(nullptr), insideMethod(false) {
}

SymbolCollectorVisitor::SymbolCollectorVisitor(TypeCollectorVisitor* typeCollector) 
    : typeCollector(typeCollector), typeCollectorVisitor(typeCollector), globalContext(nullptr), insideMethod(false) {
    if (typeCollector) {
        globalContext = typeCollector->getGlobalContext();
    }
}

void SymbolCollectorVisitor::addError(const std::string& message) {
    errors.push_back(message);
}

bool SymbolCollectorVisitor::hasErrors() const {
    return !errors.empty();
}

const std::vector<std::string>& SymbolCollectorVisitor::getErrors() const {
    return errors;
}

void SymbolCollectorVisitor::printErrors() const {
    if (!errors.empty()) {
        for (const auto& error : errors) {
            std::cout << "SymbolCollector Error: " << error << std::endl;
        }
    }
}

void SymbolCollectorVisitor::visit(ASTNode* node, Context* context) {
    if (node) {
    }
}

void SymbolCollectorVisitor::visit(FloatNode* node, Context* context) {
    if (node) {
        node->inferredType = Context::numberType;
    }
}

void SymbolCollectorVisitor::visit(BoolNode* node, Context* context) {
    if (node) {
        node->inferredType = Context::boolType;
    }
}

void SymbolCollectorVisitor::visit(StringNode* node, Context* context) {
    if (node) {
        node->inferredType = Context::stringType;
    }
}

void SymbolCollectorVisitor::visit(UnaryOpNode* node, Context* context) {
    if (node && node->node) {
        node->node->accept(this, context);
        node->inferredType = node->node->inferredType;
    }
}

void SymbolCollectorVisitor::visit(BinOpNode* node, Context* context) {
    if (node) {
        if (node->left) node->left->accept(this, context);
        if (node->right) node->right->accept(this, context);
        if (node->op == "+" || node->op == "-" || node->op == "*" || node->op == "/" || node->op == "^" || node->op == "%") {
            node->inferredType = Context::numberType;
        } else if (node->op == "&" || node->op == "|" || node->op == "==" || node->op == "!=" || node->op == "is" || node->op == "<" || node->op == ">" || node->op == "<=" || node->op == ">=") {
            node->inferredType = Context::boolType;
        } else if (node->op == "@" || node->op == "@@") {
            node->inferredType = Context::stringType;
        } else {
            node->inferredType = Context::voidType;
        }
    }
}

void SymbolCollectorVisitor::visit(FunctionCallNode* node, Context* context) {
    if (!node) return;
    
    if (node->argument) {
        node->argument->accept(this, context);
    }

    std::shared_ptr<TypeInfo> returnType = Context::voidType;
    
    if (node->func_name == "print") {
        returnType = Context::voidType;
    } else if (node->func_name == "sin" || node->func_name == "cos" || 
               node->func_name == "sqrt" || node->func_name == "exp" || 
               node->func_name == "log" || node->func_name == "rand") {
        returnType = Context::numberType;
    }
    
    node->inferredType = returnType;
}

void SymbolCollectorVisitor::visit(IDNode* node, Context* context) {
    if (node && context) {
        if (node->inferredType) {
            return;
        }
        if (context->isDefined(node->id_name)) {
            node->inferredType = context->getVarType(node->id_name);
            return;
        }

        if (node->id_name == "PI" || node->id_name == "E") {
            node->inferredType = Context::numberType;
            return;
        }
        
        auto varType = context->getVarType(node->id_name);
        if (varType) {
            node->inferredType = varType;
        }
    }
}

void SymbolCollectorVisitor::visit(BlockNode* node, Context* context) {
    if (!node) return;
    
    std::shared_ptr<TypeInfo> lastType = Context::voidType;
    
    for (auto* child : node->children) {
        if (child) {
            child->accept(this, context);
            if (child->inferredType) {
                lastType = child->inferredType;
            }
        }
    }
    
    node->inferredType = lastType;
}

void SymbolCollectorVisitor::visit(ArgsList* node, Context* context) {
    if (!node) return;
    
    for (auto* child : node->children) {
        if (child) {
            child->accept(this, context);
        }
    }
    
    node->inferredType = Context::objectType;
}

void SymbolCollectorVisitor::visit(AssignFuncNode* node, Context* context) {
    if (!node) return;
    std::vector<TypeAttribute> methodArgs = std::vector<TypeAttribute>();

    for (auto* arg: node->args->children) {
        arg->accept(this, context);
        
        if (arg->id_type.empty() || arg->id_type == "none") {
            arg->inferredType = Context::numberType;
        } else {
            arg->inferredType = context->getType(arg->id_type);
        }

        if (arg->inferredType == nullptr) {
            addError("Type error in line " + std::to_string(node->line) + ": Could not resolve type '" + arg->id_type + "' for argument '" + arg->id_name + "'");
            continue;
        }

        methodArgs.push_back(TypeAttribute{arg->id_name, globalContext->getType(arg->inferredType->name)});
    }

    node->body->accept(this, context);
    node->inferredType = node->body->inferredType;

    if (!currentTypeName.empty()) {
        if (node->inferredType == nullptr) {
            addError("Type error in line " + std::to_string(node->line) + ": Could not infer return type for method '" + node->func_name + "' in type '" + currentTypeName + "'");
        }
        
        auto returnType = node->inferredType ? context->getType(node->inferredType->name) : Context::objectType;
        std::shared_ptr<TypeInfo> currentType = context->getType(currentTypeName);

        if (!currentType->defineMethod(node->func_name, returnType, methodArgs)) {
            addError("Could not define method '" + node->func_name + "' in type '" + currentTypeName + "'");
            return;
        }
    } else {
        if (node->inferredType == nullptr) {
            addError("Type error in line " + std::to_string(node->line) + ": Could not infer return type for function '" + node->func_name + "' in global scope");
        }

        auto returnType = node->inferredType ? context->getType(node->inferredType->name) : Context::objectType;
        if (!context->defineFunc(node->func_name, returnType, methodArgs)) {
            addError("Could not define function '" + node->func_name + "' in global scope");
        }
    }
}

void SymbolCollectorVisitor::visit(LetAssign* node, Context* context) {
    if (!node) return;
    
    for (auto* assign : node->assigns) {
        if (assign) {
            assign->accept(this, context);
        }
    }
    
    if (node->body) {
        node->body->accept(this, context);
        node->inferredType = node->body->inferredType;
    } else {
        node->inferredType = Context::objectType;
    }
}

void SymbolCollectorVisitor::visit(VarAssign* node, Context* context) {
    if (!node) return;
    
    // Debug output
    //           << "' - currentTypeName: '" << currentTypeName 
    //           << "', insideMethodBody: " << (insideMethod ? "true" : "false") << std::endl;


    node->value->accept(this, context);
    node->inferredType = node->value->inferredType;
    node->var_id->inferredType = node->inferredType;
    
    if (!currentTypeName.empty() && !insideMethod) {

        std::shared_ptr<TypeInfo> currentType = globalContext->getType(currentTypeName);

        if (!currentType->hasAttribute(node->var_id->id_name)) {
            currentType->defineAttribute(node->var_id->id_name, node->value->inferredType);
        }
    } 
    // else {
    //     processVariableDefinition(node, "global", context);
    // }
}

void SymbolCollectorVisitor::visit(NewTypeNode* node, Context* context) {
    if (node) {
        node->inferredType = globalContext->getType(node->id_type_name);
    }
}

void SymbolCollectorVisitor::visit(VarAssignType* node, Context* context) {
    if (!node) return;
    
    if (node->new_type) {
        node->new_type->accept(this, context);
    }
    
    if (node->body) {
        node->body->accept(this, context);
        node->inferredType = node->body->inferredType;
    } else {
        if (node->new_type && node->new_type->inferredType) {
            node->inferredType = node->new_type->inferredType;
        } else {
            node->inferredType = Context::objectType;
        }
    }
}

void SymbolCollectorVisitor::visit(VarAssignList* node, Context* context) {
    if (!node) return;
    
    std::shared_ptr<TypeInfo> lastType = Context::voidType;
    
    for (auto* assign : node->assigns) {
        if (assign) {
            assign->accept(this, context);
            if (assign->inferredType) {
                lastType = assign->inferredType;
            }
        }
    }
    
    node->inferredType = lastType;
}

void SymbolCollectorVisitor::visit(Conditional* node, Context* context) {
    if (node) {
        if (node->bool_expr) node->bool_expr->accept(this, context);
        if (node->if_body) node->if_body->accept(this, context);
        if (node->else_body) node->else_body->accept(this, context);
        
        if (node->if_body && node->if_body->inferredType) {
            node->inferredType = node->if_body->inferredType;
        }
    }
}

void SymbolCollectorVisitor::visit(BoolExprNode* node, Context* context) {
    if (node) {
        if (node->expr) {
            node->expr->accept(this, context);
        }
        node->inferredType = Context::boolType;
    }
}

void SymbolCollectorVisitor::visit(WhileNode* node, Context* context) {
    if (!node) return;
    
    // Process the boolean condition
    if (node->bool_expr) {
        node->bool_expr->accept(this, context);
    }
    
    // Process the loop body
    
    if (node->body) {
        node->body->accept(this, context);
    }
    
    node->inferredType = node->body ? node->body->inferredType : Context::voidType;
}

void SymbolCollectorVisitor::visit(VarDesAssign* node, Context* context) {
    if (!node) return;
    
    if (node->value) {
        node->value->accept(this, context);
    }
    
    if (node->id) {
        node->id->accept(this, context);
    }
    
    if (node->value && node->value->inferredType) {
        node->inferredType = node->value->inferredType;
    } else {
        node->inferredType = Context::voidType;
    }
}

void SymbolCollectorVisitor::visit(ForNode* node, Context* context) {
    if (!node) return;
    
    if (node->group) {
        node->group->accept(this, context);
    }
    
    if (node->body) {
        node->body->accept(this, context);
    }
    
    node->inferredType = Context::voidType;
}

void SymbolCollectorVisitor::visit(TypeDeclNode* node, Context* context) {
    if (!node) return;
    
    currentTypeName = node->id->id_name;

    for (auto* element : node->body) {
        if (element) {
            element->accept(this, context);
        }
    }
    
    // if (globalContext) {
    //     auto typeInfo = globalContext->getType(node->id->id_name);
    //     if (typeInfo && typeInfo->typeDef && typeInfo->typeDef->parentType) {
    //                 //  << typeInfo->typeDef->parentType->name << "'" << std::endl;
            
    //         auto inheritedAttrs = getAttributesForType(typeInfo->typeDef->parentType->name, true);
            
    //         auto inheritedMethods = getMethodsForType(typeInfo->typeDef->parentType->name, true);
    //     }
    // }
    
    currentTypeName.clear();
    
    std::shared_ptr<TypeInfo> declaredType = context->getType(node->id->id_name);
    if (declaredType) {
        node->inferredType = declaredType;
    } else {
        node->inferredType = Context::objectType;
    }
}

void SymbolCollectorVisitor::visit(ASTNodeVector* node, Context* context) {
    if (!node) return;
    
    std::shared_ptr<TypeInfo> lastType = Context::voidType;
    
    for (auto* child : node->children) {
        if (child) {
            child->accept(this, context);
            if (child->inferredType) {
                lastType = child->inferredType;
            }
        }
    }
    
    node->inferredType = lastType;
}

void SymbolCollectorVisitor::visit(ExprsList* node, Context* context) {
    if (!node) return;
    
    std::shared_ptr<TypeInfo> lastType = Context::voidType;
    
    for (auto* child : node->children) {
        if (child) {
            child->accept(this, context);
            if (child->inferredType) {
                lastType = child->inferredType;
            }
        }
    }
    
    node->inferredType = lastType;
}

void SymbolCollectorVisitor::visit(ProgramNode* node, Context* context) {
    if (!node) return;
    
    if (node->getNode()) {
        node->getNode()->accept(this, context);
        node->inferredType = node->getNode()->inferredType;
    } else {
        node->inferredType = Context::voidType;
    }
}

void SymbolCollectorVisitor::visit(AccessNode* node, Context* context) {
    if (!node) return;
    std::cout << "Visiting AccessNode: " << node->member->get_name() << std::endl;
    if (node->var_name == "self") {
        if (currentTypeName.empty()) {
            addError("Type error in line " + std::to_string(node->line) + ": 'self' reference outside of type context");
            return;
        }

        auto currentType = context->getType(currentTypeName);
        if (!currentType) {
            addError("Type error in line " + std::to_string(node->line) + ": Current type '" + currentTypeName + "' not found");
            return;
        }
        if (node->member->get_form() == TypeAssMember::Form::Method) {
            if (!currentType->hasMethod(node->member->get_name())) {
                addError("Type error in line " + std::to_string(node->line) + ": Type '" + currentTypeName + 
                         "' does not have method '" + node->member->get_name() + "'");
                return;
            }
            std::cout << "Method found: " << node->member->get_name() << std::endl;
            node->inferredType = currentType->getMethod(node->member->get_name()).returnType;
        } else {
            if (!currentType->hasAttribute(node->member->get_name())) {
                addError("Type error in line " + std::to_string(node->line) + ": Type '" + currentTypeName + 
                         "' does not have attribute '" + node->member->get_name() + "'");
                return;
            }
            std::cout << "Attribute found: " << node->member->get_name() << std::endl;
            node->inferredType = currentType->getAttribute(node->member->get_name()).type;
        }
        
        node->member->inferredType = node->inferredType;
    }
}

void SymbolCollectorVisitor::visit(TypeAssMember* node, Context* context) {
    if (!node) return;
    
    node->inferredType = Context::objectType;
}

void SymbolCollectorVisitor::visit(AttributeMember* node, Context* context) {
    if (!node) return;
    
    node->inferredType = Context::numberType; // Default attribute type
}

void SymbolCollectorVisitor::visit(MethodMember* node, Context* context) {
    if (!node) return;
    
    for (auto* arg : node->args) {
        if (arg) {
            arg->accept(this, context);
        }
    }
    
    node->inferredType = Context::objectType;
}

void SymbolCollectorVisitor::visit(TypeCastNode* node, Context* context) {
    if (!node) {
        addError("Null TypeCastNode");
        return;
    }

    node->expr->accept(this, context);

    std::shared_ptr<TypeInfo> sourceType = node->expr->inferredType ? node->expr->inferredType : context->getType(node->target_type);
    if (!sourceType) {
        addError("Cannot determine type of expression being cast");
        return;
    }
    
    std::shared_ptr<TypeInfo> targetType = context->getType(node->target_type);
    if (!targetType) {
        addError("Unknown target type in cast: " + node->target_type);
        return;
    }
    
    auto isSubclassOfByName = [context](const std::string& subTypeName, const std::string& superTypeName) -> bool {
        if (subTypeName == superTypeName) return true;
        
        auto subType = context->getType(subTypeName);
        if (!subType) {
            return false;
        }
        
        return subType->isCompatibleWith(superTypeName);
    };
    
    bool isValidCast = false;
    
    if (sourceType->name == targetType->name) {
        isValidCast = true;
    }
    else if (isSubclassOfByName(sourceType->name, targetType->name)) {
        isValidCast = true;
    }
    else if (isSubclassOfByName(targetType->name, sourceType->name)) {
        isValidCast = true;
    }
    
    if (!isValidCast) {
        addError("Invalid cast: Cannot cast from type '" + sourceType->name + 
                "' to type '" + targetType->name + "' - types are not related through inheritance");
        return;
    }
    
        //  << "' to type '" << targetType->name << "'" << std::endl;
    node->inferredType = targetType;
}

void SymbolCollectorVisitor::processAttributeDefinition(VarAssign* node, const std::string& typeName, Context* context) {
    if (!node || !node->var_id) {
        addError("Null VarAssign or var_id in processAttributeDefinition");
        return;
    }
    

    
    std::shared_ptr<TypeInfo> attrType = Context::numberType; // Default type

    if (!currentTypeName.empty()) {
        auto currentType = globalContext->getType(currentTypeName);

        if (!currentType->hasAttribute(node->var_id->id_name)) {
            attrType = node->value->inferredType;
            
            currentType->defineAttribute(node->var_id->id_name, attrType);
        }
    }

    if (node->value) {
        node->value->accept(this, context);
        if (node->value->inferredType) {
            attrType = node->value->inferredType;
        }
    }
    
    if (!node->var_id->id_type.empty()) {
        auto explicitType = context->getType(node->var_id->id_type);
        if (explicitType) {
            attrType = explicitType;
        }
    }
        
    node->var_id->inferredType = attrType;
    node->inferredType = attrType;
    
}

void SymbolCollectorVisitor::processVariableDefinition(VarAssign* node, const std::string& scope, Context* context) {
    if (node && node->var_id && node->value) {
        node->value->accept(this, context);
        
        std::shared_ptr<TypeInfo> declaredType = nullptr;
        std::shared_ptr<TypeInfo> valueType = node->value->inferredType ? node->value->inferredType : Context::numberType;
        
        if (!node->var_id->id_type.empty()) {
            declaredType = context->getType(node->var_id->id_type);
            if (!declaredType) {
                addError("Variable '" + node->var_id->id_name + "' declared with unknown type '" + node->var_id->id_type + "'");
                return;
            }
            
            if (!valueType->isCompatibleWith(declaredType->name) && !declaredType->isCompatibleWith(valueType->name)) {
                addError("Type mismatch: Cannot assign value of type '" + valueType->name + 
                        "' to variable '" + node->var_id->id_name + "' of type '" + declaredType->name + "'");
                return;
            }
        }
        
        std::shared_ptr<TypeInfo> varType = declaredType ? declaredType : valueType;
        
        context->defineVar(node->var_id->id_name, varType);
        
        node->var_id->inferredType = varType;
        node->inferredType = varType;
                
    }
}

void SymbolCollectorVisitor::processGlobalFunction(AssignFuncNode* node, Context* context) {
    if (node) {
        for (auto* arg : node->args->children) {
            if (arg) {
                arg->accept(this, context);
                if (arg->inferredType) {
                    continue;
                }
                if (!arg->id_type.empty()) {
                    arg->inferredType = context->getType(arg->id_type);
                } else {
                    arg->inferredType = Context::numberType;
                }
            }
        }

        node->body->accept(this, context);
        node->inferredType = node->body->inferredType;
    }
}
