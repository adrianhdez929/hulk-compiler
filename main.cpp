#include <stdio.h>
#include "hulk/parser.tab.h"

#include "Ast/ast.hpp"
#include "semantic/visitor.h"
#include "semantic/context.h"
#include "codegen/visitor.h"
#include <iostream>
#include <exception>
// #include "test.h"
#include "Lexer/grammar_parser.h"


using namespace std;
// using namespace manipulation;

//NO BORRAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAARRR
// extern FILE *yyin;
// extern int yyparse();
// ASTNode* root = nullptr;

SemanticCheckerVisitor* semanticVisitor = new SemanticCheckerVisitor();
CodegenVisitor* codegenVisitor = new CodegenVisitor();

// SemanticCheckerVisitor* visitor = new SemanticCheckerVisitor();


int main(int argc, char* argv[]) {
    //run tests
    // execute_test();

    
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

        std::cout << "Visiting AST with SemanticCheckerVisitor" << std::endl;
        // Create a global context for semantic analysis (with null parent for root context)
        Context* globalContext = new Context(nullptr);
        
        // Define built-in functions and variables
        globalContext->define("print", 1);  // print function takes 1 argument
        globalContext->define("pi");         // pi constant (variable)
        
        try {
            root->accept(semanticVisitor, globalContext);
            std::cout << "Semantic check completed." << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Semantic analysis error: " << e.what() << std::endl;
            delete globalContext;
            delete root;
            fclose(yyin);
            return 1;
        }

        std::cout << "Generating code with CodegenVisitor" << std::endl;
        // Initialize the codegen visitor
        codegenVisitor->initialize();
        
        // Set the root node and context
        codegenVisitor->setRootNode(root, globalContext);
        
        // Generate and execute code
        codegenVisitor->generateCode();
        std::cout << "Code generation completed." << std::endl;

        // Clean up context
        delete globalContext;
		delete root;
	}
    //     std::cout << "Visiting AST with SemanticCheckerVisitor" << std::endl;
    //     root->accept(visitor, nullptr);
    //     std::cout << "Semantic check completed." << std::endl;

	// 	delete root;
	// }

	// fclose(yyin);

    return 0;
}
