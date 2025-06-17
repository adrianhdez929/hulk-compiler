/// \file execute.cpp
/// \brief Archivo para la ejecución del compilador Hulk

// #include "create_artifacts.cpp"
#include "Lexer/Lexer.h"
#include "Parser/SLR1Parser.h"
#include "Grammar/grammar.h"
#include "Lexer/grammar_parser.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include "Parser/reverse_evaluate.h"
#include "hulkGrammar.hpp"

void create_artifacts() {
    std::cout << "=== Creando artefactos del compilador Hulk ===" << std::endl;

    // Reset production counter to ensure fresh start
    Grammar::ResetProductionCounter();

    // Verificar y crear directorio hulk
    if (!std::filesystem::exists("hulk")) {
        std::filesystem::create_directory("hulk");
    }

    // Crear gramática del lexer
    Grammar lexer_grammar = GrammarParser::Parse("Lexer/grammar.txt");
    
    // Crear parser
    SLR1Parser parser(lexer_grammar);
    
    // Serializar parser
    if (!parser.serialize_parser("parser.p")) {
        std::cerr << "Error: No se pudo serializar el parser" << std::endl;
        return;
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
    if (!lexer.serialize_lexer("lexer.l")) {
        std::cerr << "Error: No se pudo serializar el lexer" << std::endl;
        return;
    }

    // Crear gramatica de Hulk
    // POR IMPLEMENTAR: Aquí deberías definir la gramática específica de Hulk
    Grammar::ResetProductionCounter();  // Reset before creating Hulk grammar
    Grammar hulk_grammar = getHulkGrammar(); //getHulkGrammar();

    // Crear parser de Hulk
    SLR1Parser hulk_parser(hulk_grammar); // true for verbose mode
    // Serializar parser de Hulk
    if (!hulk_parser.serialize_parser("hulk_parser.p")) {
        std::cerr << "Error: No se pudo serializar el parser de Hulk" << std::endl;
        return;
    }
    
    std::cout << "Artefactos creados exitosamente en la carpeta 'hulk'" << std::endl;
}

int run_compiler() {
    std::cout << "=== Ejecutando el compilador Hulk ===" << std::endl;
    
    // Verificar y crear directorio hulk
    if (!std::filesystem::exists("hulk")) {
        std::filesystem::create_directory("hulk");
    }
    // Crear artefactos del compilador si no estan creados
    if (!std::filesystem::exists("hulk/lexer.l") || !std::filesystem::exists("hulk/parser.p")) {
        create_artifacts();
    }

    // Cargar Lexer y Parser
    Lexer* lexer = Lexer::deserialize_lexer("lexer.l");
    if (!lexer) {
        std::cerr << "Error: No se pudo cargar el lexer" << std::endl;
        return 1;
    }

    // Cragar gramatica de hulk
    Grammar hulk_grammar = getHulkGrammar();

    // Cargar Parser
    SLR1Parser* parser = SLR1Parser::deserialize_parser("hulk_parser.p", hulk_grammar);
    if (!parser) {
        std::cerr << "Error: No se pudo cargar el parser" << std::endl;
        return 1;
    }
    std::cout << "Artefactos cargados exitosamente." << std::endl;

    // Cargar script.hulk
    std::ifstream script_file("script.hulk");
    if (!script_file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo script.hulk" << std::endl;
        return 1;
    }
    std::string script_content((std::istreambuf_iterator<char>(script_file)), std::istreambuf_iterator<char>());
    script_file.close();
    std::cout << "Contenido del script.hulk cargado exitosamente." << std::endl;
    
    // Tokenizar el script con información de posición
    try {
        // Usar el nuevo método tokenize_with_positions que incluye información de línea y columna
        // y filtra automáticamente espacios, tabuladores y saltos de línea
        auto tokens_with_positions = lexer->tokenize_with_positions(script_content, hulk_grammar);
        std::cout << "Tokens generados con información de posición:" << std::endl;
        for (const auto& token : tokens_with_positions) {
            std::cout << token.ToString() << std::endl;
        }
        
        if (tokens_with_positions.empty()) {
            std::cerr << "Error: No se generaron tokens del script" << std::endl;
            return 1;
        }
        
        std::cout << "Tokens significativos generados exitosamente." << std::endl;
        
        // Usar los tokens con posición para el análisis sintáctico
        try {
            auto parse_result = parser->Parse(tokens_with_positions);
            std::cout << "Análisis sintáctico exitoso." << std::endl;
            
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

            std::queue<std::shared_ptr<AttrProd>> productions;
            std::vector<std::string> actions;
            // Get the productions and actions from the parse result
            for (const auto& id : parse_result.first) {
                productions.push(make_shared<AttrProd>(hulk_grammar.GetProduction(id)));
            }
            for (const auto& action : parse_result.second) {
                actions.push_back(action);
            }

            std::vector<std::pair<std::string, std::string>> result;
            for (const auto& token : tokens_with_positions) {
                result.push_back(std::make_pair(token.Lexeme(), token.Name()));
            }

            // Aplicar reverse_evaluate
            std::cout << "Aplicando reverse_evaluate..." << std::endl;
            auto ast = build_ast(productions, actions, tokens_with_positions, hulk_grammar);
            if (!ast) {
                std::cerr << "Error: No se pudo construir el AST" << std::endl;
                return 1;
            }
            std::cout << "AST construido exitosamente." << std::endl;
            // Imprimir el AST
            std::cout << "Imprimiendo el AST:" << std::endl;
            ast->print();
            std::cout << "Ejecución del compilador Hulk finalizada exitosamente." << std::endl;
            
        } catch (const ParsingError& e) {
            std::cerr << "Error sintáctico: " << e.what() << std::endl;
            return 1;
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Error de tokenización: " << e.what() << std::endl;
        return 1;
    }
    

    return 0;
}
