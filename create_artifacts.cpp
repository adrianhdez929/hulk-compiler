/// \file create_artifacts.cpp
/// \brief Archivo para la creación de artefactos del compilador Hulk

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include "Lexer/Lexer.h"
#include "Parser/SLR1Parser.h"
#include "Grammar/grammar.h"
#include "Lexer/grammar_parser.h"
#include "hulkGrammar.hpp"

/// \brief Crea los artefactos necesarios para el compilador Hulk
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
        {"type_id", "[A-Z][_a-zA-Z0-9]*"},
        {"var_id", "[_a-z][_a-zA-Z0-9]*"},
        {"space", " +"},
        {"newline", "\n"}, // Regular expression for newlines
        //{"whitespace", "[\\t\\r\\f]+"}, // Regular expression for whitespace
        //{"comment", "//.*"}, // Regular expression for single-line comments
        //{"block_comment", "/\\*.*?\\*/"}, // Regular expression for block comments
        {"EOF", "EOF"},      // End of file token
        //{"identifier", "[_a-zA-Z][_a-zA-Z0-9]*"}, // Regular expression for identifiers
        {"(", "\\("},            // Left parenthesis
        {")", "\\)"},
        {"{", "\\{"},            // Left curly brace
        {"}", "\\}"},            // Right curly brace
        {";", "\\;"},            // Semicolon
        {"+", "\\+"},            // Plus sign
        {"-", "\\-"},            // Minus sign
        {"*", "\\*"},            // Asterisk
        {"/", "\\/"},            // Slash
        {"^", "\\^"}             // Caret for exponentiation
    };
    
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
    Grammar hulk_grammar = getHulkGrammar();

    // Crear parser de Hulk
    SLR1Parser hulk_parser(hulk_grammar);
    // Serializar parser de Hulk
    if (!hulk_parser.serialize_parser("hulk_parser.p")) {
        std::cerr << "Error: No se pudo serializar el parser de Hulk" << std::endl;
        return;
    }
    
    std::cout << "Artefactos creados exitosamente en la carpeta 'hulk'" << std::endl;
}
// int main() {
//     create_artifacts();
//     return 0;
// }