#include <stdio.h>
// #include "build/parser.tab.h"
#include "Ast/ast.hpp"
#include "semantic/visitor.h"
#include <iostream>
#include "test.h"
#include "Lexer/lexer_grammar.h"
#include "Lexer/grammar_parser.h"

using namespace std;
// using namespace manipulation;

//NO BORRAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAARRR
// extern FILE *yyin;
// extern int yyparse();
// ASTNode* root = nullptr;

// SemanticCheckerVisitor* visitor = new SemanticCheckerVisitor();

int main() {
    //run tests
    execute_test();
	// Grammar g = GrammarParser::Parse("Lexer/grammar.txt");
	// cout << "Lexer Grammar:" << endl;
	// cout << g.ToString() << endl;



    
    //NOTE: NO BORRAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAARRR
    // const char* filename = "script.txt"; //default

    // if (argc > 1) {
	// 	filename = argv[1];
    // }
		
	// yyin = fopen(filename, "r");

	// if (!yyin) {
	// 	std::cerr << "Error: No se pudo abrir el archivo " << std::endl;
	// 	return 1;
	// }

    // yyparse();

	// if (root) {
	// 	std::cout << "Arbol de Sintaxis Abstracta:" << std::endl; 
	// 	root->print();

    //     std::cout << "Visiting AST with SemanticCheckerVisitor" << std::endl;
    //     root->accept(visitor, nullptr);
    //     std::cout << "Semantic check completed." << std::endl;

	// 	delete root;
	// }

	// fclose(yyin);

    return 0;
}
