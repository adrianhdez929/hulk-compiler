%{
#include "parser.tab.h"
#include <stdlib.h>
extern "C"  {
	int yylex(void);
	void yyerror(const char *);
} 

%}

%%

[0-9]+      { yylval.num = atoi(yytext); return NUMBER; }
"+"         { return PLUS; }
"-"			{ return MINUS; }
[ \t\n]     ; /* skip whitespace */
.           { yyerror("Caracter invalido"); }

%%

int yywrap() {
    return 1;
}