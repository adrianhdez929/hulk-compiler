#include <iostream>
#include "Gramatica2/grammar.h"

using namespace std;

int main() {
    Grammar g;
    auto E = g.SetNonTerminal("E", true);
    auto T = g.SetNonTerminal("T");
    auto plus = g.SetTerminal("+");
    auto num = g.SetTerminal("num");
    auto pipe = g.SetTerminal("|");
    auto symbol = g.SetTerminal("symbol");

    // Producción con atributos
    E %= std::make_pair(
        E + plus + T,
        std::vector<AttributeProduction::SemanticAction>{
            // Acción para E (heredado, sintetizado)
            [](auto h, auto s) { 
                auto left = std::any_cast<double>(s[0]);
                auto right = std::any_cast<double>(s[2]);
                return left + right; 
            },
            // Acción para '+'
            [](auto h, auto s) { return 0.0; }, 
            // Acción para T
            [](auto h, auto s) { return s[3]; },
            // Acción para LHS (E)
            [](auto h, auto s) { return s[0]; }
        }
    );

    // Producción sin atributos
    // E %= T + plus + T;

    string grammar_string = g.ToString();
    std::cout << grammar_string << std::endl;














    //ATTEMPT 1
    // Grammar g;
    
    // // Crear símbolos
    // Symbol* E = g.createSymbol(Symbol::NON_TERMINAL, "E");
    // Symbol* T = g.createSymbol(Symbol::NON_TERMINAL, "T");
    // Symbol* pipe = g.createSymbol(Symbol::TERMINAL, "|");
    // Symbol* symb = g.createSymbol(Symbol::TERMINAL, "symbol");
    
    // // Añadir producciones con acciones semánticas
    // g.addProduction(
    //     E,
    //     {E, pipe, T},
    //     [](const std::vector<std::unique_ptr<Node>>& children) {
    //         return std::make_unique<UnionNode>(
    //             std::move(children[0]), 
    //             std::move(children[2])
    //         );
    //     }
    // );
    
    // g.addProduction(
    //     T,
    //     {symb},
    //     [](const std::vector<std::unique_ptr<Node>>& children) {
    //         return std::make_unique<SymbolNode>(
    //             std::move(children[0])
    //         );
    //     }
    // );
    // std::any a;
    // std::cout << a.type().name() << std::endl;
    
    // Crear un nodo raiz









    //ATTEMPT 2
    
    // Grammar g;
    // auto E = g.SetNonTerminal("E", true);
    // auto T = g.SetNonTerminal("T");
    // auto plus = g.SetTerminal("+");
    // auto num = g.SetTerminal("num");

    // // Producción con atributos
    // E %= std::make_pair(
    //     E + plus + T,
    //     std::vector<AttributeProduction::SemanticAction>{
    //         // Acción para E (heredado, sintetizado)
    //         [](auto h, auto s) { 
    //             auto left = std::any_cast<double>(s[0]);
    //             auto right = std::any_cast<double>(s[2]);
    //             return left + right; 
    //         },
    //         // Acción para '+'
    //         [](auto h, auto s) { return 0.0; }, 
    //         // Acción para T
    //         [](auto h, auto s) { return s[3]; },
    //         // Acción para LHS (E)
    //         [](auto h, auto s) { return s[0]; }
    //     }
    // );
    return 0;
}