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
	int num;
	class ASTNode* node; 
}

%token NUMBER
%token PLUS
%token MINUS

%token <num> NUMBER
%type <node> expr

%left PLUS
%left MINUS

%%

input:    
	/* empty */
    | input expr { root = $2; }
    ;

expr: 
	NUMBER { $$ = new FloatNode($1); }
	| expr PLUS expr { $$ = new BinOpNode( $1, "+", $3); }
	| expr MINUS expr { $$ = new BinOpNode( $1, "-", $3); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "Error: %s\n", s);
}