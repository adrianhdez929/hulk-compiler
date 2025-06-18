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
        // std::cout << "\n=== Symbol Collection Errors ===" << std::endl;
        for (const auto& error : errors) {
            // std::cout << "Error: " << error << std::endl;
        }
    }
}

void SymbolCollectorVisitor::visit(ASTNode* node, Context* context) {
    if (node) {
    }
}

void SymbolCollectorVisitor::visit(FloatNode* node, Context* context) {
    if (node && !node->inferredType) {
        node->inferredType = Context::numberType;
    }
}

void SymbolCollectorVisitor::visit(BoolNode* node, Context* context) {
    if (node && !node->inferredType) {
        node->inferredType = Context::boolType;
    }
}

void SymbolCollectorVisitor::visit(StringNode* node, Context* context) {
    if (node && !node->inferredType) {
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
    
    node->inferredType = Context::voidType;
}

void SymbolCollectorVisitor::visit(AssignFuncNode* node, Context* context) {
    if (!node) return;

    for (auto* arg: node->args->children) {
        arg->accept(this, context);
        
        if (arg->inferredType) {
            continue;
        }
        if (arg->id_type.empty() || arg->id_type == "none") {
            arg->inferredType = Context::numberType;
        } else {
            arg->inferredType = context->getType(arg->id_type);
        }
    }

    node->body->accept(this, context);
    node->inferredType = node->body->inferredType;

    if (!currentTypeName.empty()) {
       processMethodDefinition(node, currentTypeName, context);
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
        node->inferredType = Context::voidType;
    }
}

void SymbolCollectorVisitor::visit(VarAssign* node, Context* context) {
    if (!node) return;
    
    // Debug output
    // std::cout << "SymbolCollector: Processing VarAssign '" << node->var_id->id_name 
    //           << "' - currentTypeName: '" << currentTypeName 
    //           << "', insideMethodBody: " << (insideMethod ? "true" : "false") << std::endl;


    node->value->accept(this, context);
    node->inferredType = node->value->inferredType;
    node->var_id->inferredType = node->inferredType;
    
    if (!currentTypeName.empty() && !insideMethod) {
        // std::cout << "  -> Processing as type attribute" << std::endl;
        processAttributeDefinition(node, currentTypeName, context);
    } 
    // else {
        // std::cout << "  -> Processing as variable definition" << std::endl;
    //     processVariableDefinition(node, "global", context);
    // }
}

void SymbolCollectorVisitor::visit(NewTypeNode* node, Context* context) {
    if (node) {
        // cout << "Setting inferredType for NewTypeNode '" << globalContext->getType(node->id_type_name)->name << "'" << std::endl;
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
            node->inferredType = Context::voidType;
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
    
    // std::cout << "SymbolCollector: Processing type body for '" << node->id->id_name << "'" << std::endl;
    currentTypeName = node->id->id_name;

    for (auto* element : node->body) {
        if (element) {
            element->accept(this, context);
        }
    }
    
    if (globalContext) {
        auto typeInfo = globalContext->getType(node->id->id_name);
        if (typeInfo && typeInfo->typeDef && typeInfo->typeDef->parentType) {
            // std::cout << "Type '" << node->id->id_name << "' inherits from '" 
                    //  << typeInfo->typeDef->parentType->name << "'" << std::endl;
            
            auto inheritedAttrs = getAttributesForType(typeInfo->typeDef->parentType->name, true);
            // std::cout << "Inherited " << inheritedAttrs.size() << " attributes from parent types" << std::endl;
            
            auto inheritedMethods = getMethodsForType(typeInfo->typeDef->parentType->name, true);
            // std::cout << "Inherited " << inheritedMethods.size() << " methods from parent types" << std::endl;
        }
    }
    
    currentTypeName.clear();
    
    std::shared_ptr<TypeInfo> declaredType = context->getType(node->id->id_name);
    if (declaredType) {
        node->inferredType = declaredType;
    } else {
        node->inferredType = Context::voidType;
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

    
}

void SymbolCollectorVisitor::visit(TypeAssMember* node, Context* context) {
    if (!node) return;
    
    node->inferredType = Context::voidType;
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
    
    node->inferredType = Context::voidType;
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
        if (!subType || !subType->typeDef) return false;
        
        std::shared_ptr<TypeInfo> current = subType;
        while (current && current->typeDef && current->typeDef->parentType) {
            current = current->typeDef->parentType;
            if (current->name == superTypeName) {
                return true;
            }
        }
        return false;
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
    else if (sourceType->isCompatibleWith(targetType)) {
        isValidCast = true;
    }
    
    if (!isValidCast) {
        addError("Invalid cast: Cannot cast from type '" + sourceType->name + 
                "' to type '" + targetType->name + "' - types are not related through inheritance");
        return;
    }
    
    // cout << "Casting from type '" << sourceType->name 
        //  << "' to type '" << targetType->name << "'" << std::endl;
    node->inferredType = targetType;
}

void SymbolCollectorVisitor::processMethodDefinition(AssignFuncNode* node, const std::string& typeName, Context* context) {
    if (!node) {
        addError("Null AssignFuncNode in processMethodDefinition");
        return;
    }
    
    // std::cout << "Processing method '" << node->func_name << "' in type '" << typeName << "'" << std::endl;
    
    std::vector<std::shared_ptr<TypeInfo>> paramTypes;
    std::vector<std::string> paramNames;
    
    if (!node->args) {
        // std::cout << "Method '" << node->func_name << "' has null args, treating as no parameters" << std::endl;
    } else if (node->args->children.empty()) {
        // std::cout << "Method '" << node->func_name << "' has no parameters" << std::endl;
    } else {
        for (auto* param : node->args->children) {
            if (!param) {
                addError("Null parameter in method '" + node->func_name + "' in type '" + typeName + "'");
                continue;
            }
            paramNames.push_back(param->id_name);
            
            std::shared_ptr<TypeInfo> paramType = Context::numberType;
            if (!param->id_type.empty()) {
                auto explicitType = context->getType(param->id_type);
                if (explicitType) {
                    paramType = explicitType;
                }
            }
            paramTypes.push_back(paramType);
        }
    }
    
    Context* methodContext = createMethodContext(MethodInfo(node->func_name, nullptr, paramTypes, paramNames, typeName), context);
    
    for (size_t i = 0; i < paramNames.size(); i++) {
        methodContext->defineVar(paramNames[i], paramTypes[i]);
    }
    
    if (node->body) {
        node->body->accept(this, methodContext);
    }
    
    MethodInfo method(node->func_name, node->body->inferredType, paramTypes, paramNames, typeName, false, 0);
    
    methodsByType[typeName].push_back(method);
    
    // std::cout << "Added method '" << node->func_name << "' to type '" << typeName << "'" << std::endl;
}

void SymbolCollectorVisitor::processAttributeDefinition(VarAssign* node, const std::string& typeName, Context* context) {
    if (!node || !node->var_id) {
        addError("Null VarAssign or var_id in processAttributeDefinition");
        return;
    }
    
    // std::cout << "Processing attribute '" << node->var_id->id_name << "' in type '" << typeName << "'" << std::endl;
    
    std::shared_ptr<TypeInfo> attrType = Context::numberType; // Default type
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
    
    AttributeInfo attr(node->var_id->id_name, attrType, typeName, false, 0);
    
    attributesByType[typeName].push_back(attr);
    
    node->var_id->inferredType = attrType;
    node->inferredType = attrType;
    
    // std::cout << "Added attribute '" << node->var_id->id_name << "' to type '" << typeName << "'" << std::endl;
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
            
            if (!valueType->isCompatibleWith(declaredType) && !valueType->isSubtypeOf(declaredType)) {
                addError("Type mismatch: Cannot assign value of type '" + valueType->name + 
                        "' to variable '" + node->var_id->id_name + "' of type '" + declaredType->name + "'");
                return;
            }
        }
        
        std::shared_ptr<TypeInfo> varType = declaredType ? declaredType : valueType;
        
        context->defineVar(node->var_id->id_name, varType);
        
        node->var_id->inferredType = varType;
        node->inferredType = varType;
        
        VariableInfo varInfo(node->var_id->id_name, varType, scope, false, 0);
        globalVariables.push_back(varInfo);
        
        // std::cout << "Added variable '" << node->var_id->id_name << "' with type '" << varType->name << "' in scope '" << scope << "'" << std::endl;
    }
}

void SymbolCollectorVisitor::processGlobalFunction(AssignFuncNode* node, Context* context) {
    if (node) {
        // std::cout << "Processing global function '" << node->func_name << "'" << std::endl;
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

Context* SymbolCollectorVisitor::createMethodContext(const MethodInfo& method, Context* parentContext) {
    Context* methodContext = parentContext->createChildContext();
    
    if (!method.ownerType.empty()) {
        auto selfType = globalContext->getType(method.ownerType);
        if (selfType) {
            methodContext->defineVar("self", selfType);
        }
    }
    
    return methodContext;
}

const std::unordered_map<std::string, std::vector<MethodInfo>>& 
SymbolCollectorVisitor::getMethodsByType() const {
    return methodsByType;
}

const std::unordered_map<std::string, std::vector<AttributeInfo>>& 
SymbolCollectorVisitor::getAttributesByType() const {
    return attributesByType;
}

const std::vector<VariableInfo>& SymbolCollectorVisitor::getGlobalVariables() const {
    return globalVariables;
}

std::vector<MethodInfo> SymbolCollectorVisitor::getMethodsForType(const std::string& typeName, bool includeInherited) const {
    std::vector<MethodInfo> result;
    
    auto it = methodsByType.find(typeName);
    if (it != methodsByType.end()) {
        result = it->second;
    }
    
    if (includeInherited && globalContext) {
        auto typeInfo = globalContext->getType(typeName);
        if (typeInfo && typeInfo->typeDef && typeInfo->typeDef->parentType) {
            auto parentMethods = getMethodsForType(typeInfo->typeDef->parentType->name, true);
            
            for (const auto& parentMethod : parentMethods) {
                bool isOverridden = false;
                for (const auto& currentMethod : result) {
                    if (currentMethod.name == parentMethod.name && 
                        currentMethod.paramTypes.size() == parentMethod.paramTypes.size()) {
                        isOverridden = true;
                        break;
                    }
                }
                if (!isOverridden) {
                    result.push_back(parentMethod);
                }
            }
        }
    }
    
    return result;
}

std::vector<AttributeInfo> SymbolCollectorVisitor::getAttributesForType(const std::string& typeName, bool includeInherited) const {
    std::vector<AttributeInfo> result;

    auto it = attributesByType.find(typeName);
    if (it != attributesByType.end()) {
        result = it->second;
    }
    
    if (includeInherited && globalContext) {
        auto typeInfo = globalContext->getType(typeName);
        if (typeInfo && typeInfo->typeDef && typeInfo->typeDef->parentType) {
            auto parentAttributes = getAttributesForType(typeInfo->typeDef->parentType->name, true);
            
            for (const auto& parentAttribute : parentAttributes) {
                bool isOverridden = false;
                for (const auto& currentAttribute : result) {
                    if (currentAttribute.name == parentAttribute.name) {
                        isOverridden = true;
                        break;
                    }
                }
                if (!isOverridden) {
                    result.push_back(parentAttribute);
                }
            }
        }
    }
    
    return result;
}

MethodInfo* SymbolCollectorVisitor::findMethod(const std::string& typeName, const std::string& methodName, 
                      const std::vector<std::shared_ptr<TypeInfo>>& paramTypes) const {
    auto it = methodsByType.find(typeName);
    if (it != methodsByType.end()) {
        for (auto& method : it->second) {
            if (method.name == methodName && method.paramTypes.size() == paramTypes.size()) {
                return const_cast<MethodInfo*>(&method);
            }
        }
    }
    
    if (globalContext) {
        auto typeInfo = globalContext->getType(typeName);
        if (typeInfo && typeInfo->typeDef && typeInfo->typeDef->parentType) {
            return findMethod(typeInfo->typeDef->parentType->name, methodName, paramTypes);
        }
    }
    
    return nullptr;
}

AttributeInfo* SymbolCollectorVisitor::findAttribute(const std::string& typeName, const std::string& attrName) const {
    auto it = attributesByType.find(typeName);
    if (it != attributesByType.end()) {
        for (auto& attr : it->second) {
            if (attr.name == attrName) {
                return const_cast<AttributeInfo*>(&attr);
            }
        }
    }
    
    if (globalContext) {
        auto typeInfo = globalContext->getType(typeName);
        if (typeInfo && typeInfo->typeDef && typeInfo->typeDef->parentType) {
            return findAttribute(typeInfo->typeDef->parentType->name, attrName);
        }
    }
    
    return nullptr;
}
