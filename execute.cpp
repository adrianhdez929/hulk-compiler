/// \file execute.cpp
/// \brief Archivo para la ejecución del compilador Hulk

#include "create_artifacts.cpp"
#include "Lexer/Lexer.h"
#include "Parser/SLR1Parser.h"
#include "Grammar/grammar.h"
#include "Lexer/grammar_parser.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include "Parser/reverse_evaluate.h"


int main() {
    std::cout << "=== Ejecutando el compilador Hulk ===" << std::endl;
    
    // // Verificar y crear directorio hulk
    // if (!std::filesystem::exists("hulk")) {
    //     std::filesystem::create_directory("hulk");
    // }
    // // Crear artefactos del compilador si no estan creados
    // if (!std::filesystem::exists("hulk/lexer.l") || !std::filesystem::exists("hulk/parser.slr")) {
    //     create_artifacts();
    // }

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
    
    // Tokenizar el script
    auto result = lexer->tokenize(script_content);
    std::cout << "Tokens generados:" << std::endl;
    for (const auto& token : result) {
        std::cout << "Tipo: " << token.first << ", Valor: " << token.second << std::endl;
    }
    if (result.empty()) {
        std::cerr << "Error: No se generaron tokens del script" << std::endl;
        return 1;
    }
    // Retirar los espacios, saltos de linea y tabulaciones
    result.erase(std::remove_if(result.begin(), result.end(),
                                 [](const std::pair<std::string, std::string>& token) {
                                     return token.second == "space" || token.second == "newline" || token.second == "tab";
                                 }), result.end());
    if (result.empty()) {
        std::cerr << "Error: No se generaron tokens significativos del script" << std::endl;
        return 1;
    }
    std::cout << "Tokens significativos generados exitosamente." << std::endl;
    // Imprimir los tokens generados
    std::cout << "Tokens generados:" << std::endl;
    for (const auto& token : result) {
        std::cout << "Tipo: " << token.first << ", Valor: " << token.second << std::endl;
    }

    std::vector<std::string> token_strings;
    for (const auto& token : result) {
        token_strings.push_back(token.second);
    }

    // Parsear los tokens
    auto parse_result = parser->Parse(token_strings);
    if (parse_result.first.empty()) {
        std::cerr << "Error: No se pudo parsear el script" << std::endl;
        return 1;
    }

    std::queue<std::shared_ptr<AttrProd>> productions;
    std::vector<std::string> actions;
    // Get the productions and actions from the parse result
    for (const auto& id : parse_result.first) {
        productions.push(make_shared<AttrProd>(hulk_grammar.GetProduction(id)));
    }
    for (const auto& action : parse_result.second) {
        actions.push_back(action);
    }

    // Aplicar reverse_evaluate
    std::cout << "Aplicando reverse_evaluate..." << std::endl;
    auto ast = build_ast(productions, actions, result, hulk_grammar);
    if (!ast) {
        std::cerr << "Error: No se pudo construir el AST" << std::endl;
        return 1;
    }
    std::cout << "AST construido exitosamente." << std::endl;
    // Imprimir el AST
    std::cout << "Imprimiendo el AST:" << std::endl;
    ast->print();
    std::cout << "Ejecución del compilador Hulk finalizada exitosamente." << std::endl;

    // Cargar gramática del lexer
    // Grammar lexer_grammar = GrammarParser::Parse("Lexer/grammar.txt");
    return 0;
}
