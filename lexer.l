%{
#include "parser.h"
#include <stdlib.h>
extern "C"  {
	int yylex(void);
	void yyerror(const char *);
} 

%}

%%

"true"		{ yylval.boolean = true; return BOOLEAN; }
"false"		{ yylval.boolean = false; return BOOLEAN; }
"function" 	{ return FUNCTION_; }
"let"		{ return LET; }
"in"		{ return IN; }
"if" 		{ return IF; }
"elif" 		{ return ELIF; }
"else" 		{ return ELSE; }
"while"		{ return WHILE; }
"for" 		{ return FOR; }
"type"		{ return TYPE; }
"new" 		{ return NEW; }
"inherits" 	{ return INHERITS; }
"is"		{ return IS; }
"as"		{ return AS_; }

\"([^"\n]|\\.)*\"	  {yylval.str = strdup(yytext); return STRING; }
[a-zA-Z_][a-zA-Z0-9]* { yylval.str = strdup(yytext); return ID_; }

[0-9]+      { yylval.num = atoi(yytext); return NUMBER; }
"+"         { return PLUS; }
"-"			{ return MINUS; }
"*"			{ return TIMES; }
"/"			{ return DIV; }
"^"			{ return POW; }

"("			{ return LPARENT; }
")" 		{ return RPARENT; }		
";"			{ return SEMICOLON; }
":="		{ return ASS_DES; }
":"			{ return TWOPOINTS; }
","			{ return COLON; }
"{"			{ return LKEY; }
"}"			{ return RKEY; }
"=>"		{ return INLINE; }
"="			{ return ASSIGN; }

">="		{ return GREATER_EQUAL; }
">" 		{ return GREATER; }
"<="		{ return LESS_EQUAL; }
"<"			{ return LESS; }
"=="		{ return EQUAL; }
"!=" 		{ return DISTINCT; }
"."			{ return ACCESS; }

[ \t\n]     ; /* skip whitespace */
.           { yyerror("Caracter invalido"); }

%%

int yywrap() {
    return 1;
}