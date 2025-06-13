%{
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include "../Ast/ast.hpp"

extern "C" {
	void yyerror(const char *);
	int yylex(void);
}

extern ASTNode* root;

%}


%union {
	float num;
	char* str;
	bool boolean;
	class ASTNode* node; 
	class BlockNode* b_node;
	class ArgsList* args_l;
	class IDNode* id_node;
	class AssignFuncNode* ass_f_node;
	class LetAssign* ass_var;
	class VarAssignList* v_ass_l;
	class BoolExprNode* b_expr_node;
	class Conditional* cond;
	class WhileNode* while_node;
	class ForNode* for_node;
	class TypeDeclNode* t_node_decl;
	class ASTNodeVector* ast_node_v; 
	class VarAssign* var_ass;
	class AttributeMember* att_member;
	class MethodMember* meth_member;
	class AccessNode* acc_node;
	class VarAssignType* v_ass_t;
	class NewTypeNode* new_t_n;
}

%token NUMBER
%token BOOLEAN
%token STRING
%token ID_ 
%token PLUS MINUS TIMES DIV POW LPARENT RPARENT SEMICOLON COLON LKEY RKEY FUNCTION_ INLINE ASSIGN ASS_DES IF ELSE ELIF WHILE FOR ACCESS UMINUS TWOPOINTS NEW INHERITS IS AS_ 
%token GREATER_EQUAL GREATER LESS_EQUAL LESS EQUAL DISTINCT AND_ NOT_ OR_
%token LET
%token IN
%token TYPE

%token <num> NUMBER
%token <str> ID_
%token <str> STRING
%token <boolean> BOOLEAN

%type <node> expr arit_op lines_block line func_call 
%type <b_node> lines non_empty_lines
%type <args_l> args_list
%type <id_node> id_expr
%type <ass_f_node> func_asign method
%type <ass_var> let_assign
%type <v_ass_l> var_assign_list
%type <b_expr_node> bool_expr
%type <cond> conditional
%type <while_node> while_expr
//%type <for_node> for_expr
%type <t_node_decl> type_node_decl
%type <ast_node_v> type_body_elements
%type <ast_node_v> expr_list
%type <var_ass> attribute
%type <acc_node> member_access_expr
%type <new_t_n> new_expr


%nonassoc GREATER_EQUAL GREATER LESS_EQUAL LESS EQUAL DISTINCT
%nonassoc ELSE ELIF
%nonassoc ASSIGN IN 
%nonassoc ASS_DES
%nonassoc WHILE LET IF

%right NOT_
%left AND_ OR_

%left PLUS MINUS
%left TIMES DIV
%right POW
%left UMINUS

%%

input:    
	line { root = $1; }
	| lines_block { root = $1; }
    ;

lines_block:
	LKEY lines RKEY { $$ = $2; }
	;

lines:
	/* empty */	 { $$ = new BlockNode({}); }
	| non_empty_lines { $$ = $1; }
	;

non_empty_lines:
	line { $$ = new BlockNode({$1}); }
	| non_empty_lines line { $1->add_child($2); $$ = $1; }
	;

line:
	expr SEMICOLON { $$ = $1; }
	| func_asign SEMICOLON { $$ = $1; } 
	| type_node_decl { root = $1; }
	;

expr: 
	arit_op { $$ = $1; }
	| bool_expr { $$ = $1; }
	| WHILE while_expr { $$ = $2; }
//	| for_expr { $$ = $1; }
	| STRING { $$ = new StringNode($1); }
	| id_expr %prec ASS_DES  { $$ = $1; }
	| func_call { $$ = $1; }
	| let_assign { $$ = $1; }
	| IF conditional { $$ = $2; }
	| member_access_expr { $$ = $1; }
	| expr ASS_DES expr { $$ = new BinOpNode($1, ":=", $3); }
	| expr ASSIGN expr { $$ = new BinOpNode($1, "=", $3); }
    ;

func_asign:
	FUNCTION_ ID_ LPARENT args_list RPARENT INLINE expr  { $$ = new AssignFuncNode(new IDNode($2), $4, $7); }
	| FUNCTION_ ID_ LPARENT args_list RPARENT TWOPOINTS ID_ INLINE expr { $$ = new AssignFuncNode(new IDNode($2), $4, $9, $7); }
	| FUNCTION_ ID_ LPARENT args_list RPARENT LKEY lines RKEY { $$ = new AssignFuncNode(new IDNode($2), $4, $7); }
	| FUNCTION_ ID_ LPARENT args_list RPARENT TWOPOINTS ID_ LKEY lines RKEY { $$ = new AssignFuncNode(new IDNode($2), $4, $9, $7); } 
	;

args_list:
	/* empty */ { $$ = new ArgsList({}); }
	| id_expr { $$ = new ArgsList({$1}); }
	| args_list COLON id_expr { $1->add_child($3); $$ = $1; }
	;

id_expr:
	ID_ TWOPOINTS ID_ { $$ = new IDNode($1, $3); }
	| ID_ { $$ = new IDNode($1); }
	;

let_assign:
	LET var_assign_list IN expr { $$ = new LetAssign($2->assigns, $4); }
	| LET var_assign_list IN lines_block { $$ = new LetAssign($2->assigns, $4); }
	;

var_assign_list:
	id_expr ASSIGN expr { $$ = new VarAssignList({ new VarAssign($1, $3) }); }
	| id_expr ASSIGN expr AS_ ID_ { $$ = new VarAssignList({ new VarAssign($1, $3, $5) }); }
	| id_expr ASSIGN new_expr { $$ = new VarAssignList({ new VarAssign($1, $3)}); }
	| var_assign_list COLON id_expr ASSIGN expr { $1->add_child(new VarAssign($3, $5)); $$ = $1; }
	| var_assign_list COLON id_expr ASSIGN expr AS_ ID_ { $1->add_child(new VarAssign($3, $5, $7)); $$ = $1; }
	| var_assign_list COLON id_expr ASSIGN new_expr { $1->add_child(new VarAssign($3, $5)); $$ = $1; }
	;

new_expr:
	NEW ID_ LPARENT expr_list RPARENT { $$ = new NewTypeNode($2, $4->children); }
	;

expr_list:
	/* empty */ { $$ = new ASTNodeVector({}); }
	| expr { $$ = new ASTNodeVector({$1}); }
	| new_expr { $$ = new ASTNodeVector({$1}); }
	| expr_list COLON expr { $1->add_child($3); $$ = $1; }
	| expr_list COLON new_expr { $1->add_child($3); $$ = $1; }
	;

func_call:
	ID_ LPARENT expr_list RPARENT { $$ = new FunctionCallNode($1, $3); }
	;

arit_op:
	NUMBER { $$ = new FloatNode($1); }
	| LPARENT expr RPARENT { $$ = $2; }
	| expr PLUS expr { $$ = new BinOpNode( $1, "+", $3); }
	| expr MINUS expr { $$ = new BinOpNode( $1, "-", $3); }
	| expr TIMES expr { $$ = new BinOpNode( $1, "*", $3); }
	| expr DIV expr { $$ = new BinOpNode( $1, "/", $3); }
	| expr POW expr { $$ = new BinOpNode( $1, "^", $3); }
	| MINUS expr %prec UMINUS { $$ = new UnaryOpNode("-", $2); }
	;

bool_expr:
	BOOLEAN { $$ = new BoolExprNode(new BoolNode($1)); }
	| expr GREATER_EQUAL expr { $$ = new BoolExprNode(new BinOpNode($1, ">=", $3)); }
	| expr GREATER expr { $$ = new BoolExprNode(new BinOpNode($1, ">", $3)); }
	| expr LESS_EQUAL expr { $$ = new BoolExprNode(new BinOpNode($1, "<=", $3)); }
	| expr LESS expr { $$ = new BoolExprNode(new BinOpNode($1, "<", $3)); }
	| expr EQUAL expr { $$ = new BoolExprNode(new BinOpNode($1, "==", $3)); }
	| expr DISTINCT expr { $$ = new BoolExprNode(new BinOpNode($1, "!=", $3)); }
	| bool_expr AND_ bool_expr { $$ = new BoolExprNode(new BinOpNode($1, "&", $3)); }
	| bool_expr OR_ bool_expr { $$ = new BoolExprNode(new BinOpNode($1, "|", $3)); }
	| NOT_ bool_expr { $$ = new BoolExprNode(new UnaryOpNode("!", $2)); }
	| id_expr IS ID_ { $$ = new BoolExprNode(new BinOpNode($1, "is", new IDNode($3))); }
	| func_call IS ID_ { $$ = new BoolExprNode(new BinOpNode($1, "is", new IDNode($3))); }
	;

conditional:
	LPARENT bool_expr RPARENT expr ELSE expr { $$ = new Conditional($2, $4, $6); }
	| LPARENT bool_expr RPARENT lines_block ELSE expr { $$ = new Conditional($2, $4, $6); }
	| LPARENT bool_expr RPARENT expr ELSE lines_block { $$ = new Conditional($2, $4, $6); }
	| LPARENT bool_expr RPARENT lines_block ELSE lines_block { $$ = new Conditional($2, $4, $6); }
	| LPARENT bool_expr RPARENT expr ELIF conditional { $$ = new Conditional($2, $4, $6); }
	| LPARENT bool_expr RPARENT lines_block ELIF conditional { $$ = new Conditional($2, $4, $6); }
	;

while_expr:
	LPARENT bool_expr RPARENT lines_block { $$ = new WhileNode($2, $4); }
	| LPARENT bool_expr RPARENT expr { $$ = new WhileNode($2, $4); }
	;

//for_expr:
//	FOR LPARENT ID_ IN func_call RPARENT expr { $$ = new ForNode(new IDNode($3), $5, $7); }
//	| FOR LPARENT ID_ IN func_call RPARENT lines_block { $$ = new ForNode(new IDNode($3), $5, $7); }
//	| FOR LPARENT ID_ IN ID_ RPARENT expr { $$ = new ForNode(new IDNode($3), new IDNode($5), $7); }
//	| FOR LPARENT ID_ IN ID_ RPARENT lines_block { $$ = new ForNode(new IDNode($3), new IDNode($5), $7); }
//	;
//	//pendiente que acepte argumentos de cualquier tipo
//
type_node_decl:
	TYPE ID_ LKEY type_body_elements RKEY { $$ = new TypeDeclNode(new IDNode($2), new ArgsList({}), $4->children); }
	| TYPE ID_ LPARENT args_list RPARENT LKEY type_body_elements RKEY { $$ = new TypeDeclNode(new IDNode($2), $4, $7->children); }
	| TYPE ID_ LPARENT args_list RPARENT INHERITS ID_ LKEY type_body_elements RKEY { $$ = new TypeDeclNode(new IDNode($2), $4, $9->children, {$7}); }
	| TYPE ID_ LPARENT args_list RPARENT INHERITS ID_ LPARENT args_list RPARENT LKEY type_body_elements RKEY { $$ = new TypeDeclNode(new IDNode($2), $4, $12->children, {$7}, $9); }
	;

type_body_elements:
	/* empty */ { $$ = new ASTNodeVector({}); }
	| type_body_elements attribute { $1->add_child($2); $$ = $1; }
	| type_body_elements method { $1->add_child($2); $$ = $1; }
	;

attribute:
	id_expr ASSIGN expr SEMICOLON { $$ = new VarAssign($1, $3); }
	| id_expr ASSIGN expr AS_ ID_ SEMICOLON { $$ = new VarAssign($1, $3, $5); }
	;

method:
	ID_ LPARENT args_list RPARENT INLINE expr SEMICOLON { $$ = new AssignFuncNode(new IDNode($1), $3, $6); }
	| ID_ LPARENT args_list RPARENT TWOPOINTS ID_ INLINE expr SEMICOLON { $$ = new AssignFuncNode(new IDNode($1), $3, $8, $6); } 
	| ID_ LPARENT args_list RPARENT LKEY lines RKEY { $$ = new AssignFuncNode(new IDNode($1), $3, $6); }
	| ID_ LPARENT args_list RPARENT TWOPOINTS ID_ LKEY lines RKEY { $$ = new AssignFuncNode(new IDNode($1), $3, $8, $6); }
	;

member_access_expr:
	ID_ ACCESS ID_ { $$ = new AccessNode($1, new AttributeMember($3)); }
	| ID_ ACCESS ID_ LPARENT expr_list RPARENT { $$ = new AccessNode($1, new MethodMember($3, $5->children)); } 
	;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}