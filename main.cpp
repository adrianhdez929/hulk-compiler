#include <stdio.h>
#include "build/parser.tab.h"
#include "Ast/ast.hpp"
#include "semantic/visitor.h"
#include "codegen/visitor.h"
#include <iostream>
#include "test.h"

using namespace std;
// using namespace manipulation;

extern FILE *yyin;
extern int yyparse();
ASTNode* root = nullptr;

SemanticCheckerVisitor* visitor = new SemanticCheckerVisitor();
CodegenVisitor* codegenVisitor = new CodegenVisitor();

int main() {
    //run tests
    execute_test();
    
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

        std::cout << "Visiting AST with SemanticCheckerVisitor" << std::endl;
        root->accept(visitor, nullptr);
        std::cout << "Semantic check completed." << std::endl;

        std::cout << "Generating code with CodegenVisitor" << std::endl;
        root->accept(codegenVisitor, nullptr);
        std::cout << "Code generation completed." << std::endl;

		delete root;
	}

	fclose(yyin);

    return 0;
}
