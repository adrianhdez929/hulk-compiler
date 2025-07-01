#include "hulkGrammar.hpp" 
#include "Parser/SLR1Parser.h"
#include <iostream>
#include <sstream>

int main() {
    try {
        std::cout << "Creating Hulk grammar..." << std::endl;
        Grammar g = getHulkGrammar();
        std::cout << "Grammar created with " << g.productions.size() << " productions." << std::endl;
        
        // Print productions related to type declarations and attributes
        std::cout << "\nType-related productions:" << std::endl;
        for (const auto& p : g.productions) {
            std::string lhs = p.Left().value;
            if (lhs == "type_node_decl" || lhs == "type_body_elements" || lhs == "attribute") {
                std::cout << "  " << lhs << " -> ";
                for (const auto& s : p.Right().symbols) {
                    std::cout << s.value << " ";
                }
                std::cout << std::endl;
            }
        }
        
        std::cout << "\nAttempting to build SLR(1) parser..." << std::endl;
        SLR1Parser parser(g);
        std::cout << "SLR(1) parser built successfully!" << std::endl;
        
        // Create a simple test token stream for "type Point { x = 3; }"
        std::vector<Token> tokens;
        tokens.push_back(Token("type", "type", g, 1));
        tokens.push_back(Token("var_id", "Point", g, 1));
        tokens.push_back(Token("{", "{", g, 1));
        tokens.push_back(Token("var_id", "x", g, 2));
        tokens.push_back(Token("=", "=", g, 2));
        tokens.push_back(Token("number", "3", g, 2));
        tokens.push_back(Token(";", ";", g, 2));
        tokens.push_back(Token("}", "}", g, 3));
        
        std::cout << "\nAttempting to parse:" << std::endl;
        for (const auto& t : tokens) {
            std::cout << t.Name() << "(" << t.Lexeme() << ") ";
        }
        std::cout << std::endl;
        
        parser.Parse(tokens);
        std::cout << "\nParsing successful!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
