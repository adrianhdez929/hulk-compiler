#include <stdio.h>
// #include "hulk/parser.tab.h"
#include "Ast/ast.hpp"
// #include "semantic/visitor.h"
// #include "semantic/type_collector_visitor.h"
// #include "semantic/symbol_collector_visitor.h"
// #include "semantic/context.h"
// #include "codegen/visitor.h"
#include <iostream>
#include <fstream>
#include <exception>
#include "test.h"

using namespace std;
// // using namespace manipulation;

extern FILE *yyin;
extern int yyparse();
ASTNode* root = nullptr;

// Create the new two-pass semantic analysis system
// TypeCollectorVisitor* typeCollectorVisitor = new TypeCollectorVisitor();
// SymbolCollectorVisitor* symbolCollectorVisitor = nullptr; // Will be initialized after type collection
// SemanticCheckerVisitor* semanticVisitor = new SemanticCheckerVisitor();
// CodegenVisitor* codegenVisitor = new CodegenVisitor();

int main(int argc, char* argv[]) {

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
    // //run tests
    // execute_test();
    const char* filename = "script.hulk"; //default
    if (argc > 1) {
		filename = argv[1];
    }
	yyin = fopen(filename, "r");
	if (!yyin) {
		std::cerr << "Error: No se pudo abrir el archivo " << std::endl;
		return 1;
	}
    yyparse();
    // //endregion

    // //region: Custom setup for Hulk language
    // // Cargar Lexer y Parser
    // Lexer* lexer = Lexer::deserialize_lexer("lexer.l");
    // if (!lexer) {
    //     std::cerr << "Error: No se pudo cargar el lexer" << std::endl;
    //     return 1;
    // }

    // // Cargar gramatica de hulk
    // Grammar hulk_grammar = getHulkGrammar();

    // // Cargar Parser
    // SLR1Parser* parser = SLR1Parser::deserialize_parser("hulk_parser.p", hulk_grammar);
    // if (!parser) {
    //     std::cerr << "Error: No se pudo cargar el parser" << std::endl;
    //     return 1;
    // }
    // std::cout << "Artefactos cargados exitosamente." << std::endl;

    // // Cargar script.hulk
    // std::ifstream script_file("script.hulk");
    // if (!script_file.is_open()) {
    //     std::cerr << "Error: No se pudo abrir el archivo script.hulk" << std::endl;
    //     return 1;
    // }
    // std::string script_content((std::istreambuf_iterator<char>(script_file)), std::istreambuf_iterator<char>());
    // script_file.close();
    // std::cout << "Contenido del script.hulk cargado exitosamente." << std::endl;
    
    // // Tokenizar el script
    // auto result = lexer->tokenize(script_content);
    // std::cout << "Tokens generados:" << std::endl;
    // for (const auto& token : result) {
    //     std::cout << "Tipo: " << token.first << ", Valor: " << token.second << std::endl;
    // }
    // if (result.empty()) {
    //     std::cerr << "Error: No se generaron tokens del script" << std::endl;
    //     return 1;
    // }
    // // Retirar los espacios, saltos de linea y tabulaciones
    // result.erase(std::remove_if(result.begin(), result.end(),
    //                              [](const std::pair<std::string, std::string>& token) {
    //                                  return token.second == "space" || token.second == "newline" || token.second == "tab";
    //                              }), result.end());
    // if (result.empty()) {
    //     std::cerr << "Error: No se generaron tokens significativos del script" << std::endl;
    //     return 1;
    // }
    // std::cout << "Tokens significativos generados exitosamente." << std::endl;
    // // Imprimir los tokens generados
    // std::cout << "Tokens generados:" << std::endl;
    // for (const auto& token : result) {
    //     std::cout << "Tipo: " << token.first << ", Valor: " << token.second << std::endl;
    // }

    // std::vector<std::string> token_strings;
    // for (const auto& token : result) {
    //     token_strings.push_back(token.second);
    // }

    // // Parsear los tokens
    // auto parse_result = parser->Parse(token_strings);
    // if (parse_result.first.empty()) {
    //     std::cerr << "Error: No se pudo parsear el script" << std::endl;
    //     return 1;
    // }

    // std::queue<std::shared_ptr<AttrProd>> productions;
    // std::vector<std::string> actions;
    // // Get the productions and actions from the parse result
    // for (const auto& id : parse_result.first) {
    //     productions.push(make_shared<AttrProd>(hulk_grammar.GetProduction(id)));
    // }
    // for (const auto& action : parse_result.second) {
    //     actions.push_back(action);
    // }

    // // Aplicar reverse_evaluate
    // std::cout << "Aplicando reverse_evaluate..." << std::endl;
    // auto root = build_ast(productions, actions, result, hulk_grammar);
    // if (!root) {
    //     std::cerr << "Error: No se pudo construir el AST" << std::endl;
    //     return 1;
    // }
    // std::cout << "AST construido exitosamente." << std::endl;
    // //enregion

	if (root) {
		std::cout << "Arbol de Sintaxis Abstracta:" << std::endl; 
		root->print();
    }
    //     std::cout << "\n=== Starting Two-Pass Semantic Analysis ===" << std::endl;
        
    //     // Create a global context for semantic analysis (with null parent for root context)
    //     Context* globalContext = new Context(nullptr);
       
    // //     // Define built-in functions and variables
    // //     globalContext->define("print", 1);  // print function takes 1 argument
    // //     globalContext->define("pi");         // pi constant (variable)
    //     // PASS 1: Type Collection
    //     std::cout << "\n--- Pass 1: Collecting Type Definitions ---" << std::endl;
    //     root->accept(typeCollectorVisitor, globalContext);
        
    //     if (typeCollectorVisitor->hasErrors()) {
    //         std::cerr << "Type collection failed with the following errors:" << std::endl;
    //         typeCollectorVisitor->printErrors();
    //     }
        
    //     std::cout << "Type collection completed successfully." << std::endl;
    //     std::cout << "Collected " << typeCollectorVisitor->getCollectedTypes().size() << " types" << std::endl;

    //     globalContext->localTypes = typeCollectorVisitor->getCollectedTypes();

        
    //     // PASS 2: Symbol Collection
    //     std::cout << "\n--- Pass 2: Collecting Symbols and Type Inference ---" << std::endl;
    //     symbolCollectorVisitor = new SymbolCollectorVisitor(globalContext);
    //     root->accept(symbolCollectorVisitor, globalContext);
        
    //     if (symbolCollectorVisitor->hasErrors()) {
    //         std::cerr << "Symbol collection failed with the following errors:" << std::endl;
    //         symbolCollectorVisitor->printErrors();
    //     }
        
    //     std::cout << "Symbol collection completed successfully." << std::endl;
        
    //     // Store collected symbols in the global context
    //     std::cout << "Storing collected symbols in context..." << std::endl;
    //     std::cout << "Symbol data successfully stored in context." << std::endl;
        
    //     // Display collected symbols
    //     globalContext->methodsByType = symbolCollectorVisitor->getMethodsByType();
    //     globalContext->attributesByType = symbolCollectorVisitor->getAttributesByType();
    //     globalContext->globalVariables = symbolCollectorVisitor->getGlobalVariables();
        
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
    //     root->accept(semanticVisitor, globalContext);
        
    //     if (semanticVisitor->hasErrors()) {
    //         std::cerr << "Final semantic analysis failed with the following errors:" << std::endl;
    //         semanticVisitor->printErrors();
    //         delete symbolCollectorVisitor;
    //         delete globalContext;
    //         delete root;
    //         fclose(yyin);
    //         return 1;
    //     }
        
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
