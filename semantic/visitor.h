#include "context.h"
#include <stdexcept>
#include <memory>

#ifndef VISITOR_H
#define VISITOR_H

class ASTNode;
class FloatNode;
class BoolNode;
class StringNode;
class UnaryOpNode;
class BinOpNode;
class FunctionCallNode;
class IDNode;
class BlockNode;
class ArgsList;
class AssignFuncNode;
class LetAssign;
class VarAssign;
class NewTypeNode;
class VarAssignType;
class VarAssignList;
class Conditional;
class BoolExprNode;
class WhileNode;
class VarDesAssign;
class ForNode;
class TypeDeclNode;
class ASTNodeVector;
class ExprsList;
class ProgramNode;
class AccessNode;
class TypeAssMember;
class AttributeMember;
class MethodMember;

class Visitor {
    public:
    virtual ~Visitor() = default;
    virtual void visit(ASTNode* node, Context* context) = 0;
    virtual void visit(FloatNode* node, Context* context) = 0;
    virtual void visit(BoolNode* node, Context* context) = 0;
    virtual void visit(StringNode* node, Context* context) = 0;
    virtual void visit(UnaryOpNode* node, Context* context) = 0;
    virtual void visit(BinOpNode* node, Context* context) = 0;
	virtual void visit(FunctionCallNode* node, Context* context) = 0;
	virtual void visit(IDNode* node, Context* context) = 0;
	virtual void visit(BlockNode* node, Context* context) = 0;
	virtual void visit(ArgsList* node, Context* context) = 0;
	virtual void visit(AssignFuncNode* node, Context* context) = 0;    virtual void visit(LetAssign* node, Context* context) = 0;
    virtual void visit(VarAssign* node, Context* context) = 0;
    virtual void visit(NewTypeNode* node, Context* context) = 0;
    virtual void visit(VarAssignType* node, Context* context) = 0;
    virtual void visit(VarAssignList* node, Context* context) = 0;
	virtual void visit(Conditional* node, Context* context) = 0;
	virtual void visit(BoolExprNode* node, Context* context) = 0;
	virtual void visit(WhileNode* node, Context* context) = 0;
	virtual void visit(VarDesAssign* node, Context* context) = 0;
	virtual void visit(ForNode* node, Context* context) = 0;
	virtual void visit(TypeDeclNode* node, Context* context) = 0;
	virtual void visit(ASTNodeVector* node, Context* context) = 0;
	virtual void visit(ExprsList* node, Context* context) = 0;
	virtual void visit(ProgramNode* node, Context* context) = 0;
	virtual void visit(AccessNode* node, Context* context) = 0;
	virtual void visit(TypeAssMember* node, Context* context) = 0;
	virtual void visit(AttributeMember* node, Context* context) = 0;
	virtual void visit(MethodMember* node, Context* context) = 0;

};

class SemanticCheckerVisitor : public Visitor {
    public:
    SemanticCheckerVisitor() {
        globalContext = new Context(nullptr);
        Context::initializeBuiltinTypes();
        
        if (Context::objectType && Context::objectType->typeDef) {
            globalContext->defineType("Object", Context::objectType->typeDef);
        }
        
        std::vector<std::shared_ptr<TypeInfo>> printParams = {Context::stringType};
        globalContext->defineFunc("print", Context::voidType, printParams);
        
        std::vector<std::shared_ptr<TypeInfo>> readParams;
        globalContext->defineFunc("read", Context::stringType, readParams);
    }
    
    ~SemanticCheckerVisitor() {
        if (globalContext)
            delete globalContext;
    }

    void visit(ASTNode* node, Context* context) override;
    void visit(FloatNode* node, Context* context) override;
    void visit(BoolNode* node, Context* context) override ;
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
    
    Context* getContext() { return globalContext; }
    
    void checkTypeCompatibility(std::shared_ptr<TypeInfo> expected, std::shared_ptr<TypeInfo> actual, const std::string& operation) {
        if (!expected || !actual) return;
        
        if (!globalContext->canAssign(actual, expected)) {
            throw std::runtime_error("Type error in " + operation + ": Cannot assign " + 
                                   actual->name + " to " + expected->name);
        }
    }
    
    std::shared_ptr<TypeInfo> inferBinaryOperationType(const std::string& op, 
                                                      std::shared_ptr<TypeInfo> leftType, 
                                                      std::shared_ptr<TypeInfo> rightType) {
        if (op == "+" || op == "-" || op == "*" || op == "/" || op == "%" || op == "^") {
            return Context::numberType;
        } else if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=" ||
                   op == "&&" || op == "||" || op == "and" || op == "or") {
            return Context::boolType;
        } else if (op == "=" || op == ":=") {
            return leftType;
        }
        return Context::voidType;
    }
    
private:
    Context* globalContext;
};

#endif