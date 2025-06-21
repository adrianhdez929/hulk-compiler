#include "Grammar/grammar.h"
#include "Parser/LR1Parser.h"
#include "Lexer/Token.h"
#include <iostream>
#include <vector>
#include <memory>
#include <string>

// Definición de una gramática de ejemplo simple para probar el parser LR1
void defineBooleanExpressionGrammar(Grammar& g) {
    // Definir símbolos terminales
    auto TRUE = g.SetTerminal("TRUE");
    auto FALSE = g.SetTerminal("FALSE");
    auto AND = g.SetTerminal("AND");
    auto OR = g.SetTerminal("OR");
    auto NOT = g.SetTerminal("NOT");
    auto LPAREN = g.SetTerminal("(");
    auto RPAREN = g.SetTerminal(")");
    auto END = g.SetTerminal("$"); // Marcador de fin de entrada
    
    // Definir símbolos no terminales
    auto expr = g.SetNonTerminal("expr", true); // Símbolo inicial
    auto term = g.SetNonTerminal("term");
    auto factor = g.SetNonTerminal("factor");
    auto literal = g.SetNonTerminal("literal");
    
    // Definir producciones para expresiones booleanas
    g.AddProduction(Production(expr, Sentence({expr, OR, term})));
    g.AddProduction(Production(expr, Sentence(term)));
    
    g.AddProduction(Production(term, Sentence({term, AND, factor})));
    g.AddProduction(Production(term, Sentence(factor)));
    
    g.AddProduction(Production(factor, Sentence({NOT, factor})));
    g.AddProduction(Production(factor, Sentence({LPAREN, expr, RPAREN})));
    g.AddProduction(Production(factor, Sentence(literal)));
    
    g.AddProduction(Production(literal, Sentence(TRUE)));
    g.AddProduction(Production(literal, Sentence(FALSE)));
}

// Función para probar el parser con una secuencia de tokens
void testParser(LR1Parser& parser, const std::vector<std::string>& tokens, const std::string& description) {
    std::cout << "=== Prueba: " << description << " ===" << std::endl;
    std::cout << "Tokens: ";
    for (const auto& token : tokens) {
        std::cout << token << " ";
    }
    std::cout << std::endl;
    
    try {
        auto result = parser.Parse(tokens);
        std::cout << "Análisis exitoso. Producciones utilizadas:" << std::endl;
        for (size_t i = 0; i < result.first.size(); ++i) {
            std::cout << "  " << i+1 << ". Producción " << result.first[i] 
                      << " (" << result.second[i] << ")" << std::endl;
        }
    } catch (const ParsingError& e) {
        std::cout << "Error de análisis en el estado " << e.getState() 
                  << " con el token '" << e.getToken() << "'" << std::endl;
        std::cout << "Tokens esperados: ";
        for (const auto& expected : e.getExpectedTokens()) {
            std::cout << expected << " ";
        }
        std::cout << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Excepción: " << e.what() << std::endl;
    }
    
    std::cout << std::endl;
}

int main() {
    // Crear una gramática
    Grammar grammar;
    defineBooleanExpressionGrammar(grammar);
    
    // Crear parser LR1
    LR1Parser parser(grammar, true);
    
    // Probar varias expresiones
    testParser(parser, {"TRUE", "$"}, "Literal TRUE");
    testParser(parser, {"FALSE", "$"}, "Literal FALSE");
    testParser(parser, {"NOT", "TRUE", "$"}, "NOT TRUE");
    testParser(parser, {"TRUE", "AND", "FALSE", "$"}, "TRUE AND FALSE");
    testParser(parser, {"TRUE", "OR", "FALSE", "$"}, "TRUE OR FALSE");
    testParser(parser, {"(", "TRUE", "AND", "FALSE", ")", "OR", "TRUE", "$"}, 
               "(TRUE AND FALSE) OR TRUE");
    testParser(parser, {"NOT", "(", "TRUE", "OR", "FALSE", ")", "$"}, 
               "NOT (TRUE OR FALSE)");
    testParser(parser, {"TRUE", "AND", "TRUE", "OR", "FALSE", "$"}, 
               "TRUE AND TRUE OR FALSE (precedencia)");
    
    // Caso de error para probar el manejo de errores
    testParser(parser, {"TRUE", "AND", "$"}, "Error: falta operando");
    
    return 0;
}
