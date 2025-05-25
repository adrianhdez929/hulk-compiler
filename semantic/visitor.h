#include "context.h"

#ifndef VISITOR_H
#define VISITOR_H

class ASTNode;
class FloatNode;
class BoolNode;
class StringNode;
class UnaryOpNode;
class BinOpNode;
class FunctionNode;
class IDNode;
class BlockNode;
class ArgsList;
class AssignFuncNode;

class Visitor {
    public:
    virtual void visit(ASTNode* node, Context* context) {};
    virtual void visit(FloatNode* node, Context* context) {};
    virtual void visit(BoolNode* node, Context* context) {};
    virtual void visit(StringNode* node, Context* context) {};
    virtual void visit(UnaryOpNode* node, Context* context) {};
    virtual void visit(BinOpNode* node, Context* context) {};
	virtual void visit(FunctionNode* node, Context* context) {};
	virtual void visit(IDNode* node, Context* context) {};
	virtual void visit(BlockNode* node, Context* context) {};
	virtual void visit(ArgsList* node, Context* context) {};
	virtual void visit(AssignFuncNode* node, Context* context) {};

};

class SemanticCheckerVisitor : public Visitor {
    public:
    void visit(ASTNode* node, Context* context) override;
    void visit(FloatNode* node, Context* context) override;
    void visit(BoolNode* node, Context* context) override ;
    void visit(StringNode* node, Context* context) override;
    void visit(UnaryOpNode* node, Context* context) override;
    void visit(BinOpNode* node, Context* context) override;
    void visit(FunctionNode* node, Context* context) override;
    void visit(IDNode* node, Context* context) override;
    void visit(BlockNode* node, Context* context) override;
    void visit(ArgsList* node, Context* context) override;
    void visit(AssignFuncNode* node, Context* context) override;

};

#endif