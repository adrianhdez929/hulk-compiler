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

    // Crear producciones
    E %= {
        E + plus + T,
        {
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
    };
    E %= {T.GetSentence(),
        {
            // Acción para E (heredado, sintetizado)
            [](auto h, auto s) { 
                auto left = std::any_cast<double>(s[0]);
                return left; 
            }
        }
    };

    string grammar_string = g.ToString();
    std::cout << grammar_string << std::endl;







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