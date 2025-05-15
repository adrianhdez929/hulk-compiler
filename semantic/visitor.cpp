#include "visitor.h"
#include "../Ast/ast.hpp"
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


void SemanticCheckerVisitor::visit(ASTNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    node->accept(this, context);
}

void SemanticCheckerVisitor::visit(FloatNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting Float Node: " << node->value << endl;
/*************  ✨ Windsurf Command ⭐  *************/
/**
 * Visita un nodo de tipo float y lo muestra en la consola de depuración.
 *
 * @param node Nodo de tipo float a visitar
 * @param context Contexto en el que se visita el nodo
 *
 * @throw std::runtime_error Si el nodo es nulo
 */
/*******  e44fb964-8e56-4568-9d93-144fe618622a  *******/}

void SemanticCheckerVisitor::visit(BoolNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    cout << "Visiting Bool Node: " << node->value << endl;
}

void SemanticCheckerVisitor::visit(StringNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    cout << "Visiting String Node: " << node->value << endl;
}

void SemanticCheckerVisitor::visit(UnaryOpNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting UnaryOp Node: " << node->op << endl;
    
    this->visit(node->node, context);
}

void SemanticCheckerVisitor::visit(BinOpNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting BinOp Node: " << node->op << endl;
    
    this->visit(node->left, context);
    this->visit(node->right, context);
}