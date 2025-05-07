#include "../Ast/node.h"
#include "nodes.h"

#ifndef VISITOR_H
#define VISITOR_H

class Visitor {
    public:
    virtual void visit(Node* node, Context* context);
};

class SemanticCheckerVisitor : public Visitor {
    public:
    virtual void visit(Node* node);

    void visit(VisitableAtomicNode* node, Context* context);
    void visit(VisitableUnaryNode* node, Context* context);
    void visit(VisitableBinaryNode* node, Context* context);
};

#endif