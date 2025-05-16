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



#endif