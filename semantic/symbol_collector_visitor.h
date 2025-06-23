#ifndef SYMBOL_COLLECTOR_VISITOR_H
#define SYMBOL_COLLECTOR_VISITOR_H

#include "visitor.h"
#include "context.h"
#include "type_collector_visitor.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <memory>

/**
 * SymbolCollectorVisitor - Second pass visitor that collects symbols and assigns types
 * 
 * This visitor performs a detailed traversal of the AST to:
 * 1. Collect all method definitions and their signatures
 * 2. Collect all attribute/property definitions
 * 3. Collect all variable definitions and their types
 * 4. Perform type inference where needed
 * 5. Build complete symbol tables for each type
 * 6. Validate method overrides and attribute access
 */
class SymbolCollectorVisitor : public Visitor {
public:
    SymbolCollectorVisitor(TypeCollectorVisitor* typeCollector);
    SymbolCollectorVisitor(Context* globalCtx);  // Alternative constructor
    ~SymbolCollectorVisitor() = default;

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

private:
    // Error collection
    std::vector<std::string> errors;
    
    // Reference to type collector for type information
    TypeCollectorVisitor* typeCollector;
    TypeCollectorVisitor* typeCollectorVisitor; // Alternative name used in code
        
    // Current context tracking
    std::string currentTypeName;
    std::string currentFunctionName;
    bool insideMethod;  // Track when we're processing method body vs type body
    std::vector<std::string> scopeStack;
    Context* globalContext;
    
    // Helper methods
    void processTypeBody(TypeDeclNode* node, Context* context);
    void processAttributeDefinition(VarAssign* node, const std::string& typeName, Context* context);
    void processVariableDefinition(VarAssign* node, const std::string& scope, Context* context);
    void processGlobalFunction(AssignFuncNode* node, Context* context);
    
    // Type inference helpers
    std::shared_ptr<TypeInfo> inferTypeFromExpression(ASTNode* expr, Context* context);
    std::shared_ptr<TypeInfo> inferTypeFromLiteral(ASTNode* literal);
    std::shared_ptr<TypeInfo> inferTypeFromBinaryOp(BinOpNode* binOp, Context* context);
    std::shared_ptr<TypeInfo> inferTypeFromFunctionCall(FunctionCallNode* funcCall, Context* context);
    
    // Inheritance handling
    void inheritMethodsAndAttributes(const std::string& typeName);
    
    // Scope management
    void enterScope(const std::string& scopeName);
    void exitScope();
    std::string getCurrentScope() const;
    
    // Validation helpers
    bool typesMatch(std::shared_ptr<TypeInfo> type1, std::shared_ptr<TypeInfo> type2) const;
    
    // Context setup
    void setupTypeContext(const std::string& typeName, Context* context);
};

#endif // SYMBOL_COLLECTOR_VISITOR_H