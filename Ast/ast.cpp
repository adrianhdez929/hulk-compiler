#include "ast.hpp"
#include "../semantic/visitor.h"

ProgramNode::ProgramNode(ASTNode* n) : ASTNode(), node(n) {}

void ProgramNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "PROGRAM\n";
    node->print(indent + 1);
}

FloatNode::FloatNode(float v) : ASTNode(), value(v) {}

void FloatNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "NUMBER(" << value << ")\n";
}

void FloatNode::accept(Visitor* visitor, Context* context) {
    visitor->visit(this, context);
}

BoolNode::BoolNode(bool v) : ASTNode(), value(v) {}

void BoolNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "BOOL(" << value << ")\n";
}

void BoolNode::accept(Visitor* visitor, Context* context) {
    visitor->visit(this, context);
}

StringNode::StringNode(const std::string& v) : ASTNode(), value(v) {}

void StringNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "STRING(\"" << value << "\")\n";
}

void StringNode::accept(Visitor* visitor, Context* context) {
    visitor->visit(this, context);
}

UnaryOpNode::UnaryOpNode(const std::string& o, ASTNode* n) : ASTNode(), op(o), node(n) {}

void UnaryOpNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "UNARY_OP(" << op << ")\n";
    node->print(indent + 1);
}

void UnaryOpNode::accept(Visitor* visitor, Context* context) {
    visitor->visit(this, context);
}

UnaryOpNode::~UnaryOpNode() {
    delete node;
}

BinOpNode::BinOpNode(ASTNode* l, const std::string& o, ASTNode* r) : ASTNode(), op(o), left(l), right(r) {}

/*************  ✨ Windsurf Command ⭐  *************/
    /**
     * @brief Imprime el nodo de operaci n binaria como un string indentado
     *
     * @param indent Nivel de indentaci n para la impresi n
     */
/*******  04711f06-7700-4b76-899c-dc8f97647e64  *******/
void BinOpNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "BINOP(" << op << ")\n";
    left->print(indent + 1);
    right->print(indent + 1);
}
void BinOpNode::accept(Visitor* visitor, Context* context) {
    visitor->visit(this, context);
}
BinOpNode::~BinOpNode() {
    delete left;
    delete right;
}
