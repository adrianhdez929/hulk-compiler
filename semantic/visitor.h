#include "context.h"

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
	virtual void visit(AssignFuncNode* node, Context* context) = 0;
	virtual void visit(LetAssign* node, Context* context) = 0;
	virtual void visit(VarAssign* node, Context* context) = 0;
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

};

class SemanticCheckerVisitor : public Visitor {
    public:
    SemanticCheckerVisitor() {
        globalContext = new Context(nullptr);
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
    
    Context* getContext() { return globalContext; }
    
private:
    Context* globalContext;
};

#endif