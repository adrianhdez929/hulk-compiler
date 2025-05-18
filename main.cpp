#include <stdio.h>
#include "build/parser.h"
#include "ast.hpp"
#include <iostream>

extern FILE *yyin;
extern int yyparse();
ASTNode* root = nullptr;

int main(int argc, char **argv) {
	const char* filename = "script.txt"; //default

    if (argc > 1) {
		filename = argv[1];
    }
		
	yyin = fopen(filename, "r");

	if (!yyin) {
		std::cerr << "Error: No se pudo abrir el archivo " << std::endl;
		return 1;
	}

    yyparse();

	if (root) {
		std::cout << "Arbol de Sintaxis Abstracta:" << std::endl; 
		root->print();
		delete root;
	}

	fclose(yyin);

    return 0;
}