#include "symbol.h"
#include "sentence.h" // Ensure Sentence is fully declared

Symbol::Symbol(const std::string& name, Grammar& grammar) 
    : name(name), grammar(grammar) {}

// Symbol::~Symbol() = default;
const std::string& Symbol::Name() const { return name; }

bool Symbol::IsTerminal() const { return false; }
bool Symbol::IsNonTerminal() const { return false; }
bool Symbol::IsEpsilon() const { return false; }
bool Symbol::IsEndOfFile() const { return false; }

bool Symbol::operator==(const Symbol& other) const {
    return this == &other;
}

Terminal::Terminal(const std::string& name, Grammar& grammar) 
    : Symbol(name, grammar) {}

bool Terminal::IsTerminal() const { return true; }

Sentence Symbol::operator+(const Symbol& other) const {
    return Sentence(this, &other);
}

Sentence Symbol::operator+(const Sentence& other) const {
    std::vector<const Symbol*> combinedSymbols = {this};
    combinedSymbols.insert(combinedSymbols.end(), other.Symbols().begin(), other.Symbols().end());
    return Sentence(combinedSymbols);
}

Sentence Symbol::GetSentence() const {
    return Sentence(this);
}

// Sentence Symbol::operator+(const std::vector<const Symbol*>& symbols) const {
//     // symbol + sentence(symbol1, symbol2,...,symboln) = sentence(symbol, symbol1,...,symboln)
//     std::vector<const Symbol*> combinedSymbols = {this};
//     combinedSymbols.insert(combinedSymbols.end(), symbols.begin(), symbols.end());
//     return Sentence(combinedSymbols);
// }

// Sentence Symbol::operator+(const Symbol& other) const {
//     return Sentence(this, &other);
// }

// Sentence Symbol::operator+(const Sentence& other) const {
//     std::vector<const Symbol*> combinedSymbols = {this};
//     combinedSymbols.insert(combinedSymbols.end(), other.Symbols().begin(), other.Symbols().end());
//     return Sentence(combinedSymbols);
//     // return Sentence(this, other);
// }
// Sentence Symbol::operator+(const std::vector<const Symbol*>& symbols) const {
//     // symbol + sentence(symbol1, symbol2,...,symboln) = sentence(symbol, symbol1,...,symboln)
//     std::vector<const Symbol*> combinedSymbols = {this};
//     combinedSymbols.insert(combinedSymbols.end(), symbols.begin(), symbols.end());
//     return Sentence(combinedSymbols);
// }