#include "../Ast/ast.h"

#ifndef SEMANTIC_H
#define SEMANTIC_H

class SemanticChecker {
    public:
    SemanticChecker() = default;
    ~SemanticChecker() = default;

    void visit(Node* node) {
        if (node == nullptr) {
            throw std::runtime_error("Node is null");
        }
    }
    private:
};

#endif