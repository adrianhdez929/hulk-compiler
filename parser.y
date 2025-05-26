%{
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
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
	class AssignFuncNode* ass_f_node;
}

%token NUMBER
%token BOOLEAN
%token STRING
%token ID
%token PLUS MINUS TIMES DIV POW LPARENT RPARENT SEMICOLON COLON LKEY RKEY FUNCTION INLINE

%token <num> NUMBER
%token <str> ID
%token <str> STRING
%token <boolean> BOOLEAN

%type <node> expr func_call arit_op lines_block line
%type <b_node> lines
%type <args_l> args_list
%type <ass_f_node> func_asign


%left PLUS
%left MINUS
%left TIMES
%left DIV
%right POW

%%

input:    
	line { root = $1; }
	| lines_block { root = $1; }
	| func_asign SEMICOLON { root = $1; }
	
    ;

lines_block:
	LKEY lines RKEY { $$ = $2; }
	;

lines:
	/* empty */	 { $$ = new BlockNode({}); }
	| line { $$ = new BlockNode({$1}); }
	| lines line { $1->add_child($2); $$ = $1; }
	;

line:
	expr SEMICOLON { $$ = $1; }

	;

expr: 
	arit_op { $$ = $1; }
	| BOOLEAN { $$ = new BoolNode($1); }
	| STRING { $$ = new StringNode($1); }
	| ID { $$ = new IDNode($1); }
	| func_call { $$ = $1; }
    ;

func_asign:
	FUNCTION ID LPARENT args_list RPARENT INLINE expr  { $$ = new AssignFuncNode(new IDNode($2), $4, $7); }
	| FUNCTION ID LPARENT args_list RPARENT LKEY lines RKEY { $$ = new AssignFuncNode(new IDNode($2), $4, $7); }
	;

args_list:
	/* empty */ { $$ = new ArgsList({}); }
	| ID { $$ = new ArgsList({new IDNode($1)}); }
	| args_list COLON ID { $1->add_child(new IDNode($3)); $$ = $1; }
	;

func_call:
	ID LPARENT expr RPARENT { $$ = new FunctionNode($1, $3); free($1); }
	;

arit_op:
	NUMBER { $$ = new FloatNode($1); }
	| expr PLUS expr { $$ = new BinOpNode( $1, "+", $3); }
	| expr MINUS expr { $$ = new BinOpNode( $1, "-", $3); }
	| expr TIMES expr { $$ = new BinOpNode( $1, "*", $3); }
	| expr DIV expr { $$ = new BinOpNode( $1, "/", $3); }
	| expr POW expr { $$ = new BinOpNode( $1, "^", $3); }
	| LPARENT expr RPARENT { $$ = $2; }
	;





%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}