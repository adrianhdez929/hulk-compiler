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

// Forward declarations para eliminar dependencias
class Context;
class Visitor;
class TypeInfo;

using namespace std;

/**
 * @class ASTNode
 * @brief Nodo base para el Árbol de Sintaxis Abstracta (AST).
 */

class Visitor;

class ASTNode {
public:
	// Semantic attributes for attributed grammar
	
	int line; // Número de línea de origen del nodo
	std::shared_ptr<TypeInfo> inferredType;
	std::string semanticValue;
	
	ASTNode() : inferredType(nullptr), semanticValue(""), line(0) {}
	ASTNode(int line_) : inferredType(nullptr), semanticValue(""), line(line_) {}
	virtual ~ASTNode() = default;
	
	virtual void print(int indent=0) const=0;
	virtual void accept(Visitor* visitor, Context* context) = 0;
};	

class ASTNodeVector : public ASTNode { 
public:
	std::vector<ASTNode*> children;

	ASTNodeVector(const std::vector<ASTNode*>& nodes, int line_);
	void add_child(ASTNode* node);
	void print(int indent=0) const override;
	void accept(Visitor* visitor, Context* context);
	~ASTNodeVector();
};

class ProgramNode : public ASTNode {
private:
	ASTNode* node;
public:
	ProgramNode(ASTNode* n, int line_);
	void print(int indent=0) const override;
	void accept(Visitor* visitor, Context* context) override;
	ASTNode* getNode() const { return node; }
};

class FloatNode : public ASTNode {
public:
	float value;
	FloatNode(float v, int line_);
	void print(int indent=0) const override;
	void accept(Visitor* visitor, Context* context) override;

};

class BoolNode : public ASTNode {
public:
	bool value;
    BoolNode(bool v, int line_);
    void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;

};

class BoolExprNode : public ASTNode {
public: 
	ASTNode* expr;

	BoolExprNode(ASTNode* expr_, int line_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};

class StringNode : public ASTNode {
public:
	
    std::string value;
    StringNode(const std::string& v, int line_);
    void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;

};

class UnaryOpNode : public ASTNode {
public:
    std::string op;
    ASTNode* node;

    UnaryOpNode(const std::string& o, ASTNode* n, int line_);
    void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
    ~UnaryOpNode();
}; 

class BinOpNode : public ASTNode {
public:
	
    std::string op;
    ASTNode* left;
    ASTNode* right;

    BinOpNode(ASTNode* l, const std::string& o, ASTNode* r, int line_);
    void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
    ~BinOpNode();
};

class FunctionCallNode : public ASTNode {
public:
	
	std::string func_name;
	ASTNode* argument;

	FunctionCallNode(const std::string& name, ASTNode* arg, int line_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* Context) override;
};

class IDNode : public ASTNode {
public:
	
	std::string id_name;
	std::string id_type;
	IDNode(const std::string& name, int line_);
	IDNode(const std::string& name, const std::string& type, int line_);
	void print(int indent =0 ) const override;
	void accept(Visitor* visitor, Context* Context) override;
};

class BlockNode : public ASTNode {
public:
	
	std::vector<ASTNode*> children;

	BlockNode(const std::vector<ASTNode*>& node, int line_);
	void add_child(ASTNode* node);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* Context) override;
	~BlockNode();
};

class ArgsList: public ASTNode { // lista de IDNode solamente
public:
	
	std::vector<IDNode*> children;

	ArgsList(const std::vector<IDNode*>& nodes, int line_);
	void add_child(IDNode* node);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* Context) override;
	~ArgsList();
};

class ExprsList : public ASTNode {
public:
	
	std::vector<ASTNode*> children;

	ExprsList(const std::vector<ASTNode*>& nodes, int line_);
	void add_child(ASTNode* node);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
	~ExprsList();
};

class AssignFuncNode: public ASTNode {
public:
	
	std::string func_name;
	ArgsList* args;
	ASTNode* body;
	std::string func_type;

	AssignFuncNode(IDNode* id, ArgsList* arg, ASTNode* body_, int line_);
	AssignFuncNode(IDNode* id, ArgsList* arg, ASTNode* body_, std::string func_type_, int line_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* Context) override;
};

class VarAssign: public ASTNode {
public:
	
	IDNode* var_id;
	ASTNode* value;
	std::string treated_as_type;

	VarAssign(IDNode* id, ASTNode* value_, int line_);
	VarAssign(IDNode* id, ASTNode* value_, std::string treated_as_type_, int line_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};
class NewTypeNode : public ASTNode {
public:
	
	std::string id_type_name;
	std::vector<ASTNode*> expr_list;

	NewTypeNode(std::string id, std::vector<ASTNode*> expr_list_, int line_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};

class VarAssignType : public ASTNode {
public:
	
	std::string var_name;
	NewTypeNode* new_type;
	ASTNode* body;

	VarAssignType(const std::string name, NewTypeNode* new_type_, ASTNode* body_, int line_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};


class VarAssignList: public ASTNode {
public:
	
	std::vector<VarAssign*> assigns;

	VarAssignList(const std::vector<VarAssign*>& assigns_, int line_);
	void add_child(VarAssign* assign);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};

class LetAssign: public ASTNode {
public:
	
	std::vector<VarAssign*> assigns;
	ASTNode* body;

	LetAssign(const std::vector<VarAssign*>& assigns_, ASTNode* body_, int line_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* Context) override;
};

class VarDesAssign: public ASTNode {
public:
	
	IDNode* id;
	ASTNode* value;

	VarDesAssign(IDNode* id_, ASTNode* value_, int line_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};

class Conditional: public ASTNode {
public:
	
	BoolExprNode* bool_expr;
	ASTNode* if_body;
	ASTNode* else_body;

	Conditional(BoolExprNode* bool_expr_, ASTNode* if_body_, ASTNode* else_body_, int line_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};

class WhileNode: public ASTNode {
public:
	
	BoolExprNode* bool_expr;
	ASTNode* body;

	WhileNode(BoolExprNode* bool_expr_, ASTNode* body_, int line_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};

class ForNode: public ASTNode {
public:
	
	IDNode* id;
	ASTNode* group;
	ASTNode* body;

	ForNode(IDNode* id_, ASTNode* group_, ASTNode* body_, int line_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};

class TypeDeclNode: public ASTNode {
public:
	
	IDNode* id;
	ArgsList* args;
	std::vector<std::string> parents;
	std::vector<ASTNode*> body;
	ArgsList* parent_args;

	TypeDeclNode(IDNode* id, ArgsList* args_, const std::vector<ASTNode*>& body_, int line_);
	TypeDeclNode(IDNode* id, ArgsList* args_, const std::vector<ASTNode*>& body_, std::vector<std::string> parents_, int line_);
	TypeDeclNode(IDNode* id, ArgsList* args_, const std::vector<ASTNode*>& body_, std::vector<std::string> parents_, ArgsList* parent_args_, int line_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};



class TypeAssMember : public ASTNode {
public:
	
	enum class Form { Attribute, Method };

	TypeAssMember(TypeAssMember::Form form_, int line_);
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
	
	AttributeMember(std::string name_, int line_);
	std::string get_name() const override;
	void print(int indent) const override;
	void accept(Visitor* visitor, Context* context);
	~AttributeMember();
};

class MethodMember : public TypeAssMember {
public:
	
	std::string name;
	std::vector<ASTNode*> args;

	MethodMember(std::string name_, std::vector<ASTNode*> args_, int line_); //por ahora las llamadas de funcion solo aceptan un parametro, arreglar luego
	std::string get_name() const override;
	void print(int indent) const override;
	void accept(Visitor* visitor, Context* context);
	~MethodMember();
};

class AccessNode: public ASTNode {
public:
	
	const std::string var_name;
	TypeAssMember* member;
	
	AccessNode(const std::string var_name_, TypeAssMember* member_, int line_);
	std::string get_name() const;
	TypeAssMember::Form get_form() const;
	void print(int indent) const override;
	void accept(Visitor* visitor, Context* context);
};

class TypeCastNode: public ASTNode {
public:
	ASTNode* expr;
	std::string target_type;

	TypeCastNode(ASTNode* expr_, const std::string& target_type_);
	TypeCastNode(ASTNode* expr_, const std::string& target_type_, int line_);
	void print(int indent = 0) const override;
	void accept(Visitor* visitor, Context* context) override;
};

#endif