#include <iostream>
#include "Grammar/grammar.h"
#include "Parser/SLR1Parser.h"
#include "hulkGrammar.hpp"

int main() {
    Grammar::ResetProductionCounter();
    Grammar hulk_grammar = getHulkGrammar();
    
    std::cout << "Creating SLR1 parser with the grammar..." << std::endl;
    SLR1Parser parser(hulk_grammar, true); // true enables verbose mode
    
    std::cout << "Parser created successfully!" << std::endl;
    
    // If we get here without infinite loops, our grammar is working correctly
    return 0;
}
