#include "../Ast/node.h"

#ifndef NODES_H
#define NODES_H

class VisitableNode {
    public:
    virtual void accept(Visitor* visitor) = 0;
};

class VisitableAtomicNode : public VisitableNode, public AtomicNode {
    public:
    void accept(Visitor* visitor) override;
};

class VisitableUnaryNode : public VisitableNode, public UnaryNode {
    public:
    void accept(Visitor* visitor) override;
};

class VisitableBinaryNode : public VisitableNode, public BinaryNode {
    public:
    void accept(Visitor* visitor) override;
};

#endif