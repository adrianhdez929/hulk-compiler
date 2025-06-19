/// \file execute.cpp
/// \brief Archivo para la ejecución del compilador Hulk

#include "execute.h"
#include "Lexer/grammar_parser.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include "Parser/reverse_evaluate.h"
#include "hulkGrammar.hpp"
#include "Parser/LR1Parser.h"
// #include "validate_grammar.hpp"

bool create_artifacts(Grammar& hulk_grammar, bool verbose) {
    if (verbose) {
        std::cout << "=== Creando artefactos del compilador Hulk ===" << std::endl;
    }

    // Reset production counter to ensure fresh start
    Grammar::ResetProductionCounter();

    // Verificar y crear directorio hulk
    if (!std::filesystem::exists("hulk")) {
        std::filesystem::create_directory("hulk");
    }

    // Crear gramática del lexer
    Grammar lexer_grammar = GrammarParser::Parse("Lexer/grammar.txt");
    
    // Crear parser
    // SLR1Parser parser(lexer_grammar);
    LR1Parser parser(lexer_grammar); // Usando LR1Parser en lugar de SLR1Parser
    
    // Serializar parser
    if (!parser.serialize_parser("parser.p", "hulk")) {
        std::cerr << "Error: No se pudo serializar el parser" << std::endl;
        return false;
    }
    // Crear tabla de tokens para el lexer
    std::vector<std::pair<std::string, std::string>> token_table = {
        {"string", "\"([\\x20-!#-\\x7e])*\""},
        {"number", "(0|[1-9][0-9]*)(.[0-9]+)?"}, // Regular expression for numbers
        {"bool", "true|false"},  // Regular expression for boolean values

        {"space", " +"}, // Regular expression for spaces
        // Regular expression for identifiers
        {"(", "\\("},            // Left parenthesis
        {")", "\\)"},            // Right parenthesis
		{"{", "\\{"},
		{"}", "\\}"},
		{";", "\\;"},
		{"+", "\\+"},
		{"-", "\\-"},
		{"*", "\\*"},
		{"/", "\\/"},
		{"^", "\\^"},
        {"%", "\\%"},
        {":=", ":="},          // Alternative assignment operator
        {":", "\\:"},
        {"|", "\\|"},
        {"&", "\\&"},
        {">=", ">="},
        {">", "\\>"},
        {"<=", "<="},
        {"<", "<"},
        {"==", "=="},
        {"!=", "!="},
        {"is", "is"},
        {"!", "!"},
        {",", "\\,"},           // Comma for lists
        {"=", "="},            // Assignment operator
        
        {"function", "function"}, // Function keyword
        {"=>", "=>"},          // Arrow for inline functions
        {"let", "let"},        // Let keyword
        {"in", "in"},          // In keyword for let expressions
        {"if", "if"},          // If keyword
        {"else", "else"},      // Else keyword
        {"elif", "elif"},      // Else-if keyword
        {"while", "while"},    // While keyword
        {"type", "type"},      // Type definition keyword
        {"inherits", "inherits"}, // Inheritance keyword
        {".", "\\."},          // Member access operator
        {"@", "@"},            // At operator
        {"@@", "@@"},          // Double at operator
        {"as", "as"},          // Type cast keyword
        {"for", "for"},        // For loop keyword 
        {"new", "new"},         // Object instantiation keyword
        {"type_id", "[A-Z][_a-zA-Z0-9]*"},
        {"var_id", "[a-z][_a-zA-Z0-9]*"},
        {"tab", "\t"},
        {"newline", "\n"} // Regular expression for newlines
    };
    // 	std::vector<std::pair<std::string, std::string>> token_table = {
    //     {"string", "\"([\\x20-!#-\\x7e])*\""},
    //     {"number", "(0|[1-9][0-9]*)(.[0-9]+)?"}, // Regular expression for numbers
    //     {"bool", "true|false"},  // Regular expression for boolean values
    //     {"type_id", "[A-Z][_a-zA-Z0-9]*"},
    //     {"var_id", "[_a-z][_a-zA-Z0-9]*"},
    //     {"space", " +"}, // Regular expression for spaces
    //     // Regular expression for identifiers
    //     {"(", "\\("},            // Left parenthesis
    //     {")", "\\)"},            // Right parenthesis
	// 	{"{", "\\{"},
	// 	{"}", "\\}"},
	// 	{";", "\\;"},
	// 	{"+", "\\+"},
	// 	{"-", "\\-"},
	// 	{"*", "\\*"},
	// 	{"/", "\\/"},
	// 	{"^", "\\^"},
    //     {"tab", "\t"},
    //     {"newline", "\n"}, // Regular expression for newlines
    //     {"EOF", "EOF"}      // End of file token
    // };
    
    // Crear lexer
    Lexer lexer(token_table, lexer_grammar, parser);
    
    // Serializar lexer
    if (!lexer.serialize_lexer("lexer.l", "hulk")) {
        std::cerr << "Error: No se pudo serializar el lexer" << std::endl;
        return false;
    }

    // Crear gramatica de Hulk
    Grammar::ResetProductionCounter();  // Reset before creating Hulk grammar
    // Grammar hulk_grammar = getHulkGrammar(); //getHulkGrammar();

    // Crear parser de Hulk
    LR1Parser hulk_parser(hulk_grammar); // true for verbose mode
    // Serializar parser de Hulk
    if (!hulk_parser.serialize_parser("hulk_parser.p", "hulk")) {
        std::cerr << "Error: No se pudo serializar el parser de Hulk" << std::endl;
        return false;
    }
    
    if (verbose) {
        std::cout << "Artefactos creados exitosamente en la carpeta 'hulk'" << std::endl;
    }
    return true;
}

std::pair<Lexer*, LR1Parser*> load_compiler_artifacts(std::string& error_message, Grammar& parser_grammar, bool verbose) {
    // Verificar que existen los artefactos
    if (!std::filesystem::exists("hulk")) {
        std::filesystem::create_directory("hulk");
    }
    
    // Verificar si necesitamos crear los artefactos
    static bool force_recreate = false;  // Esta variable se puede cambiar para forzar la recreación
    if (force_recreate || !std::filesystem::exists("hulk/lexer.l") || !std::filesystem::exists("hulk/parser.p")) {
        if (!create_artifacts(parser_grammar, verbose)) {
            error_message = "No se pudieron crear los artefactos necesarios para el compilador";
            return {nullptr, nullptr};
        }
    }

    // Cargar Lexer y Parser
    Lexer* lexer = Lexer::deserialize_lexer("lexer.l", "hulk");
    if (!lexer) {
        error_message = "No se pudo cargar el lexer";
        return {nullptr, nullptr};
    }

    // Para depuración, verifiquemos los símbolos de la gramática 
    // if (verbose) {
    //     std::cout << "Símbolos en la gramática cargada:" << std::endl;
    //     for (const auto& terminal : parser_grammar.Terminals()) {
    //         std::cout << " - Terminal: " << terminal->Name() << std::endl;
    //     }
    //     for (const auto& nonterminal : parser_grammar.NonTerminals()) {
    //         std::cout << " - No Terminal: " << nonterminal->Name() << std::endl;
    //     }
    //     for (const auto& production : parser_grammar.Productions()) {
    //         std::cout << " - Producción: " << production.ToString() << std::endl;
    //     }
    // }

    LR1Parser* parser = LR1Parser::deserialize_parser("hulk_parser.p", "hulk", parser_grammar);
    if (!parser) {
        delete lexer; // Liberar memoria del lexer cargado
        error_message = "No se pudo cargar el parser. Es posible que la gramática utilizada para crear el parser serializado sea diferente de la gramática actual. Intenta recrear los artefactos.";
        return {nullptr, nullptr};
    }
    
    // Verificar que el parser tenga producciones válidas
    if (verbose) {
        std::cout << "Validando producciones del parser después de deserialización..." << std::endl;
        
        // Imprimir algunas producciones para verificar que estén correctas
        const auto& productions = parser_grammar.Productions();
        if (!productions.empty()) {
            std::cout << "Primera producción: " << productions[0].ToString() << " (ID: " << productions[0].get_id() << ")" << std::endl;
            if (productions.size() > 1) {
                std::cout << "Segunda producción: " << productions[1].ToString() << " (ID: " << productions[1].get_id() << ")" << std::endl;
            }
        }
    }

    return {lexer, parser};
}

// Sobrecarga eliminada: usamos parámetros por defecto en lugar de sobrecargas

std::vector<Token> tokenize_source(const std::string& source_code, Lexer* lexer, 
                                  Grammar& hulk_grammar, std::string& error_message) {
    try {
        // Tokenizar el código fuente con información de posición
        auto tokens = lexer->tokenize_with_positions(source_code, hulk_grammar);
        
        if (tokens.empty()) {
            error_message = "No se generaron tokens del código fuente";
            return {};
        }
        
        return tokens;
    } catch (const std::runtime_error& e) {
        error_message = std::string("Error de tokenización: ") + e.what();
        return {};
    }
}

ASTNode* build_ast_from_tokens(const std::vector<Token>& tokens, LR1Parser* parser, 
                              Grammar& hulk_grammar, std::string& error_message, bool verbose) {
    try {
        if (verbose) {
            std::cout << "Iniciando análisis sintáctico con " << tokens.size() << " tokens..." << std::endl;
            
            // print tokens
            std::cout << "Tokens generados:" << std::endl;
            for (const auto& token : tokens) {
                std::cout << "  " << token.ToString() << std::endl;
            }

            // // Check if productions are still valid before parsing
            // std::cout << "Verificando integridad de producciones..." << std::endl;
            // int count = 0;
            // for (const auto& prod : hulk_grammar.Productions()) {
            //     std::cout << "Producción " << prod.get_id() << ": " << prod.ToString() << std::endl;
            //     count++;
            // }
            // std::cout << "Total de producciones verificadas: " << count << std::endl;
        }
        
        // Realizar análisis sintáctico
        auto parse_result = parser->Parse(tokens);
        
        if (verbose) {
            std::cout << "Análisis sintáctico exitoso. Obtenidos " << parse_result.first.size() 
                     << " IDs de producción y " << parse_result.second.size() << " acciones." << std::endl;
        }
        
        // Preparar producciones y acciones
        std::queue<std::shared_ptr<AttrProd>> productions;
        std::vector<std::string> actions;
        
        // Obtener producciones y acciones del resultado del parser
        if (verbose) {
            std::cout << "Obteniendo producciones a partir de IDs..." << std::endl;
        }
        
        int count = 0;
        for (const auto& id : parse_result.first) {
            try {
                if (verbose) {
                    std::cout << "  Buscando producción con ID: " << id << std::endl;
                }
                const AttrProd& prod = hulk_grammar.GetProduction(id);
                if (verbose) {
                    std::cout << "  Producción encontrada: " << prod.ToString() << std::endl;
                }
                productions.push(make_shared<AttrProd>(prod));
                count++;
            } catch (const std::runtime_error& e) {
                if (verbose) {
                    std::cerr << "  Error al obtener producción con ID " << id << ": " << e.what() << std::endl;
                }
                throw;
            }
        }
        
        // if (verbose) {
        //     std::cout << "Se obtuvieron " << count << " producciones correctamente." << std::endl;
        //     std::cout << "Procesando " << parse_result.second.size() << " acciones..." << std::endl;
        // }
        
        for (const auto& action : parse_result.second) {
            actions.push_back(action);
        }
        
        // Construir el AST
        auto ast = build_ast(productions, actions, tokens, hulk_grammar);
        if (!ast) {
            error_message = "No se pudo construir el AST";
            return nullptr;
        }
        
        return ast;
    } catch (const LR1ParsingError& e) {
        error_message = std::string("Error sintáctico: ") + e.what();
        return nullptr;
    } catch (const std::exception& e) {
        error_message = std::string("Error durante el análisis: ") + e.what();
        return nullptr;
    }
}

ASTNode* compile_hulk(const std::string& source_code, std::string& error_message, Grammar& hulk_grammar, bool verbose) {
    // Crear una única instancia de gramática para todo el proceso
    // Grammar hulk_grammar = getHulkGrammar();

    // Cargar artefactos del compilador
    std::string artifact_error;
    auto [lexer, parser] = load_compiler_artifacts(artifact_error, hulk_grammar, verbose);
    if (!lexer || !parser) {
        error_message = artifact_error;
        return nullptr;
    }
    
    // Tokenizar el código fuente
    std::string tokenize_error;
    auto tokens = tokenize_source(source_code, lexer, hulk_grammar, tokenize_error);
    if (tokens.empty()) {
        error_message = tokenize_error;
        delete lexer;
        delete parser;
        return nullptr;
    }
    
    // Construir el AST
    std::string ast_error;
    ASTNode* ast = build_ast_from_tokens(tokens, parser, hulk_grammar, ast_error, verbose);
    if (!ast) {
        error_message = ast_error;
        delete lexer;
        delete parser;
        return nullptr;
    }
    
    // Liberar memoria de lexer y parser una vez que hemos terminado con ellos
    delete lexer;
    delete parser;
    
    return ast;
}

ASTNode* compile_hulk_file(const std::string& file_path, std::string& error_message, Grammar& hulk_grammar, bool verbose) {
    // Leer contenido del archivo
    std::string source_code = read_source_file(file_path, error_message);
    if (source_code.empty()) {
        return nullptr; // Error ya está en error_message
    }
    
    if (verbose) {
        std::cout << "Archivo '" << file_path << "' cargado exitosamente." << std::endl;
    }
    
    // Compilar el código fuente
    return compile_hulk(source_code, error_message, hulk_grammar, verbose);
}

// Sobrecarga eliminada: usamos parámetros por defecto en lugar de sobrecargas

std::string read_source_file(const std::string& file_path, std::string& error_message) {
    // Intentar abrir el archivo
    std::ifstream source_file(file_path);
    if (!source_file.is_open()) {
        error_message = "No se pudo abrir el archivo: " + file_path;
        return "";
    }

    try {
        // Leer todo el contenido del archivo
        std::string source_content((std::istreambuf_iterator<char>(source_file)), 
                                   std::istreambuf_iterator<char>());
        source_file.close();
        return source_content;
    }
    catch(const std::exception& e) {
        error_message = std::string("Error al leer el archivo: ") + e.what();
        if (source_file.is_open()) {
            source_file.close();
        }
        return "";
    }
}

bool validate_grammar_compatibility(const Grammar& g1, const Grammar& g2, bool verbose) {
    bool compatible = true;
    
    // Check number of productions
    if (g1.Productions().size() != g2.Productions().size()) {
        if (verbose) {
            std::cout << "Warning: Grammars have different number of productions: " 
                      << g1.Productions().size() << " vs " << g2.Productions().size() << std::endl;
        }
        compatible = false;
    }
    
    // Check productions by ID
    for (const auto& prod1 : g1.Productions()) {
        try {
            const AttrProd& prod2 = g2.GetProduction(prod1.get_id());
            // Check if productions are equivalent
            if (prod1.ToString() != prod2.ToString()) {
                if (verbose) {
                    std::cout << "Warning: Production with ID " << prod1.get_id() 
                              << " differs between grammars:" << std::endl;
                    std::cout << "  G1: " << prod1.ToString() << std::endl;
                    std::cout << "  G2: " << prod2.ToString() << std::endl;
                }
                compatible = false;
            }
        } catch (const std::runtime_error& e) {
            if (verbose) {
                std::cout << "Warning: Production with ID " << prod1.get_id() 
                          << " not found in second grammar: " << e.what() << std::endl;
            }
            compatible = false;
        }
    }
    
    return compatible;
}



// int main(int argc, char* argv[]) {
//     std::cout << "=== Ejecutando el compilador Hulk ===" << std::endl;
    
//     // Verificar si se solicita recrear los artefactos
//     bool recreate_artifacts = false;
//     std::string file_path = "script.hulk";
    
//     // for (int i = 1; i < argc; i++) {
//     //     std::string arg = argv[i];
//     //     if (arg == "--recreate" || arg == "-r") {
//     //         recreate_artifacts = true;
//     //     } else {
//     //         file_path = arg;
//     //     }
//     // }

//     // Verificar si estan los artefactos del compilador
//     if (!std::filesystem::exists("hulk/lexer.l") || !std::filesystem::exists("hulk/parser.p")) {
//         recreate_artifacts = true; // Forzar recreación si no existen los artefactos
//     }
    
//     if (recreate_artifacts) {
//         std::cout << "Recreando artefactos del compilador..." << std::endl;
//         std::string error_msg;
//         Grammar hulk_grammar = getHulkGrammar();
//         bool success = create_artifacts(hulk_grammar, true);
//         if (!success) {
//             std::cerr << "Error al recrear los artefactos" << std::endl;
//             return 1;
//         }
//         std::cout << "Artefactos recreados exitosamente" << std::endl;
//     }
    
//     // Cargar archivo de código fuente
//     std::string error_message;
//     std::string script_content = read_source_file(file_path, error_message);
//     if (script_content.empty()) {
//         std::cerr << "Error: " << error_message << std::endl;
//         return 1;
//     }
//     std::cout << "Contenido de '" << file_path << "' cargado exitosamente." << std::endl;
    
//     // Compilar el código utilizando nuestra función principal con modo verbose
//     // Nota: Utilizamos una única gramática para todo el proceso de compilación
//     Grammar hulk_grammar = getHulkGrammar();
//     ASTNode* ast = compile_hulk(script_content, error_message, hulk_grammar, false);
    
//     if (!ast) {
//         std::cerr << "Error al compilar: " << error_message << std::endl;
//         return 1;
//     }
    
//     // El AST se ha construido correctamente, podemos imprimirlo o procesarlo
//     std::cout << "\nImprimiendo el AST construido:" << std::endl;
//     ast->print();
    
//     // Aquí podrías agregar más procesamiento del AST, como:
//     // - Análisis semántico
//     // - Optimización
//     // - Generación de código
    
//     // Liberar la memoria del AST cuando hayamos terminado con él
//     // delete ast;  // Descomenta cuando termines de usarlo
    
//     return 0;
// }
