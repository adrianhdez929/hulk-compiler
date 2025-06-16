#ifndef TYPE_COLLECTOR_VISITOR_H
#define TYPE_COLLECTOR_VISITOR_H

#include "visitor.h"
#include "context.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <memory>

/**
 * TypeCollectorVisitor - First pass visitor that collects all type definitions in the AST
 * 
 * This visitor performs a pre-order traversal of the AST to:
 * 1. Collect all type declarations (classes/types)
 * 2. Build inheritance hierarchy information
 * 3. Register types in the context for later use
 * 4. Detect type redefinitions and circular inheritance
 */
class TypeCollectorVisitor : public Visitor {
public:
    TypeCollectorVisitor();
    ~TypeCollectorVisitor() = default;

    // Visitor interface implementation
    void visit(ASTNode* node, Context* context) override;
    void visit(FloatNode* node, Context* context) override;
    void visit(BoolNode* node, Context* context) override;
    void visit(StringNode* node, Context* context) override;
    void visit(UnaryOpNode* node, Context* context) override;
    void visit(BinOpNode* node, Context* context) override;
    void visit(FunctionCallNode* node, Context* context) override;
    void visit(IDNode* node, Context* context) override;
    void visit(BlockNode* node, Context* context) override;
    void visit(ArgsList* node, Context* context) override;
    void visit(AssignFuncNode* node, Context* context) override;
    void visit(LetAssign* node, Context* context) override;
    void visit(VarAssign* node, Context* context) override;
    void visit(NewTypeNode* node, Context* context) override;
    void visit(VarAssignType* node, Context* context) override;
    void visit(VarAssignList* node, Context* context) override;
    void visit(Conditional* node, Context* context) override;
    void visit(BoolExprNode* node, Context* context) override;
    void visit(WhileNode* node, Context* context) override;
    void visit(VarDesAssign* node, Context* context) override;
    void visit(ForNode* node, Context* context) override;
    void visit(TypeDeclNode* node, Context* context) override;
    void visit(ASTNodeVector* node, Context* context) override;
    void visit(ExprsList* node, Context* context) override;
    void visit(ProgramNode* node, Context* context) override;
    void visit(AccessNode* node, Context* context) override;
    void visit(TypeAssMember* node, Context* context) override;
    void visit(AttributeMember* node, Context* context) override;
    void visit(MethodMember* node, Context* context) override;
    void visit(TypeCastNode* node, Context* context) override;

    // Error collection methods
    void addError(const std::string& error);
    bool hasErrors() const;
    const std::vector<std::string>& getErrors() const;
    void printErrors() const;

    // Type collection results
    const std::unordered_map<std::string, std::shared_ptr<TypeDef>>& getCollectedTypes() const;
    const std::unordered_map<std::string, std::string>& getInheritanceMap() const;
    
    // Check if a type was successfully collected
    bool isTypeCollected(const std::string& typeName) const;
    
    // Get type definition by name
    std::shared_ptr<TypeDef> getTypeDef(const std::string& typeName) const;
    
    // Get the global context
    Context* getGlobalContext() const { return globalContext; }

private:
    // Error collection
    std::vector<std::string> errors;
    
    // Type collection data structures
    std::unordered_map<std::string, std::shared_ptr<TypeDef>> collectedTypes;
    std::unordered_map<std::string, std::string> inheritanceMap; // child -> parent
    std::unordered_set<std::string> typesBeingProcessed; // For circular inheritance detection
    
    // Helper methods
    void processTypeDeclaration(TypeDeclNode* node, Context* context);
    void registerBuiltinTypes(Context* context);
    bool detectCircularInheritance(const std::string& typeName, const std::string& parentName);
    void validateInheritanceChain(const std::string& typeName);
    std::shared_ptr<TypeDef> createTypeDef(const std::string& typeName, const std::string& parentName = "");
    
    // Inheritance validation
    bool isValidParentType(const std::string& parentName, Context* context);
    void checkTypeRedefinition(const std::string& typeName, int line);
    
    // Context reference for type registration
    Context* globalContext;
};

#endif // TYPE_COLLECTOR_VISITOR_H