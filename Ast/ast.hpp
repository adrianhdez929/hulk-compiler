#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <variant>
#include <stdexcept>
#include <string>

#include "../semantic/context.h"

#ifndef AST_H
#define AST_H

using namespace std;

class Visitor;

class ASTNode {
public:
	ASTNode() {}
	virtual ~ASTNode() {}
	virtual void print(int indent=0) const=0;
    virtual void accept(Visitor* visitor, Context* context) = 0;
};

class ProgramNode : public ASTNode {
public:
    ASTNode* node;
    ProgramNode(ASTNode* n);
    void print(int indent=0) const override;
};

class FloatNode : public ASTNode {
public:
	float value;
	FloatNode(float v);
	void print(int indent=0) const override;
    void accept(Visitor* visitor, Context* context) override;
};

class BoolNode : public ASTNode {
public:
	bool value;
    BoolNode(bool v);
    void print(int indent = 0) const override;
    void accept(Visitor* visitor, Context* context) override;
};

class StringNode : public ASTNode {
public:
    std::string value;
    StringNode(const std::string& v);
    void print(int indent = 0) const override;
    void accept(Visitor* visitor, Context* context) override;
};

class UnaryOpNode : public ASTNode {
public:
    std::string op;
    ASTNode* node;

    UnaryOpNode(const std::string& o, ASTNode* n);
    void print(int indent = 0) const override;
    void accept(Visitor* visitor, Context* context) override;
    ~UnaryOpNode();
}; 

class BinOpNode : public ASTNode {
public:
    std::string op;
    ASTNode* left;
    ASTNode* right;

    BinOpNode(ASTNode* l, const std::string& o, ASTNode* r);
    void print(int indent = 0) const override;
    void accept(Visitor* visitor, Context* context) override;
    ~BinOpNode();
};

#endif