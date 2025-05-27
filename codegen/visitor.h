#include "../semantic/visitor.h"
#include "../semantic/context.h"

#ifndef CODEGEN_VISITOR_H
#define CODEGEN_VISITOR_H

class CodegenVisitor : public Visitor {
    public:
    void visit(ASTNode* node, Context* context) override;
    void visit(FloatNode* node, Context* context) override;
    void visit(BoolNode* node, Context* context) override ;
    void visit(StringNode* node, Context* context) override;
    void visit(UnaryOpNode* node, Context* context) override;
    void visit(BinOpNode* node, Context* context) override;
};

#endif