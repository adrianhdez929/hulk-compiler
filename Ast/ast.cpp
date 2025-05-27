#include "ast.hpp"


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
    std::cout << std::string(indent, ' ') << "STRING(" << value << ")\n";
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

FunctionNode::FunctionNode(const std::string& name, ASTNode* arg): func_name(name), argument(arg) {}

void FunctionNode::print(int indent) const {
	std::cout << std::string(indent, ' ') << "FunctionCall: " << func_name << std::endl;
	std::cout << std::string(indent+2, ' ') << "Argument: " << std::endl;
	argument->print(indent+4);
}

void FunctionNode::accept(Visitor* visitor, Context* context) {
	visitor->visit(this, context);
}

IDNode::IDNode(const std::string& name) : id_name(name) {}

void IDNode::print(int indent) const {
	std::cout << std::string(indent, ' ') << "ID(" << id_name << ")" << std::endl;
}

void IDNode::accept(Visitor* visitor, Context* context) {
	visitor->visit(this, context);
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

void BlockNode::accept(Visitor* visitor, Context* context) {
	visitor->visit(this, context);
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

void ArgsList::accept(Visitor* visitor, Context* context) {
	visitor->visit(this, context);
}

ArgsList::~ArgsList() {
	for (auto child : children) {
		delete child;
	}
}

AssignFuncNode::AssignFuncNode(IDNode* id, ArgsList* arg, ASTNode* body_) : func_name(id->id_name), args(arg), body(body_) {}

void AssignFuncNode::print(int indent) const {
	std::cout << std::string(indent, ' ') << "AssignFunction(" << func_name << ")" << std::endl;
	std::cout << std::string(indent+1, ' ') << "Args:" << std::endl;
	args->print(indent + 2);
	std::cout << std::string(indent+1, ' ') << "Body:" << std::endl;
	body->print(indent + 2);
}

void AssignFuncNode::accept(Visitor* visitor, Context* context) {
	visitor->visit(this, context);
}

LetAssign::LetAssign(const std::vector<VarAssign*>& assigns_, ASTNode* body_) : assigns(assigns_), body(body_) {}

void LetAssign::print(int indent) const {
	std::cout << std::string(indent, ' ') << "LetAssign" << std::endl;
	std::cout << std::string(indent+1, ' ') << "VarAssigns:" << std::endl;
	for (const auto& assign : assigns) {
		assign->print(indent+2);
	}
	std::cout << std::string(indent+1, ' ') << "Body:" << std::endl;
	body->print(indent + 2);
}

void LetAssign::accept(Visitor* visitor, Context* context) {
	visitor->visit(this, context);
}

VarAssign::VarAssign(IDNode* id, ASTNode* value_) : var_name(id->id_name), value(value_) {}

void VarAssign::print(int indent) const {
	std::cout << std::string(indent, ' ') << "VarAssign(" << var_name << ")" << std::endl;
	std::cout << std::string(indent+1, ' ') << "Value" << std::endl;
	value->print(indent + 2);
}

void VarAssign::accept(Visitor* visitor, Context* context) {
	visitor->visit(this, context); 
}

VarAssignList::VarAssignList(const std::vector<VarAssign*>& assigns_) : assigns(assigns_) {}

void VarAssignList::add_child(VarAssign* node) {
	assigns.push_back(node);
}

void VarAssignList::print(int indent) const {
	std::cout << std::string(indent, ' ') << "VarAssignList:" << std::endl;
	for (const auto& assign : assigns) {
		assign->print(indent + 2);
	}
}

void VarAssignList::accept(Visitor* visitor, Context* context) {
	visitor->visit(this, context);
}

VarDesAssign::VarDesAssign(IDNode* id_, ASTNode* value_) : id(id_), value(value_) {}

void VarDesAssign::print(int indent) const {
	std::cout << std::string(indent, ' ') << "VarDesAssign(" << id->id_name << ")" << std::endl;
	std::cout << std::string(indent+1, ' ') << "Value:" << std::endl;
	value->print(indent+2);
}

void VarDesAssign::accept(Visitor* visitor, Context* context) {
	visitor->visit(this, context);
}