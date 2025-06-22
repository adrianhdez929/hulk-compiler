#include "type_collector_visitor.h"
#include "../Ast/ast.hpp"
#include <iostream>
#include <algorithm>

TypeCollectorVisitor::TypeCollectorVisitor() : globalContext(nullptr) {
}

void TypeCollectorVisitor::visit(ASTNode* node, Context* context) {
    if (node == nullptr) {
        addError("Null AST node encountered");
        return;
    }
    
    if (!globalContext) {
        globalContext = context;
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
    
    if (node->args) {
        node->args->accept(this, context);
    }
}

void TypeCollectorVisitor::visit(ProgramNode* node, Context* context) {
    if (node == nullptr) {
        addError("Null ProgramNode encountered");
        return;
    }
    
    
    if (node->getNode()) {
        node->getNode()->accept(this, context);
    }
    
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
    
    for (auto* child : node->children) {
        if (child) {
            child->accept(this, context);
        }
    }
}

void TypeCollectorVisitor::visit(BlockNode* node, Context* context) {
    if (node == nullptr) return;
    
    for (auto* child : node->children) {
        if (child) {
            child->accept(this, context);
        }
    }
}

void TypeCollectorVisitor::visit(FloatNode* node, Context* context) {
}

void TypeCollectorVisitor::visit(BoolNode* node, Context* context) {
}

void TypeCollectorVisitor::visit(StringNode* node, Context* context) {
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
}

void TypeCollectorVisitor::visit(ArgsList* node, Context* context) {
}

void TypeCollectorVisitor::visit(AssignFuncNode* node, Context* context) {
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
}

void TypeCollectorVisitor::visit(AttributeMember* node, Context* context) {
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

void TypeCollectorVisitor::processTypeDeclaration(TypeDeclNode* node, Context* context) {
    const std::string& typeName = node->id->id_name;
    
    checkTypeRedefinition(typeName, node->line);

    if (!context->defineType(typeName)) {
        throw std::runtime_error("Failed to register type '" + typeName + "' in context");
        return;
    }

    std::shared_ptr<TypeInfo> typeDef = context->getType(typeName);
    
    if (!node->parents.empty()) {
        for (std::string parent : node->parents) {
            if (!isValidParentType(parent, context)) {
                addError("Type error in line " + std::to_string(node->line) + " :Type '" + typeName + "' cannot inherit from undefined type '" + parent + "'");
                return;
            }
            
            if (detectCircularInheritance(typeName, parent)) {
                addError("Type error in line " + std::to_string(node->line) + " :Circular inheritance detected: type '" + typeName + "' cannot inherit from '" + parent + "'");
                return;
            }
            
            inheritanceMap[typeName] = parent;
            std::shared_ptr<TypeInfo> parentType = context->getType(parent);

            if (!parentType) {
                addError("Type error in line " + std::to_string(node->line) + " :Parent type '" + parent + "' is not defined for type '" + typeName + "'");
                return;
            }
            
            typeDef->defineParent(parentType);
            std::cout << "Added parent " << parent << " to type " << typeName << std::endl;
        }
    }
    
    collectedTypes[typeName] = typeDef;
}

bool TypeCollectorVisitor::detectCircularInheritance(const std::string& typeName, const std::string& parentName) {
    if (typeName == parentName) {
        return true; 
    }
    
    std::string current = parentName;
    std::unordered_set<std::string> visited;
    
    while (!current.empty() && visited.find(current) == visited.end()) {
        visited.insert(current);
        
        auto it = inheritanceMap.find(current);
        if (it != inheritanceMap.end()) {
            if (it->second == typeName) {
                return true; 
            }
            current = it->second;
        } else {
            break;
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
            
            if (collectedTypes.find(current) == collectedTypes.end() && 
                current != "Object" && 
                !current.empty()) {
                addError("Type '" + typeName + "' inherits from undefined type '" + current + "'");
                return;
            }
        } else {
            break;
        }
    }
}

bool TypeCollectorVisitor::isValidParentType(const std::string& parentName, Context* context) {
    if (parentName == "Object" || parentName == "String" || parentName == "Number" || parentName == "Boolean") {
        return true;
    }
    
    if (collectedTypes.find(parentName) != collectedTypes.end()) {
        return true;
    }
    
    return context->getType(parentName) != nullptr;
}

void TypeCollectorVisitor::checkTypeRedefinition(const std::string& typeName, int line) {
    if (collectedTypes.find(typeName) != collectedTypes.end()) {
        addError("Type '" + typeName + "' is already defined (line " + std::to_string(line) + ")");
    }
}

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

