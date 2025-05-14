#include <stdio.h>
#include "build/parser.tab.h"
#include "ast.hpp"
#include <iostream>

extern FILE *yyin;
extern int yyparse();
ASTNode* root = nullptr;

int main(int argc, char **argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
    }
    yyparse();

	if (root) {
		std::cout << "Arbol de Sintaxis Abstracta:" << std::endl; 
		root->print();
		delete root;
    
    return 0;
	}

    return 1;
}
