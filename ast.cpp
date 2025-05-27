#include "ast.hpp"

FloatNode::FloatNode(float v) : ASTNode(), value(v) {}

void FloatNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "NUMBER(" << value << ")\n";
}

BoolNode::BoolNode(bool v) : ASTNode(), value(v) {}

void BoolNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "BOOL(" << value << ")\n";
}

StringNode::StringNode(const std::string& v) : ASTNode(), value(v) {}

void StringNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "STRING(" << value << ")\n";
}

UnaryOpNode::UnaryOpNode(const std::string& o, ASTNode* n) : ASTNode(), op(o), node(n) {}

void UnaryOpNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "UNARY_OP(" << op << ")\n";
    node->print(indent + 1);
}

UnaryOpNode::~UnaryOpNode() {
    delete node;
}

BinOpNode::BinOpNode(ASTNode* l, const std::string& o, ASTNode* r) : ASTNode(), op(o), left(l), right(r) {}

void BinOpNode::print(int indent) const {
    std::cout << std::string(indent, ' ') << "BINOP(" << op << ")\n";
    left->print(indent + 1);
    right->print(indent + 1);
}

BinOpNode::~BinOpNode() {
    delete left;
    delete right;
}

FunctionNode::FunctionNode(const std::string& name, ASTNode* arg): func_name(name), argument(arg) {}

void FunctionNode::print(int indent) const {
	std::cout << std::string(indent, ' ') << "FunctionCall: " << func_name << std::endl;
	std::cout << std::string(indent+2, ' ') << "Argument: " << std::endl;
	argument->print(indent+4);
}

IDNode::IDNode(const std::string& name) : id_name(name) {}

void IDNode::print(int indent) const {
	std::cout << std::string(indent, ' ') << "ID(" << id_name << ")" << std::endl;
}

BlockNode::BlockNode(const std::vector<ASTNode*>& nodes) : children(nodes) {}
void BlockNode::print(int indent) const {
	std::cout << std::string(indent, ' ') << "Block:" << std::endl;
	for (const auto& child : children) {
		child->print(indent + 2);
	}
}

void BlockNode::add_child(ASTNode* node) {
	children.push_back(node);
}

BlockNode::~BlockNode() {
	for (auto child : children) {
		delete child;
	}
}

ArgsList::ArgsList(const std::vector<IDNode*>& nodes) : children(nodes) {}
void ArgsList::print(int indent) const {
	std::cout << std::string(indent, ' ') << "ArgsList:" << std::endl;
	for (const auto& child : children) {
		child->print(indent + 1);
	}
}

void ArgsList::add_child(IDNode* node) {
	children.push_back(node);
}

ArgsList::~ArgsList() {
	for (auto child : children) {
		delete child;
	}
}

AssignFuncNode::AssignFuncNode(IDNode* id, ArgsList* arg, ASTNode* body_) : func_name(id->id_name), args(arg), body(body_) {}

void AssignFuncNode::print(int indent) const {
	std::cout << std::string(indent, ' ') << "AssignFunction(" << func_name << ")" << std::endl;
	args->print(indent + 2);
	body->print(indent + 2);
}