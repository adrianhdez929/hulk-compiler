#include "../Ast/ast.hpp"

#ifndef NODES_H
#define NODES_H

class VisitableNode {
    public:
    virtual void accept(Visitor* visitor) = 0;
};

class VisitableFloatNode : public VisitableNode, public FloatNode {
    public:
    void accept(Visitor* visitor) override;
};

class VisitableStringNode : public VisitableNode, public StringNode {
    public:
    void accept(Visitor* visitor) override;
};

class VisitableBoolNode : public VisitableNode, public BoolNode {
    public:
    void accept(Visitor* visitor) override;
};

class VisitableUnaryOpNode : public VisitableNode, public UnaryOpNode {
    public:
    void accept(Visitor* visitor) override;
};

class VisitableBinOpNode : public VisitableNode, public BinOpNode {
    public:
    void accept(Visitor* visitor) override;
};

#endif