#include <stdio.h>
#include "hulk/parser.tab.h"
#include "Ast/ast.hpp"
#include "semantic/visitor.h"
#include "semantic/type_collector_visitor.h"
#include "semantic/symbol_collector_visitor.h"
#include "semantic/context.h"
// #include "codegen/visitor.h"
#include <iostream>
#include <exception>
#include "test.h"

using namespace std;
// // using namespace manipulation;

// extern FILE *yyin;
// extern int yyparse();
// ASTNode* root = nullptr;

// Create the new two-pass semantic analysis system
TypeCollectorVisitor* typeCollectorVisitor = new TypeCollectorVisitor();
SymbolCollectorVisitor* symbolCollectorVisitor = nullptr; // Will be initialized after type collection
SemanticCheckerVisitor* semanticVisitor = new SemanticCheckerVisitor();
CodegenVisitor* codegenVisitor = new CodegenVisitor();

int main(int argc, char* argv[]) {
    //run tests
    execute_test();
    
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
        std::cout << "\n=== Starting Two-Pass Semantic Analysis ===" << std::endl;
        
        // Create a global context for semantic analysis (with null parent for root context)
        Context* globalContext = new Context(nullptr);
       
    //     // Define built-in functions and variables
    //     globalContext->define("print", 1);  // print function takes 1 argument
    //     globalContext->define("pi");         // pi constant (variable)
        // PASS 1: Type Collection
        std::cout << "\n--- Pass 1: Collecting Type Definitions ---" << std::endl;
        root->accept(typeCollectorVisitor, globalContext);
        
        if (typeCollectorVisitor->hasErrors()) {
            std::cerr << "Type collection failed with the following errors:" << std::endl;
            typeCollectorVisitor->printErrors();
        }
        
        std::cout << "Type collection completed successfully." << std::endl;
        std::cout << "Collected " << typeCollectorVisitor->getCollectedTypes().size() << " types" << std::endl;

        globalContext->localTypes = typeCollectorVisitor->getCollectedTypes();

        
        // PASS 2: Symbol Collection
        std::cout << "\n--- Pass 2: Collecting Symbols and Type Inference ---" << std::endl;
        symbolCollectorVisitor = new SymbolCollectorVisitor(globalContext);
        root->accept(symbolCollectorVisitor, globalContext);
        
        if (symbolCollectorVisitor->hasErrors()) {
            std::cerr << "Symbol collection failed with the following errors:" << std::endl;
            symbolCollectorVisitor->printErrors();
        }
        
        std::cout << "Symbol collection completed successfully." << std::endl;
        
        // Store collected symbols in the global context
        std::cout << "Storing collected symbols in context..." << std::endl;
        std::cout << "Symbol data successfully stored in context." << std::endl;
        
        // Display collected symbols
        globalContext->methodsByType = symbolCollectorVisitor->getMethodsByType();
        globalContext->attributesByType = symbolCollectorVisitor->getAttributesByType();
        globalContext->globalVariables = symbolCollectorVisitor->getGlobalVariables();
        
        if (!globalContext->methodsByType.empty() || !globalContext->attributesByType.empty() || !globalContext->globalVariables.empty()) {
            std::cout << "\n=== Collected Symbols Summary ===" << std::endl;
            
            // Show methods by type
            for (const auto& pair : globalContext->methodsByType) {
                std::cout << "Type '" << pair.first << "' methods:" << std::endl;
                for (const auto& method : pair.second) {
                    std::cout << "  - " << method.name << "(";
                    for (size_t i = 0; i < method.paramNames.size(); ++i) {
                        if (i > 0) std::cout << ", ";
                        std::cout << method.paramNames[i];
                        if (method.paramTypes[i]) {
                            std::cout << ": " << method.paramTypes[i]->name;
                        }
                    }
                    std::cout << ") -> " << (method.returnType ? method.returnType->name : "void") << std::endl;
                }
            }
            
            // Show attributes by type
            for (const auto& pair : globalContext->attributesByType) {
                std::cout << "Type '" << pair.first << "' attributes:" << std::endl;
                for (const auto& attr : pair.second) {
                    std::cout << "  - " << attr.name << ": " << (attr.type ? attr.type->name : "unknown") << std::endl;
                }
            }
            
            // Show global variables
            if (!globalContext->globalVariables.empty()) {
                std::cout << "Global variables:" << std::endl;
                for (const auto& var : globalContext->globalVariables) {
                    std::cout << "  - " << var.name << ": " << (var.type ? var.type->name : "unknown") << std::endl;
                }
            }
        }
        
        // PASS 3: Final Semantic Analysis (Optional - for additional validation)
        std::cout << "\n--- Pass 3: Final Semantic Validation ---" << std::endl;
        root->accept(semanticVisitor, globalContext);
        
        if (semanticVisitor->hasErrors()) {
            std::cerr << "Final semantic analysis failed with the following errors:" << std::endl;
            semanticVisitor->printErrors();
            delete symbolCollectorVisitor;
            delete globalContext;
            delete root;
            fclose(yyin);
            return 1;
        }
        
        std::cout << "All semantic analysis passes completed successfully!" << std::endl;

        std::cout << "\n=== Generating Code ===" << std::endl;
        // Initialize the codegen visitor
        codegenVisitor->initialize();
        
    //     // Set the root node and context
    //     codegenVisitor->setRootNode(root, globalContext);
        
    //     // Generate and execute code
    //     codegenVisitor->generateCode();
    //     std::cout << "Code generation completed." << std::endl;
        // Clean up
        delete symbolCollectorVisitor;
        delete globalContext;
		    delete root;
	}

	// fclose(yyin);

    return 0;
}
