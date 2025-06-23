#include "Grammar/grammar.h"
#include "Parser/LR1Parser.h"
#include "Lexer/grammar_parser.h"
#include <iostream>

/**
 * Programa de prueba específico para probar el parser LR1 con la gramática del lexer
 */
int main(int argc, char* argv[]) {
    std::cout << "Probando el parser LR1 con la gramática del lexer..." << std::endl;
    
    // Cargar la gramática del lexer
    std::cout << "Leyendo gramática del lexer desde 'Lexer/grammar.txt'..." << std::endl;
    Grammar lexer_grammar = GrammarParser::Parse("Lexer/grammar.txt");
    
    std::cout << "Gramática cargada exitosamente." << std::endl;
    std::cout << "  Terminales: " << lexer_grammar.Terminals().size() << std::endl;
    std::cout << "  No terminales: " << lexer_grammar.NonTerminals().size() << std::endl;
    std::cout << "  Producciones: " << lexer_grammar.Productions().size() << std::endl;

    // Crear parser LR1 con modo verbose
    std::cout << "Creando parser LR1..." << std::endl;
    bool verbose = true;
    
    try {
        LR1Parser parser(lexer_grammar, verbose);
        std::cout << "Parser LR1 creado exitosamente!" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error al crear el parser LR1: " << e.what() << std::endl;
        return 1;
    }
}
