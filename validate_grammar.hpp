#pragma once
#include <iostream>
#include <vector>
#include "Grammar/grammar.h"

// Function to validate if two grammars are compatible for parsing
bool validate_grammar_compatibility(const Grammar& g1, const Grammar& g2, bool verbose = false) {
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

// Function to ensure we use the same grammar for both tokenization and parsing
Grammar& ensure_same_grammar(Grammar& grammar) {
    // This is a simple wrapper that can be expanded with validation logic
    return grammar;
}
