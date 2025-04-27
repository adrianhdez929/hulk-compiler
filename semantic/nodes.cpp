#include "nodes.h"

void VisitableAtomicNode::accept(Visitor* visitor) {
    if (visitor == nullptr) {
        throw std::runtime_error("Visitor is null");
    }
    visitor->visit(this);
};

void VisitableUnaryNode::accept(Visitor* visitor) {
    if (visitor == nullptr) {
        throw std::runtime_error("Visitor is null");
    }
    visitor->visit(this);
}
void VisitableBinaryNode::accept(Visitor* visitor) {
    if (visitor == nullptr) {
        throw std::runtime_error("Visitor is null");
    }
    visitor->visit(this);
}