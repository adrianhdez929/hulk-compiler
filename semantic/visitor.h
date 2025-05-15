#include "context.h"

#ifndef VISITOR_H
#define VISITOR_H

class ASTNode;
class FloatNode;
class BoolNode;
class StringNode;
class UnaryOpNode;
class BinOpNode;

class Visitor {
    public:
    virtual void visit(ASTNode* node, Context* context) {};
    virtual void visit(FloatNode* node, Context* context) {};
    virtual void visit(BoolNode* node, Context* context) {};
    virtual void visit(StringNode* node, Context* context) {};
    virtual void visit(UnaryOpNode* node, Context* context) {};
    virtual void visit(BinOpNode* node, Context* context) {};
};

class SemanticCheckerVisitor : public Visitor {
    public:
    void visit(ASTNode* node, Context* context) override;
    void visit(FloatNode* node, Context* context) override;
    void visit(BoolNode* node, Context* context) override ;
    void visit(StringNode* node, Context* context) override;
    void visit(UnaryOpNode* node, Context* context) override;
    void visit(BinOpNode* node, Context* context) override;
};

#endif