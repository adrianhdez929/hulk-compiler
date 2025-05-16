%{
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "../ast.hpp"

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
}

%token NUMBER
%token BOOLEAN
%token STRING
%token ID
%token PLUS MINUS TIMES DIV POW LPARENT RPARENT SEMICOLON

%token <num> NUMBER
%token <str> ID
%token <str> STRING
%token <boolean> BOOLEAN

%type <node> expr func_call

%left PLUS
%left MINUS
%left TIMES
%left DIV
%right POW

%%

input:    
	/* empty */
    | input expr SEMICOLON { root = $2; }
    ;

expr: 
	NUMBER { $$ = new FloatNode($1); }
	| BOOLEAN { $$ = new BoolNode($1); }
	| STRING { $$ = new StringNode($1); }
	| ID { $$ = new IDNode($1); }

	| func_call { $$ = $1; }

	| expr PLUS expr { $$ = new BinOpNode( $1, "+", $3); }
	| expr MINUS expr { $$ = new BinOpNode( $1, "-", $3); }
	| expr TIMES expr { $$ = new BinOpNode( $1, "*", $3); }
	| expr DIV expr { $$ = new BinOpNode( $1, "/", $3); }
	| expr POW expr { $$ = new BinOpNode( $1, "^", $3); }
	| LPARENT expr RPARENT { $$ = $2; }
    ;

func_call:
	ID LPARENT expr RPARENT { $$ = new FunctionNode($1, $3); free($1); }
	;


%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}