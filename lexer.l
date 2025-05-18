%{
#include "parser.tab.h"
#include <stdlib.h>
extern "C"  {
	int yylex(void);
	void yyerror(const char *);
} 

%}

%%

"true"		{ yylval.boolean = true; return BOOLEAN; }
"false"		{ yylval.boolean = false; return BOOLEAN; }
"function" 	{ return FUNCTION; }

\"([^"\n]|\\.)*\"	  {yylval.str = strdup(yytext); return STRING; }
[a-zA-Z_][a-zA-Z0-9]* { yylval.str = strdup(yytext); return ID; }

[0-9]+      { yylval.num = atoi(yytext); return NUMBER; }
"+"         { return PLUS; }
"-"			{ return MINUS; }
"*"			{ return TIMES; }
"/"			{ return DIV; }
"^"			{ return POW; }
"("			{ return LPARENT; }
")" 		{ return RPARENT; }		
";"			{ return SEMICOLON; }
","			{ return COLON; }
"{"			{ return LKEY; }
"}"			{ return RKEY; }
"=>"		{ return INLINE; }
[ \t\n]     ; /* skip whitespace */
.           { yyerror("Caracter invalido"); }

%%

int yywrap() {
    return 1;
}