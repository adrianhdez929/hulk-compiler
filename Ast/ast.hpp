#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <variant>
#include <stdexcept>
#include <string>

#include "../semantic/context.h"
#include "../semantic/visitor.h"

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

class ASTNodeVector : public ASTNode { 
public:
	std::vector<ASTNode*> children;

	ASTNodeVector(const std::vector<ASTNode*>& nodes);
	void add_child(ASTNode* node);
	void print(int indent=0) const override;
	void accept(Visitor* visitor, Context* context);
	~ASTNodeVector();
};

class ProgramNode : public ASTNode {
private:
	ASTNode* node;
public:
	ProgramNode(ASTNode* n);
	void print(int indent=0) const override;
	void accept(Visitor* visitor, Context* context) override;
	ASTNode* getNode() const { return node; }
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

class BoolExprNode : public ASTNode {
public: 
	ASTNode* expr;

	BoolExprNode(ASTNode* expr_);
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

class FunctionCallNode : public ASTNode {
public:
	std::string func_name;
	ASTNode* argument;

	FunctionCallNode(const std::string& name, ASTNode* arg);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* Context) override;
};

class IDNode : public ASTNode {
public:
	std::string id_name;
	std::string id_type;
	IDNode(const std::string& name);
	IDNode(const std::string& name, const std::string& type);
	void print(int indent =0 ) const override;
	void accept(Visitor* visitor, Context* Context) override;
};

class BlockNode : public ASTNode {
public:
	std::vector<ASTNode*> children;

	BlockNode(const std::vector<ASTNode*>& node);
	void add_child(ASTNode* node);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* Context) override;
	~BlockNode();
};

class ArgsList: public ASTNode { // lista de IDNode solamente
public:
	std::vector<IDNode*> children;

	ArgsList(const std::vector<IDNode*>& nodes);
	void add_child(IDNode* node);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* Context) override;
	~ArgsList();
};

class ExprsList : public ASTNode {
public:
	std::vector<ASTNode*> children;

	ExprsList(const std::vector<ASTNode*>& nodes);
	void add_child(ASTNode* node);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context);
	~ExprsList();
};

class AssignFuncNode: public ASTNode {
public:
	std::string func_name;
	ArgsList* args;
	ASTNode* body;

	AssignFuncNode(IDNode* id, ArgsList* arg, ASTNode* body_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* Context) override;
};

class VarAssign: public ASTNode {
public:
	IDNode* var_id;
	ASTNode* value;

	VarAssign(IDNode* id, ASTNode* value_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};

class VarAssignType : public ASTNode {
public:
	std::string var_name;
	IDNode* id_type_name;
	ASTNode* body;

	VarAssignType(const std::string name, IDNode* id_type, ASTNode* body_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};

class VarAssignList: public ASTNode {
public:
	std::vector<VarAssign*> assigns;

	VarAssignList(const std::vector<VarAssign*>& assigns_);
	void add_child(VarAssign* assign);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};

class LetAssign: public ASTNode {
public:
	std::vector<VarAssign*> assigns;
	ASTNode* body;

	LetAssign(const std::vector<VarAssign*>& assigns_, ASTNode* body_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* Context) override;
};

class VarDesAssign: public ASTNode {
public:
	IDNode* id;
	ASTNode* value;

	VarDesAssign(IDNode* id_, ASTNode* value_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};

class Conditional: public ASTNode {
public:
	BoolExprNode* bool_expr;
	ASTNode* if_body;
	ASTNode* else_body;

	Conditional(BoolExprNode* bool_expr_, ASTNode* if_body_, ASTNode* else_body_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};

class WhileNode: public ASTNode {
public:
	BoolExprNode* bool_expr;
	ASTNode* body;

	WhileNode(BoolExprNode* bool_expr_, ASTNode* body_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};

class ForNode: public ASTNode {
public:
	IDNode* id;
	ASTNode* group;
	ASTNode* body;

	ForNode(IDNode* id_, ASTNode* group_, ASTNode* body_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};

class TypeDeclNode: public ASTNode {
public:
	IDNode* id;
	ArgsList* args;
	std::vector<std::string> parents;
	std::vector<ASTNode*> body;

	TypeDeclNode(IDNode* id, ArgsList* args_, const std::vector<ASTNode*>& body_);
	TypeDeclNode(IDNode* id, ArgsList* args_, const std::vector<ASTNode*>& body_, std::vector<std::string> parents_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};



class TypeAssMember : public ASTNode {
public:
	enum class Form { Attribute, Method };

	TypeAssMember(TypeAssMember::Form form_);
	TypeAssMember::Form get_form();
	virtual std::string get_name() const = 0;
	virtual void print(int indent = 0) const override = 0;
	virtual void accept(Visitor* visitor, Context* context) override = 0;
	virtual ~TypeAssMember() = default;

private:
	Form form;
};

class AttributeMember : public TypeAssMember {
public:
	std::string name;
	
	AttributeMember(std::string name_);
	std::string get_name() const override;
	void print(int indent) const override;
	void accept(Visitor* visitor, Context* context);
	~AttributeMember();
};

class MethodMember : public TypeAssMember {
public:
	std::string name;
	std::vector<ASTNode*> args;

	MethodMember(std::string name_, std::vector<ASTNode*> args_); //por ahora las llamadas de funcion solo aceptan un parametro, arreglar luego
	std::string get_name() const override;
	void print(int indent) const override;
	void accept(Visitor* visitor, Context* context);
	~MethodMember();
};

class AccessNode: public ASTNode {
public:
	const std::string var_name;
	TypeAssMember* member;
	
	AccessNode(const std::string var_name_, TypeAssMember* member_);
	std::string get_name() const;
	TypeAssMember::Form get_form() const;
	void print(int indent) const override;
	void accept(Visitor* visitor, Context* context);
};

#endif