#include <stdio.h>
#include "build/parser.tab.h"
#include "Ast/ast.hpp"
#include "semantic/visitor.h"
#include <iostream>

extern FILE *yyin;
extern int yyparse();
ASTNode* root = nullptr;

SemanticCheckerVisitor* visitor = new SemanticCheckerVisitor();

int main(int argc, char **argv) {
    if (argc > 1) {
        yyin = fopen(argv[1], "r");
    }
    yyparse();

	if (root) {
		std::cout << "Arbol de Sintaxis Abstracta:" << std::endl; 
		root->print();

        std::cout << "Visiting AST with SemanticCheckerVisitor" << std::endl;
        root->accept(visitor, nullptr);
        std::cout << "Semantic check completed." << std::endl;

		delete root;
    
    return 0;
	}

    return 1;
}
