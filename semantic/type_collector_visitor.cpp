#include "type_collector_visitor.h"
#include "../Ast/ast.hpp"
#include <iostream>
#include <algorithm>

TypeCollectorVisitor::TypeCollectorVisitor() : globalContext(nullptr) {
    // Initialize with built-in types will be called when context is available
}

void TypeCollectorVisitor::visit(ASTNode* node, Context* context) {
    if (node == nullptr) {
        addError("Null AST node encountered");
        return;
    }
    
    // Store global context reference
    if (!globalContext) {
        globalContext = context;
        registerBuiltinTypes(context);
    }
    
    node->accept(this, context);
}

void TypeCollectorVisitor::visit(TypeDeclNode* node, Context* context) {
    if (node == nullptr) {
        addError("Null TypeDeclNode encountered");
        return;
    }
    
    // std::cout << "TypeCollector: Processing type declaration '" << node->id->id_name << "'" << std::endl;
    processTypeDeclaration(node, context);
    
    // Visit children for nested types (if any)
    if (node->args) {
        node->args->accept(this, context);
    }
    
    // Note: We don't visit the body here as that's for the symbol collector
    // We only care about the type definition itself
}

void TypeCollectorVisitor::visit(ProgramNode* node, Context* context) {
    if (node == nullptr) {
        addError("Null ProgramNode encountered");
        return;
    }
    
    // std::cout << "TypeCollector: Starting type collection pass" << std::endl;
    
    if (node->getNode()) {
        node->getNode()->accept(this, context);
    }
    
    // Validate all inheritance chains after collection
    for (const auto& pair : inheritanceMap) {
        validateInheritanceChain(pair.first);
    }
    
    // std::cout << "TypeCollector: Collected " << collectedTypes.size() << " types" << std::endl;
}

void TypeCollectorVisitor::visit(ASTNodeVector* node, Context* context) {
    if (node == nullptr) {
        addError("Null ASTNodeVector encountered");
        return;
    }
    
    // Process all children, looking for type declarations
    for (auto* child : node->children) {
        if (child) {
            child->accept(this, context);
        }
    }
}

void TypeCollectorVisitor::visit(BlockNode* node, Context* context) {
    if (node == nullptr) return;
    
    // Visit all children looking for type declarations
    for (auto* child : node->children) {
        if (child) {
            child->accept(this, context);
        }
    }
}

// For nodes that don't contain type declarations, we provide empty implementations
void TypeCollectorVisitor::visit(FloatNode* node, Context* context) {
    // No type declarations in literal nodes
}

void TypeCollectorVisitor::visit(BoolNode* node, Context* context) {
    // No type declarations in literal nodes
}

void TypeCollectorVisitor::visit(StringNode* node, Context* context) {
    // No type declarations in literal nodes
}

void TypeCollectorVisitor::visit(UnaryOpNode* node, Context* context) {
    if (node && node->node) {
        node->node->accept(this, context);
    }
}

void TypeCollectorVisitor::visit(BinOpNode* node, Context* context) {
    if (node) {
        if (node->left) node->left->accept(this, context);
        if (node->right) node->right->accept(this, context);
    }
}

void TypeCollectorVisitor::visit(FunctionCallNode* node, Context* context) {
    if (node && node->argument) {
        node->argument->accept(this, context);
    }
}

void TypeCollectorVisitor::visit(IDNode* node, Context* context) {
    // No type declarations in ID nodes
}

void TypeCollectorVisitor::visit(ArgsList* node, Context* context) {
    // Args list doesn't contain type declarations, just parameter names
}

void TypeCollectorVisitor::visit(AssignFuncNode* node, Context* context) {
    // Function definitions don't create new types, but may contain type declarations in body
    if (node && node->body) {
        node->body->accept(this, context);
    }
}

void TypeCollectorVisitor::visit(LetAssign* node, Context* context) {
    if (node && node->body) {
        node->body->accept(this, context);
    }
}

void TypeCollectorVisitor::visit(VarAssign* node, Context* context) {
    if (node && node->value) {
        node->value->accept(this, context);
    }
}

void TypeCollectorVisitor::visit(NewTypeNode* node, Context* context) {
    // Type instantiation doesn't declare new types
    for (auto* expr : node->expr_list) {
        if (expr) expr->accept(this, context);
    }
}

void TypeCollectorVisitor::visit(VarAssignType* node, Context* context) {
    if (node && node->body) {
        node->body->accept(this, context);
    }
}

void TypeCollectorVisitor::visit(VarAssignList* node, Context* context) {
    if (node) {
        for (auto* assign : node->assigns) {
            if (assign) assign->accept(this, context);
        }
    }
}

void TypeCollectorVisitor::visit(Conditional* node, Context* context) {
    if (node) {
        if (node->bool_expr) node->bool_expr->accept(this, context);
        if (node->if_body) node->if_body->accept(this, context);
        if (node->else_body) node->else_body->accept(this, context);
    }
}

void TypeCollectorVisitor::visit(BoolExprNode* node, Context* context) {
    if (node && node->expr) {
        node->expr->accept(this, context);
    }
}

void TypeCollectorVisitor::visit(WhileNode* node, Context* context) {
    if (node) {
        if (node->bool_expr) node->bool_expr->accept(this, context);
        if (node->body) node->body->accept(this, context);
    }
}

void TypeCollectorVisitor::visit(VarDesAssign* node, Context* context) {
    if (node && node->value) {
        node->value->accept(this, context);
    }
}

void TypeCollectorVisitor::visit(ForNode* node, Context* context) {
    if (node) {
        if (node->group) node->group->accept(this, context);
        if (node->body) node->body->accept(this, context);
    }
}

void TypeCollectorVisitor::visit(ExprsList* node, Context* context) {
    if (node) {
        for (auto* expr : node->children) {
            if (expr) expr->accept(this, context);
        }
    }
}

void TypeCollectorVisitor::visit(AccessNode* node, Context* context) {
    if (node && node->member) {
        node->member->accept(this, context);
    }
}

void TypeCollectorVisitor::visit(TypeAssMember* node, Context* context) {
    // Type assignment members don't declare new types
}

void TypeCollectorVisitor::visit(AttributeMember* node, Context* context) {
    // Attribute members don't declare new types
}

void TypeCollectorVisitor::visit(MethodMember* node, Context* context) {
    if (node) {
        for (auto* arg : node->args) {
            if (arg) arg->accept(this, context);
        }
    }
}

void TypeCollectorVisitor::visit(TypeCastNode* node, Context* context) {
    if (node && node->expr) {
        node->expr->accept(this, context);
    }
}

// Helper method implementations
void TypeCollectorVisitor::processTypeDeclaration(TypeDeclNode* node, Context* context) {
    const std::string& typeName = node->id->id_name;
    
    // Check for type redefinition
    checkTypeRedefinition(typeName, node->line);
    if (hasErrors()) return;
    
    // Determine parent type
    std::string parentName = "";
    if (!node->parents.empty()) {
        parentName = node->parents[0]; // HULK supports single inheritance
        
        // Validate parent type exists (will be checked later in validation phase)
        if (!isValidParentType(parentName, context)) {
            addError("Type error in line " + std::to_string(node->line) + " :Type '" + typeName + "' cannot inherit from undefined type '" + parentName + "'");
            return;
        }
        
        // Check for circular inheritance
        if (detectCircularInheritance(typeName, parentName)) {
            addError("Type error in line " + std::to_string(node->line) + " :Circular inheritance detected: type '" + typeName + "' cannot inherit from '" + parentName + "'");
            return;
        }
        
        inheritanceMap[typeName] = parentName;
    }
    
    // Create type definition
    auto typeDef = createTypeDef(typeName, parentName);
    if (!typeDef) {
        throw std::runtime_error("Failed to create type definition for '" + typeName + "'");
        return;
    }
    
    // Store collected type
    collectedTypes[typeName] = typeDef;
    
    // Register type in context
    if (!context->defineType(typeName, typeDef)) {
        throw std::runtime_error("Failed to register type '" + typeName + "' in context");
        return;
    }
    
    // std::cout << "TypeCollector: Successfully collected type '" << typeName << "'";
    if (!parentName.empty()) {
        // std::cout << " inheriting from '" << parentName << "'";
    }
    // std::cout << std::endl;
}

void TypeCollectorVisitor::registerBuiltinTypes(Context* context) {
    context->initializeBuiltinTypes();
    // std::cout << "TypeCollector: Built-in types registered" << std::endl;
}

bool TypeCollectorVisitor::detectCircularInheritance(const std::string& typeName, const std::string& parentName) {
    if (typeName == parentName) {
        return true; // Direct self-inheritance
    }
    
    // Check if parent eventually inherits from this type
    std::string current = parentName;
    std::unordered_set<std::string> visited;
    
    while (!current.empty() && visited.find(current) == visited.end()) {
        visited.insert(current);
        
        auto it = inheritanceMap.find(current);
        if (it != inheritanceMap.end()) {
            if (it->second == typeName) {
                return true; // Circular inheritance found
            }
            current = it->second;
        } else {
            break; // No more parents in chain
        }
    }
    
    return false;
}

void TypeCollectorVisitor::validateInheritanceChain(const std::string& typeName) {
    std::unordered_set<std::string> visited;
    std::string current = typeName;
    
    while (!current.empty() && visited.find(current) == visited.end()) {
        visited.insert(current);
        
        auto it = inheritanceMap.find(current);
        if (it != inheritanceMap.end()) {
            current = it->second;
            
            // Check if parent type was collected
            if (collectedTypes.find(current) == collectedTypes.end() && 
                current != "Object" && // Built-in Object type
                !current.empty()) {
                addError("Type '" + typeName + "' inherits from undefined type '" + current + "'");
                return;
            }
        } else {
            break;
        }
    }
}

std::shared_ptr<TypeDef> TypeCollectorVisitor::createTypeDef(const std::string& typeName, const std::string& parentName) {
    auto typeDef = std::make_shared<TypeDef>(typeName);
    
    if (!parentName.empty()) {
        // Set parent type info (will be fully resolved later)
        typeDef->parentType = std::make_shared<TypeInfo>(parentName, TypeKind::CLASS);
    } else if (typeName != "Object") {
        // Default to Object as parent for all types except Object itself
        typeDef->parentType = Context::objectType;
    }
    
    return typeDef;
}

bool TypeCollectorVisitor::isValidParentType(const std::string& parentName, Context* context) {
    // Check if it's a built-in type
    if (parentName == "Object" || parentName == "String" || parentName == "Number" || parentName == "Boolean") {
        return true;
    }
    
    // Check if it's already been collected
    if (collectedTypes.find(parentName) != collectedTypes.end()) {
        return true;
    }
    
    // Check if it exists in context
    return context->getType(parentName) != nullptr;
}

void TypeCollectorVisitor::checkTypeRedefinition(const std::string& typeName, int line) {
    if (collectedTypes.find(typeName) != collectedTypes.end()) {
        addError("Type '" + typeName + "' is already defined (line " + std::to_string(line) + ")");
    }
}

// Error handling methods
void TypeCollectorVisitor::addError(const std::string& error) {
    errors.push_back(error);
    std::cerr << "TypeCollector Error: " << error << std::endl;
}

bool TypeCollectorVisitor::hasErrors() const {
    return !errors.empty();
}

const std::vector<std::string>& TypeCollectorVisitor::getErrors() const {
    return errors;
}

void TypeCollectorVisitor::printErrors() const {
    if (hasErrors()) {
        // std::cout << "\n=== TYPE COLLECTION ERRORS ===" << std::endl;
        for (size_t i = 0; i < errors.size(); i++) {
            // std::cout << "Error " << (i + 1) << ": " << errors[i] << std::endl;
        }
        // std::cout << "\nTotal type collection errors: " << errors.size() << std::endl;
    }
}

// Getter methods
const std::unordered_map<std::string, std::shared_ptr<TypeDef>>& TypeCollectorVisitor::getCollectedTypes() const {
    return collectedTypes;
}

const std::unordered_map<std::string, std::string>& TypeCollectorVisitor::getInheritanceMap() const {
    return inheritanceMap;
}

bool TypeCollectorVisitor::isTypeCollected(const std::string& typeName) const {
    return collectedTypes.find(typeName) != collectedTypes.end();
}

std::shared_ptr<TypeDef> TypeCollectorVisitor::getTypeDef(const std::string& typeName) const {
    auto it = collectedTypes.find(typeName);
    return (it != collectedTypes.end()) ? it->second : nullptr;
}