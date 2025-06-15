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
        std::cout << "\n=== Symbol Collection Errors ===" << std::endl;
        for (const auto& error : errors) {
            std::cout << "Error: " << error << std::endl;
        }
    }
}

// Visitor interface implementation
void SymbolCollectorVisitor::visit(ASTNode* node, Context* context) {
    if (node) {
        // Default implementation - could dispatch to specific handlers
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
        // Basic type inference for binary operations
        if (node->op == "+" || node->op == "-" || node->op == "*" || node->op == "/") {
            // Assume numeric operations return a number
            node->inferredType = Context::numberType;
        } else if (node->op == "&" || node->op == "|" || node->op == "==" || node->op == "!=" || node->op == "is" || node->op == "<" || node->op == ">" || node->op == "<=" || node->op == ">=") {
            // Logical operations return a boolean
            node->inferredType = Context::boolType;
        } else if (node->op == "@" || node->op == "@@") {
            // Equality checks return a boolean
            node->inferredType = Context::stringType;
        } else {
            // Default to void for unsupported operations
            node->inferredType = Context::voidType;
        }
    }
}

void SymbolCollectorVisitor::visit(FunctionCallNode* node, Context* context) {
    if (!node) return;
    
    // Process argument first
    if (node->argument) {
        node->argument->accept(this, context);
    }
    
    // For now, most function calls return void unless we can determine otherwise
    // In a more complete implementation, we would look up function signatures
    std::shared_ptr<TypeInfo> returnType = Context::voidType;
    
    // Special handling for built-in functions
    if (node->func_name == "print") {
        returnType = Context::voidType;
    } else if (node->func_name == "sin" || node->func_name == "cos" || 
               node->func_name == "sqrt" || node->func_name == "exp" || 
               node->func_name == "log" || node->func_name == "rand") {
        // These functions return a number
        returnType = Context::numberType;
    }
    // Could add more built-in function types here
    
    node->inferredType = returnType;
}

void SymbolCollectorVisitor::visit(IDNode* node, Context* context) {
    if (node && context) {
         if (context->isDefined(node->id_name)) {
            node->inferredType = context->getVarType(node->id_name);
            return;
        }

        if (node->id_name == "PI" || node->id_name == "E") {
            // Special handling for constants
            node->inferredType = Context::numberType;
            return;
        }
        
        auto varType = context->getVarType(node->id_name);
        if (varType) {
            node->inferredType = varType;
        } else {
            node->inferredType = Context::numberType;
        }
    }
}

void SymbolCollectorVisitor::visit(BlockNode* node, Context* context) {
    if (!node) return;
    
    std::shared_ptr<TypeInfo> lastType = Context::voidType;
    
    for (auto* child : node->children) {
        if (child) {
            child->accept(this, context);
            // The block's type is the type of the last expression
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
    
    // ArgsList doesn't have a meaningful type, use void
    node->inferredType = Context::voidType;
}

void SymbolCollectorVisitor::visit(AssignFuncNode* node, Context* context) {
    if (!node) return;

    for (auto* arg: node->args->children) {
        arg->accept(this, context);
        arg->inferredType = context->getVarType(arg->id_name);
    }

    node->body->accept(this, context);
    node->inferredType = node->body->inferredType;

    if (!currentTypeName.empty()) {
       processMethodDefinition(node, currentTypeName, context);
    }
}

void SymbolCollectorVisitor::visit(LetAssign* node, Context* context) {
    if (!node) return;
    
    // Process variable assignments in let
    for (auto* assign : node->assigns) {
        if (assign) {
            assign->accept(this, context);
        }
    }
    
    // Process body and use its type as the let expression's type
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
    
    // Only treat VarAssign as type attribute if we're in a type but NOT inside a method body
    if (!currentTypeName.empty() && !insideMethod) {
        std::cout << "  -> Processing as type attribute" << std::endl;
        processAttributeDefinition(node, currentTypeName, context);
    } 
    // else {
    //     std::cout << "  -> Processing as variable definition" << std::endl;
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
    
    // Process type instantiation
    if (node->new_type) {
        node->new_type->accept(this, context);
    }
    
    // Process body and use its type
    if (node->body) {
        node->body->accept(this, context);
        node->inferredType = node->body->inferredType;
    } else {
        // If no body, the type is the instantiated type
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
        
        // Type inference for conditional
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
    
    // Process the value being assigned
    if (node->value) {
        node->value->accept(this, context);
    }
    
    // Process the variable being assigned to
    if (node->id) {
        node->id->accept(this, context);
    }
    
    // The assignment expression takes the type of the assigned value
    if (node->value && node->value->inferredType) {
        node->inferredType = node->value->inferredType;
    } else {
        node->inferredType = Context::voidType;
    }
}

void SymbolCollectorVisitor::visit(ForNode* node, Context* context) {
    if (!node) return;
    
    // Process the iterable expression
    if (node->group) {
        node->group->accept(this, context);
    }
    
    // Process the loop body
    if (node->body) {
        node->body->accept(this, context);
    }
    
    // For loops don't have a meaningful return type, use void
    node->inferredType = Context::voidType;
}

void SymbolCollectorVisitor::visit(TypeDeclNode* node, Context* context) {
    if (!node) return;
    
    std::cout << "SymbolCollector: Processing type body for '" << node->id->id_name << "'" << std::endl;
    currentTypeName = node->id->id_name;
    
    // Process type body
    for (auto* element : node->body) {
        if (element) {
            element->accept(this, context);
        }
    }
    
    // After processing the type's own members, ensure inherited members are accessible
    // This helps with debugging and ensures completeness
    if (globalContext) {
        auto typeInfo = globalContext->getType(node->id->id_name);
        if (typeInfo && typeInfo->typeDef && typeInfo->typeDef->parentType) {
            std::cout << "Type '" << node->id->id_name << "' inherits from '" 
                     << typeInfo->typeDef->parentType->name << "'" << std::endl;
            
            // Get all inherited attributes (for debugging)
            auto inheritedAttrs = getAttributesForType(typeInfo->typeDef->parentType->name, true);
            std::cout << "Inherited " << inheritedAttrs.size() << " attributes from parent types" << std::endl;
            
            // Get all inherited methods (for debugging)
            auto inheritedMethods = getMethodsForType(typeInfo->typeDef->parentType->name, true);
            std::cout << "Inherited " << inheritedMethods.size() << " methods from parent types" << std::endl;
        }
    }
    
    currentTypeName.clear();
    
    // Type declarations return the declared type
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


    
    // Get the variable type first
    // std::shared_ptr<TypeInfo> varType = context->getVarType(node->var_name);
    // if (!varType) {
    //     addError("Unknown variable '" + node->var_name + "' in access expression");
    //     return;
    // }
    
    // // Process the member access
    // if (node->member) {
    //     std::string memberName = node->member->get_name();
        
    //     if (node->member->get_form() == TypeAssMember::Form::Attribute) {
    //         // Look up the attribute in the variable's type (including inherited attributes)
    //         auto* attr = findAttribute(varType->name, memberName);
    //         if (attr && attr->type) {
    //             node->inferredType = attr->type;
    //         } else {
    //             addError("Attribute '" + memberName + "' not found in type '" + varType->name + "' or its parent types");
    //             node->inferredType = Context::numberType; // Default fallback
    //         }
    //     } else if (node->member->get_form() == TypeAssMember::Form::Method) {
    //         // For method access, we typically return void unless we can determine the return type
    //         // In a complete implementation, we would look up the method signature
    //         node->inferredType = Context::voidType;
    //     } else {
    //         node->inferredType = varType;
    //     }
    // } else {
    //     node->inferredType = varType;
    // }
}

void SymbolCollectorVisitor::visit(TypeAssMember* node, Context* context) {
    if (!node) return;
    
    // This is a base class, actual processing happens in derived classes
    // Set a default type
    node->inferredType = Context::voidType;
}

void SymbolCollectorVisitor::visit(AttributeMember* node, Context* context) {
    if (!node) return;
    
    // For attribute access, we need to determine the attribute's type
    // This would typically be done by looking up the attribute in the current object's type
    // For now, set a default type
    node->inferredType = Context::numberType; // Default attribute type
}

void SymbolCollectorVisitor::visit(MethodMember* node, Context* context) {
    if (!node) return;
    
    // Process method arguments first
    for (auto* arg : node->args) {
        if (arg) {
            arg->accept(this, context);
        }
    }
    
    // For method calls, we would need to look up the method signature
    // For now, assume methods return a default type
    node->inferredType = Context::voidType; // Default method return type
}

void SymbolCollectorVisitor::visit(TypeCastNode* node, Context* context) {
    if (!node) {
        addError("Null TypeCastNode");
        return;
    }

    // Visit the expression being cast first
    node->expr->accept(this, context);

    // Get the source type from the expression
    std::shared_ptr<TypeInfo> sourceType = node->expr->inferredType ? node->expr->inferredType : context->getType(node->target_type);
    if (!sourceType) {
        addError("Cannot determine type of expression being cast");
        return;
    }
    
    // Get the target type
    std::shared_ptr<TypeInfo> targetType = context->getType(node->target_type);
    if (!targetType) {
        addError("Unknown target type in cast: " + node->target_type);
        return;
    }
    
    // Helper function to check if one type is a subclass of another by name
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
    
    // Validate that the cast is legal for inheritance relationships
    // Allow upcasting (subtype to supertype): C -> B -> A
    // Allow downcasting (supertype to subtype): A -> B -> C (runtime check needed)
    bool isValidCast = false;
    
    // Same type cast is always valid
    if (sourceType->name == targetType->name) {
        isValidCast = true;
    }
    // Upcast: source is subtype of target (safe cast)
    else if (isSubclassOfByName(sourceType->name, targetType->name)) {
        isValidCast = true;
    }
    // Downcast: target is subtype of source (needs runtime check)
    else if (isSubclassOfByName(targetType->name, sourceType->name)) {
        isValidCast = true;
    }
    // Allow general compatibility for built-in types
    else if (sourceType->isCompatibleWith(targetType)) {
        isValidCast = true;
    }
    
    if (!isValidCast) {
        addError("Invalid cast: Cannot cast from type '" + sourceType->name + 
                "' to type '" + targetType->name + "' - types are not related through inheritance");
        return;
    }
    
    // Set the inferred type to the target type
    cout << "Casting from type '" << sourceType->name 
         << "' to type '" << targetType->name << "'" << std::endl;
    node->inferredType = targetType;
}

// Helper methods
void SymbolCollectorVisitor::processMethodDefinition(AssignFuncNode* node, const std::string& typeName, Context* context) {
    if (!node) {
        addError("Null AssignFuncNode in processMethodDefinition");
        return;
    }
    
    std::cout << "Processing method '" << node->func_name << "' in type '" << typeName << "'" << std::endl;
    
    // Collect parameter types and names
    std::vector<std::shared_ptr<TypeInfo>> paramTypes;
    std::vector<std::string> paramNames;
    
    // Check if args is null
    if (!node->args) {
        std::cout << "Method '" << node->func_name << "' has null args, treating as no parameters" << std::endl;
    } else if (node->args->children.empty()) {
        std::cout << "Method '" << node->func_name << "' has no parameters" << std::endl;
    } else {
        for (auto* param : node->args->children) {
            if (!param) {
                addError("Null parameter in method '" + node->func_name + "' in type '" + typeName + "'");
                continue;
            }
            paramNames.push_back(param->id_name);
            
            // Get parameter type (default to number if not specified)
            std::shared_ptr<TypeInfo> paramType = Context::numberType;
            if (!param->id_type.empty() && param->id_type != "none") {
                auto explicitType = context->getType(param->id_type);
                if (explicitType) {
                    paramType = explicitType;
                }
            }
            paramTypes.push_back(paramType);
        }
    }
    
    // Create method context and process body
    Context* methodContext = createMethodContext(MethodInfo(node->func_name, nullptr, paramTypes, paramNames, typeName), context);
    
    // Define parameters in method context
    for (size_t i = 0; i < paramNames.size(); i++) {
        methodContext->defineVar(paramNames[i], paramTypes[i]);
    }
    
    // Process method body
    if (node->body) {
        node->body->accept(this, methodContext);
    }
    
    // Determine return type
    std::shared_ptr<TypeInfo> returnType = Context::voidType;
    if (!node->func_type.empty() && node->func_type != "none") {
        auto explicitReturnType = context->getType(node->func_type);
        if (explicitReturnType) {
            returnType = explicitReturnType;
        }
    } else if (node->body && node->body->inferredType) {
        returnType = node->body->inferredType;
    }
    
    // Create method info
    MethodInfo method(node->func_name, returnType, paramTypes, paramNames, typeName, false, 0);
    
    // Add to methods collection
    methodsByType[typeName].push_back(method);
    
    std::cout << "Added method '" << node->func_name << "' to type '" << typeName << "'" << std::endl;
}

void SymbolCollectorVisitor::processAttributeDefinition(VarAssign* node, const std::string& typeName, Context* context) {
    if (!node || !node->var_id) {
        addError("Null VarAssign or var_id in processAttributeDefinition");
        return;
    }
    
    std::cout << "Processing attribute '" << node->var_id->id_name << "' in type '" << typeName << "'" << std::endl;
    
    // Infer type from value
    std::shared_ptr<TypeInfo> attrType = Context::numberType; // Default type
    if (node->value) {
        node->value->accept(this, context);
        if (node->value->inferredType) {
            attrType = node->value->inferredType;
        }
    }
    
    // Check for explicit type annotation
    if (!node->var_id->id_type.empty() && node->var_id->id_type != "none") {
        auto explicitType = context->getType(node->var_id->id_type);
        if (explicitType) {
            attrType = explicitType;
        }
    }
    
    // Create attribute info
    AttributeInfo attr(node->var_id->id_name, attrType, typeName, false, 0);
    
    // Add to attributes collection
    attributesByType[typeName].push_back(attr);
    
    // Set inferred types
    node->var_id->inferredType = attrType;
    node->inferredType = attrType;
    
    std::cout << "Added attribute '" << node->var_id->id_name << "' to type '" << typeName << "'" << std::endl;
}

void SymbolCollectorVisitor::processVariableDefinition(VarAssign* node, const std::string& scope, Context* context) {
    if (node && node->var_id && node->value) {
        // First, visit the value expression to infer its type
        node->value->accept(this, context);
        
        std::shared_ptr<TypeInfo> declaredType = nullptr;
        std::shared_ptr<TypeInfo> valueType = node->value->inferredType ? node->value->inferredType : Context::numberType;
        
        // Check if variable has explicit type annotation
        if (!node->var_id->id_type.empty() && node->var_id->id_type != "none") {
            declaredType = context->getType(node->var_id->id_type);
            if (!declaredType) {
                addError("Variable '" + node->var_id->id_name + "' declared with unknown type '" + node->var_id->id_type + "'");
                return;
            }
            
            // Validate that the assigned value type is compatible with the declared type
            // For assignment, the value type must be a subtype of (or same as) the declared type
            if (!valueType->isCompatibleWith(declaredType) && !valueType->isSubtypeOf(declaredType)) {
                addError("Type mismatch: Cannot assign value of type '" + valueType->name + 
                        "' to variable '" + node->var_id->id_name + "' of type '" + declaredType->name + "'");
                return;
            }
        }
        
        // Use declared type if available, otherwise use inferred type from value
        std::shared_ptr<TypeInfo> varType = declaredType ? declaredType : valueType;
        
        context->defineVar(node->var_id->id_name, varType);
        
        // Set inferred types on AST nodes
        node->var_id->inferredType = varType;
        node->inferredType = varType;
        
        // Add to global variables collection
        VariableInfo varInfo(node->var_id->id_name, varType, scope, false, 0);
        globalVariables.push_back(varInfo);
        
        std::cout << "Added variable '" << node->var_id->id_name << "' with type '" << varType->name << "' in scope '" << scope << "'" << std::endl;
    }
}

void SymbolCollectorVisitor::processGlobalFunction(AssignFuncNode* node, Context* context) {
    if (node) {
        std::cout << "Processing global function '" << node->func_name << "'" << std::endl;
        // Add to global context or handle global functions as needed
    }
}

Context* SymbolCollectorVisitor::createMethodContext(const MethodInfo& method, Context* parentContext) {
    Context* methodContext = parentContext->createChildContext();
    
    // Define 'self' if this is a method in a type
    if (!method.ownerType.empty()) {
        auto selfType = globalContext->getType(method.ownerType);
        if (selfType) {
            methodContext->defineVar("self", selfType);
        }
    }
    
    return methodContext;
}

// Getters for collected symbols
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

// Query methods
std::vector<MethodInfo> SymbolCollectorVisitor::getMethodsForType(const std::string& typeName, bool includeInherited) const {
    std::vector<MethodInfo> result;
    
    // Add methods from the current type
    auto it = methodsByType.find(typeName);
    if (it != methodsByType.end()) {
        result = it->second;
    }
    
    // If inheritance is requested, traverse up the inheritance chain
    if (includeInherited && globalContext) {
        auto typeInfo = globalContext->getType(typeName);
        if (typeInfo && typeInfo->typeDef && typeInfo->typeDef->parentType) {
            auto parentMethods = getMethodsForType(typeInfo->typeDef->parentType->name, true);
            
            // Add parent methods that are not overridden in the current type
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
    
    // Add attributes from the current type
    auto it = attributesByType.find(typeName);
    if (it != attributesByType.end()) {
        result = it->second;
    }
    
    // If inheritance is requested, traverse up the inheritance chain
    if (includeInherited && globalContext) {
        auto typeInfo = globalContext->getType(typeName);
        if (typeInfo && typeInfo->typeDef && typeInfo->typeDef->parentType) {
            auto parentAttributes = getAttributesForType(typeInfo->typeDef->parentType->name, true);
            
            // Add parent attributes that are not overridden in the current type
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
    // First check in the current type
    auto it = methodsByType.find(typeName);
    if (it != methodsByType.end()) {
        for (auto& method : it->second) {
            if (method.name == methodName && method.paramTypes.size() == paramTypes.size()) {
                // Simple check - could be enhanced with type compatibility
                return const_cast<MethodInfo*>(&method);
            }
        }
    }
    
    // If not found, check parent types
    if (globalContext) {
        auto typeInfo = globalContext->getType(typeName);
        if (typeInfo && typeInfo->typeDef && typeInfo->typeDef->parentType) {
            return findMethod(typeInfo->typeDef->parentType->name, methodName, paramTypes);
        }
    }
    
    return nullptr;
}

AttributeInfo* SymbolCollectorVisitor::findAttribute(const std::string& typeName, const std::string& attrName) const {
    // First check in the current type
    auto it = attributesByType.find(typeName);
    if (it != attributesByType.end()) {
        for (auto& attr : it->second) {
            if (attr.name == attrName) {
                return const_cast<AttributeInfo*>(&attr);
            }
        }
    }
    
    // If not found, check parent types
    if (globalContext) {
        auto typeInfo = globalContext->getType(typeName);
        if (typeInfo && typeInfo->typeDef && typeInfo->typeDef->parentType) {
            return findAttribute(typeInfo->typeDef->parentType->name, attrName);
        }
    }
    
    return nullptr;
}
