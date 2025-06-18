#include <stdio.h>
// #include "hulk/parser.tab.h"
#include "Ast/ast.hpp"
#include "semantic/visitor.h"
#include "semantic/type_collector_visitor.h"
#include "semantic/symbol_collector_visitor.h"
#include "semantic/context.h"
#include "codegen/visitor.h"
#include <iostream>
#include <fstream>
#include <exception>
#include "test.h"
#include "execute.h"

using namespace std;

ASTNode* parseScript(); // Forward declaration of parseScript function from execute.cpp

// // using namespace manipulation;

extern FILE *yyin;
extern int yyparse();
ASTNode* root = nullptr;

// Create the new two-pass semantic analysis system
TypeCollectorVisitor* typeCollectorVisitor = new TypeCollectorVisitor();
SymbolCollectorVisitor* symbolCollectorVisitor = nullptr; // Will be initialized after type collection
SemanticCheckerVisitor* semanticVisitor = new SemanticCheckerVisitor();
CodegenVisitor* codegenVisitor = new CodegenVisitor();

int main(int argc, char* argv[]) {
    std::string file_path = "script.hulk"; // Default file path
    
    // Si se proporciona un argumento, utilizarlo como ruta del archivo
    if (argc > 1) {
        file_path = argv[1];
        std::cout << "Usando archivo: " << file_path << std::endl;
    }

    Grammar hulk_grammar = getHulkGrammar();
    // if (!std::filesystem::exists("hulk/lexer.l") || !std::filesystem::exists("hulk/parser.p")) {
    //     bool success = create_artifacts(hulk_grammar, false); // false for no verbose output
    //     if (!success) {
    //         std::cerr << "Error al crear los artefactos" << std::endl;
    //         return 1;
    //     }
    // }

    std::string error_message;
    std::string script_content = read_source_file(file_path, error_message);
    if (script_content.empty()) {
        std::cerr << "Error: " << error_message << std::endl;
        return 1;
    }

    root = compile_hulk(script_content, error_message, hulk_grammar, false); // false for no verbose output

    if (!root) {
        std::cerr << "Error al compilar el script: " << error_message << std::endl;
        return 1;
    }

    // Simple test to avoid segfaults
    std::cout << "=== HULK Compiler Starting ===" << std::endl;
    
    // Check if we have an input file
    // const char* filename = "script.hulk"; // default
    // if (argc > 1) {
    //     filename = argv[1];
    //     std::cout << "Using input file: " << filename << std::endl;
    // } else {
    //     std::cout << "Using default input file: " << filename << std::endl;
    // }
    
    // // Check if the file exists
    // std::ifstream test_file(filename);
    // if (!test_file.is_open()) {
    //     std::cerr << "Error: Could not open file " << filename << std::endl;
    //     return 1;
    // }
    
    // // Read file content
    // std::string content((std::istreambuf_iterator<char>(test_file)), 
    //                     std::istreambuf_iterator<char>());
    // test_file.close();
    
    // std::cout << "Successfully read file content (" << content.length() << " chars)" << std::endl;
    // std::cout << "Content preview: " << content.substr(0, 100) << "..." << std::endl;
    
    // std::cout << "=== HULK Compiler Finished Successfully ===" << std::endl;
    // return 0;

    // //region: Flex/Bison setup
    // const char* filename = "script.hulk"; //default
    // if (argc > 1) {
	// 	filename = argv[1];
    // }
	// yyin = fopen(filename, "r");
	// if (!yyin) {
	// 	std::cerr << "Error: No se pudo abrir el archivo " << std::endl;
	// 	return 1;
	// }
    // yyparse();
    // //endregion

    try {
    root = parseScript();
    } catch (const std::exception& e) {
        std::cerr << "Error al parsear el script: " << e.what() << std::endl;
        return 1;
    }

    if (!root) {
        std::cerr << "Error: No se pudo construir el AST" << std::endl;
        fclose(yyin);
        return 1;
    }

    std::cout << "Arbol de Sintaxis Abstracta:" << std::endl; 
    root->print();
    //     std::cout << "\n=== Starting Two-Pass Semantic Analysis ===" << std::endl;
        
    //     // Create a global context for semantic analysis (with null parent for root context)
    Context* globalContext = new Context(nullptr);
       
    // //     // Define built-in functions and variables
    // //     globalContext->define("print", 1);  // print function takes 1 argument
    // //     globalContext->define("pi");         // pi constant (variable)
    //     // PASS 1: Type Collection
    //     std::cout << "\n--- Pass 1: Collecting Type Definitions ---" << std::endl;
        root->accept(typeCollectorVisitor, globalContext);
        
        if (typeCollectorVisitor->hasErrors()) {
            std::cerr << "Type collection failed with the following errors:" << std::endl;
            typeCollectorVisitor->printErrors();
        }
        
    //     std::cout << "Type collection completed successfully." << std::endl;
    //     std::cout << "Collected " << typeCollectorVisitor->getCollectedTypes().size() << " types" << std::endl;

        globalContext->localTypes = typeCollectorVisitor->getCollectedTypes();

        
    //     // PASS 2: Symbol Collection
    //     std::cout << "\n--- Pass 2: Collecting Symbols and Type Inference ---" << std::endl;
        symbolCollectorVisitor = new SymbolCollectorVisitor(globalContext);
        root->accept(symbolCollectorVisitor, globalContext);
        
        if (symbolCollectorVisitor->hasErrors()) {
            std::cerr << "Symbol collection failed with the following errors:" << std::endl;
            symbolCollectorVisitor->printErrors();
        }
        
    //     std::cout << "Symbol collection completed successfully." << std::endl;
        
    //     // Store collected symbols in the global context
    //     std::cout << "Storing collected symbols in context..." << std::endl;
    //     std::cout << "Symbol data successfully stored in context." << std::endl;
        
    //     // Display collected symbols
        globalContext->methodsByType = symbolCollectorVisitor->getMethodsByType();
        globalContext->attributesByType = symbolCollectorVisitor->getAttributesByType();
        globalContext->globalVariables = symbolCollectorVisitor->getGlobalVariables();
        
    //     if (!globalContext->methodsByType.empty() || !globalContext->attributesByType.empty() || !globalContext->globalVariables.empty()) {
    //         std::cout << "\n=== Collected Symbols Summary ===" << std::endl;
            
    //         // Show methods by type
    //         for (const auto& pair : globalContext->methodsByType) {
    //             std::cout << "Type '" << pair.first << "' methods:" << std::endl;
    //             for (const auto& method : pair.second) {
    //                 std::cout << "  - " << method.name << "(";
    //                 for (size_t i = 0; i < method.paramNames.size(); ++i) {
    //                     if (i > 0) std::cout << ", ";
    //                     std::cout << method.paramNames[i];
    //                     if (method.paramTypes[i]) {
    //                         std::cout << ": " << method.paramTypes[i]->name;
    //                     }
    //                 }
    //                 std::cout << ") -> " << (method.returnType ? method.returnType->name : "void") << std::endl;
    //             }
    //         }
            
    //         // Show attributes by type
    //         for (const auto& pair : globalContext->attributesByType) {
    //             std::cout << "Type '" << pair.first << "' attributes:" << std::endl;
    //             for (const auto& attr : pair.second) {
    //                 std::cout << "  - " << attr.name << ": " << (attr.type ? attr.type->name : "unknown") << std::endl;
    //             }
    //         }
            
    //         // Show global variables
    //         if (!globalContext->globalVariables.empty()) {
    //             std::cout << "Global variables:" << std::endl;
    //             for (const auto& var : globalContext->globalVariables) {
    //                 std::cout << "  - " << var.name << ": " << (var.type ? var.type->name : "unknown") << std::endl;
    //             }
    //         }
    //     }
        
    //     // PASS 3: Final Semantic Analysis (Optional - for additional validation)
    //     std::cout << "\n--- Pass 3: Final Semantic Validation ---" << std::endl;
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
        
    //     std::cout << "All semantic analysis passes completed successfully!" << std::endl;

    //     std::cout << "\n=== Generating Code ===" << std::endl;
    //     // Initialize the codegen visitor
    //     codegenVisitor->initialize();
        
    // //     // Set the root node and context
    // //     codegenVisitor->setRootNode(root, globalContext);
        
    // //     // Generate and execute code
    // //     codegenVisitor->generateCode();
    // //     std::cout << "Code generation completed." << std::endl;
    //     // Clean up
    //     delete symbolCollectorVisitor;
    //     delete globalContext;
	// 	    delete root;
	// }

	// // fclose(yyin);

    return 0;
}
