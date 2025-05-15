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
        // Aquí iría la lógica para visitar el nodo y realizar la verificación semántica
        // Por ejemplo, podrías usar un switch o un if-else para determinar el tipo de nodo
        // y aplicar las reglas semánticas correspondientes.
    }
    private:
};

#endif