#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <variant>
#include <stdexcept>
#include <string>

#ifndef AST_H
#define AST_H

using namespace std;

class ASTNode {
public:
	ASTNode() {}
	virtual ~ASTNode() {}
	virtual void print(int indent=0) const=0;

};	

class FloatNode : public ASTNode {
public:
	float value;
	FloatNode(float v);
	void print(int indent=0) const override;

};

class BoolNode : public ASTNode {
public:
	bool value;
    BoolNode(bool v);
    void print(int indent = 0) const override;
};

class StringNode : public ASTNode {
public:
    std::string value;
    StringNode(const std::string& v);
    void print(int indent = 0) const override;
};

class UnaryOpNode : public ASTNode {
public:
    std::string op;
    ASTNode* node;

    UnaryOpNode(const std::string& o, ASTNode* n);
    void print(int indent = 0) const override;
    ~UnaryOpNode();
}; 

class BinOpNode : public ASTNode {
public:
    std::string op;
    ASTNode* left;
    ASTNode* right;

    BinOpNode(ASTNode* l, const std::string& o, ASTNode* r);
    void print(int indent = 0) const override;
    ~BinOpNode();
};

class FunctionNode : public ASTNode {
public:
	std::string func_name;
	ASTNode* argument;

	FunctionNode(const std::string& name, ASTNode* arg);
	void print(int indent = 0) const override;
};

class IDNode : public ASTNode {
public:
	std::string id_name;
	IDNode(const std::string& name);
	void print(int indent =0 ) const override;
};

class BlockNode : public ASTNode {
public:
	std::vector<ASTNode*> children;

	BlockNode(const std::vector<ASTNode*>& node);
	void add_child(ASTNode* node);
	void print(int indent = 0) const override;
	~BlockNode();
};

class ArgsList: public ASTNode {
public:
	std::vector<IDNode*> children;

	ArgsList(const std::vector<IDNode*>& nodes);
	void add_child(IDNode* node);
	void print(int indent = 0) const override;
	~ArgsList();
};

class AssignFuncNode: public ASTNode {
public:
	std::string func_name;
	ArgsList* args;
	ASTNode* body;

	AssignFuncNode(IDNode* id, ArgsList* arg, ASTNode* body_);
	void print(int indent = 0) const override;
};

#endif