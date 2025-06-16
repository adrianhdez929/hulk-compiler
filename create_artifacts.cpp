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
        {"var_id", "[_a-z][_a-zA-Z0-9]*"},
    };
    
    // Crear tabla de tokens para el lexer
    // std::vector<std::pair<std::string, std::string>> token_table = {
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
    //     {":", "\\:"},
    //     {"|", "\\|"},
    //     {"&", "\\&"},
    //     {">=", ">="},
    //     {">", "\\>"},
    //     {"<=", "<="},
    //     {"<", "<"},
    //     {"==", "=="},
    //     {"!=", "!="},
    //     {"is", "is"},
    //     {"!", "!"}
    // };
    // std::vector<std::pair<std::string, std::string>> token_table = {
    //     {"string", "\"([\\x20-!#-\\x7e])*\""},
    //     {"number", "(0|[1-9][0-9]*)(.[0-9]+)?"}, // Regular expression for numbers
    //     {"bool", "true|false"},  // Regular expression for boolean values
    //     {"type_id", "[A-Z][_a-zA-Z0-9]*"},
    //     {"var_id", "[_a-z][_a-zA-Z0-9]*"},
    //     {"space", " +"},
    //     {"newline", "\n"}, // Regular expression for newlines
    //     // {"whitespace", "[\t\r\f]+"}, // Regular expression for whitespace
    //     //{"comment", "//.*"}, // Regular expression for single-line comments
    //     //{"block_comment", "/\\*.*?\\*/"}, // Regular expression for block comments
    //     {"EOF", "EOF"},      // End of file token
    //     //{"identifier", "[_a-zA-Z][_a-zA-Z0-9]*"}, // Regular expression for identifiers
    //     {"(", "\\("},            // Left parenthesis
    //     {")", "\\)"},
    //     {"{", "\\{"},            // Left curly brace
    //     {"}", "\\}"},            // Right curly brace
    //     {";", "\\;"},            // Semicolon
    //     {"+", "\\+"},            // Plus sign
    //     {"-", "\\-"},            // Minus sign
    //     {"*", "\\*"},            // Asterisk
    //     {"/", "\\/"},            // Slash
    //     {"^", "\\^"}             // Caret for exponentiation
    // };
    // std::vector<std::pair<std::string, std::string>> token_table = {
    //     {"string", "\"([\\x20-!#-\\x7e])*\""},
    //     {"number", "(0|[1-9][0-9]*)(.[0-9]+)?"},
    //     {"bool", "true|false"},
    //     {"type_id", "[A-Z][_a-zA-Z0-9]*"},
    //     {"id", "[_a-z][_a-zA-Z0-9]*"},
    //     {"space", " +"},
    //     {"let", "let"},
    //     {"iN", "in"},
    //     {"iF", "if"},
    //     {"elSe", "else"},
    //     {"eliF", "elif"},
    //     {"whilE", "while"},
    //     {"foR", "for"},
    //     {"new", "new"},
    //     {"aS", "as"},
    //     {"comma", ","},
    //     {"dot", "\\."},
    //     {"semi_colon", ";"},
    //     {"colon", ":"},
    //     {"rarrow", "=>"},
    //     {"dollar", "\\$"},
    //     {"at", "@"},
    //     {"atat", "@@"},
    //     {"normal_arrow", "->"},
    //     {"plus", "\\+"},
    //     {"minus", "\\-"},
    //     {"star", "\\*"},
    //     {"starstar", "\\*\\*"},
    //     {"divide", "\\/"},
    //     {"pow", "\\^"},
    //     {"mod", "%"},
    //     {"opar", "\\("},
    //     {"cpar", "\\)"},
    //     {"obrack", "\\["},
    //     {"cbrack", "\\]"},
    //     {"obrace", "\\{"},
    //     {"cbrace", "\\}"},
    //     {"eq_eq", "=="},
    //     {"equal", "="},
    //     {"no_eq", "!="},
    //     {"gt", ">"},
    //     {"gt_eq", ">="},
    //     {"lt", "<"},
    //     {"lt_eq", "<="},
    //     {"coloneq", ":="},
    //     {"anD", "&"},
    //     {"oR", "\\|"},
    //     {"or_or", "\\|\\|"},
    //     {"exclam", "!"},
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

// Main function for standalone artifact creation
int main() {
    create_artifacts();
    return 0;
}