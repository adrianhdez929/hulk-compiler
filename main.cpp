#include <stdio.h>
#include "build/parser.h"
#include "Ast/ast.hpp"
//#include "semantic/visitor.h"
#include <iostream>

extern FILE *yyin;
extern int yyparse();
ASTNode* root = nullptr;

SemanticCheckerVisitor* visitor = new SemanticCheckerVisitor();

int main(int argc, char **argv) {
	const char* filename = "script.txt"; //default
		
	yyin = fopen(filename, "r");

	if (!yyin) {
		std::cerr << "Error: No se pudo abrir el archivo " << std::endl;
		return 1;
	}

    yyparse();

	if (root) {
		std::cout << "Arbol de Sintaxis Abstracta:" << std::endl; 
		root->print();

        //std::cout << "Visiting AST with SemanticCheckerVisitor" << std::endl;
        //root->accept(visitor, nullptr);
        //std::cout << "Semantic check completed." << std::endl;

		delete root;
	}

	fclose(yyin);

    return 0;
}
