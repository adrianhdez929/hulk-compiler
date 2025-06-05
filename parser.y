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
}

%token NUMBER
%token BOOLEAN
%token STRING
%token ID
%token PLUS MINUS TIMES DIV POW LPARENT RPARENT SEMICOLON COLON LKEY RKEY FUNCTION INLINE ASSIGN ASS_DES IF ELSE ELIF WHILE FOR ACCESS UMINUS TWOPOINTS NEW
%token GREATER_EQUAL GREATER LESS_EQUAL LESS EQUAL DISTINCT 
%token LET
%token IN
%token TYPE

%token <num> NUMBER
%token <str> ID
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
%type <v_ass_t> var_ass_type


%nonassoc GREATER_EQUAL GREATER LESS_EQUAL LESS EQUAL DISTINCT
%nonassoc ELSE ELIF
%nonassoc ASSIGN IN ASS_DES
%nonassoc WHILE LET IF

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
	| while_expr { $$ = $1; }
//	| for_expr { $$ = $1; }
	| STRING { $$ = new StringNode($1); }
	| id_expr { $$ = $1; }
	| func_call { $$ = $1; }
	| let_assign { $$ = $1; }
	| var_ass_type { $$ = $1; }
	| id_expr ASS_DES expr { $$ = new VarDesAssign($1, $3); }
	| IF conditional { $$ = $2; }
	| member_access_expr { $$ = $1; }
	| expr ASS_DES expr { $$ = new BinOpNode($1, ":=", $3); }
	| expr ASSIGN expr { $$ = new BinOpNode($1, "=", $3); }
    ;

func_asign:
	FUNCTION ID LPARENT args_list RPARENT INLINE expr  { $$ = new AssignFuncNode(new IDNode($2), $4, $7); }
	| FUNCTION ID LPARENT args_list RPARENT LKEY lines RKEY { $$ = new AssignFuncNode(new IDNode($2), $4, $7); }
	;

args_list:
	/* empty */ { $$ = new ArgsList({}); }
	| id_expr { $$ = new ArgsList({$1}); }
	| args_list COLON id_expr { $1->add_child($3); $$ = $1; }
	;

id_expr:
	ID TWOPOINTS ID { $$ = new IDNode($1, $3); }
	| ID { $$ = new IDNode($1); }
	;

let_assign:
	LET var_assign_list IN expr { $$ = new LetAssign($2->assigns, $4); }
	| LET var_assign_list IN lines_block { $$ = new LetAssign($2->assigns, $4); }
	;

var_ass_type:
	LET ID ASSIGN NEW ID LPARENT args_list RPARENT IN expr { $$ = new VarAssignType($2, new IDNode($5), $10); }
	;

var_assign_list:
	id_expr ASSIGN expr { $$ = new VarAssignList({ new VarAssign($1, $3) });}
	| var_assign_list COLON id_expr ASSIGN expr { $1->add_child(new VarAssign($3, $5)); $$ = $1; }
	;

expr_list:
	/* empty */ { $$ = new ASTNodeVector({}); }
	| expr { $$ = new ASTNodeVector({$1}); }
	| expr_list COLON expr { $1->add_child($3); $$ = $1; }
	;

func_call:
	ID LPARENT expr_list RPARENT { $$ = new FunctionNode($1, $3); free($1); }
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
	;

conditional:
	LPARENT bool_expr RPARENT expr ELSE expr { $$ = new Conditional($2, $4, $6); }
	| LPARENT bool_expr RPARENT lines_block ELSE expr { $$ = new Conditional($2, $4, $6); }
	| LPARENT bool_expr RPARENT expr ELSE lines_block { $$ = new Conditional($2, $4, $6); }
	| LPARENT bool_expr RPARENT lines_block ELSE lines_block { $$ = new Conditional($2, $4, $6); }
	| LPARENT bool_expr RPARENT expr ELIF conditional { $$ = new Conditional($2, $4, $6); }
	;

while_expr:
	WHILE LPARENT bool_expr RPARENT lines_block { $$ = new WhileNode($3, $5); }
	| WHILE LPARENT bool_expr RPARENT expr { $$ = new WhileNode($3, $5); }
	;

//for_expr:
//	FOR LPARENT ID IN func_call RPARENT expr { $$ = new ForNode(new IDNode($3), $5, $7); }
//	| FOR LPARENT ID IN func_call RPARENT lines_block { $$ = new ForNode(new IDNode($3), $5, $7); }
//	| FOR LPARENT ID IN ID RPARENT expr { $$ = new ForNode(new IDNode($3), new IDNode($5), $7); }
//	| FOR LPARENT ID IN ID RPARENT lines_block { $$ = new ForNode(new IDNode($3), new IDNode($5), $7); }
//	;
//	//pendiente que acepte argumentos de cualquier tipo
//
type_node_decl:
	TYPE ID args_list LKEY type_body_elements RKEY { $$ = new TypeDeclNode(new IDNode($2), $3, $5->children); }
	;

type_body_elements:
	/* empty */ { $$ = new ASTNodeVector({}); }
	| type_body_elements attribute { $1->add_child($2); $$ = $1; }
	| type_body_elements method { $1->add_child($2); $$ = $1; }
	;

attribute:
	id_expr ASSIGN expr SEMICOLON { $$ = new VarAssign($1, $3); }
	;

method:
	ID LPARENT args_list RPARENT INLINE expr SEMICOLON { $$ = new AssignFuncNode(new IDNode($1), $3, $6); }
	| ID LPARENT args_list RPARENT LKEY lines RKEY { $$ = new AssignFuncNode(new IDNode($1), $3, $6); }
	;

member_access_expr:
	ID ACCESS ID { $$ = new AccessNode($1, new AttributeMember($3)); }
	| ID ACCESS ID LPARENT args_list RPARENT { $$ = new AccessNode($1, new MethodMember($3, $5)); } // por ahora solo acepta los argumentos como ID
	;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}