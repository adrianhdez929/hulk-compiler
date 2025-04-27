#include "visitor.h"
#include "context.h"
#include <iostream>

/**
 * Visita un nodo del árbol de sintaxis abstracto (AST) y verifica su
 * corrección semántica en función del contexto proporcionado.
 *
 * @param node Nodo del AST a visitar
 * @param context Contexto en el que se va a visitar el nodo
 *
 * @throw std::runtime_error Si el nodo es nulo
 */

void SemanticCheckerVisitor::visit(Node* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    // Aquí iría la lógica para visitar el nodo y realizar la verificación semántica
    // Por ejemplo, podrías usar un switch o un if-else para determinar el tipo de nodo
    // y aplicar las reglas semánticas correspondientes.
}

void SemanticCheckerVisitor::visit(VisitableAtomicNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting Atomic Node: " << node->lex << endl;
}
void SemanticCheckerVisitor::visit(VisitableUnaryNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    this.visit(node->node.get());
}
void SemanticCheckerVisitor::visit(VisitableBinaryNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    this.visit(node->left.get());
    this.visit(node->right.get());
}