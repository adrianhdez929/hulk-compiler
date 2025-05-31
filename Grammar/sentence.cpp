#include "sentence.h"
#include "symbol.h"
#include "sentence.h"
#include "symbol.h"

Sentence::Sentence(std::shared_ptr<Symbol> symbol): symbols{symbol} {}

Sentence::Sentence(std::shared_ptr<Symbol> symbol1, std::shared_ptr<Symbol> symbol2): symbols{symbol1, symbol2} {}

/**
 * @brief Inserta al inicio el simbolo `symbol`, luego los demas simbolos de `symbols`.
 */
Sentence::Sentence(std::shared_ptr<Symbol> symbol, const std::vector<std::shared_ptr<Symbol>>& symbols): symbols{symbol} {
    this->symbols.insert(this->symbols.end(), symbols.begin(), symbols.end());
}

Sentence::Sentence(std::shared_ptr<Symbol> symbol, const Sentence& sentence): symbols{symbol} {
    this->symbols.insert(this->symbols.end(), sentence.symbols.begin(), sentence.symbols.end());
}

Sentence::Sentence(const std::vector<std::shared_ptr<Symbol>>& symbols): symbols(symbols) {}

Sentence::Sentence(const std::vector<std::shared_ptr<Symbol>>& symbols, std::shared_ptr<Symbol> symbol): symbols(symbols) {
    this->symbols.push_back(symbol);
}

Sentence::Sentence(const std::vector<std::shared_ptr<Symbol>>& symbols1, const std::vector<std::shared_ptr<Symbol>>& symbols2): symbols(symbols1) {
    this->symbols.insert(this->symbols.end(), symbols2.begin(), symbols2.end());
}

Sentence::Sentence(const std::vector<std::shared_ptr<Symbol>>& symbols1, const Sentence& sentence): symbols(symbols1) {
    this->symbols.insert(this->symbols.end(), sentence.symbols.begin(), sentence.symbols.end());
}

Sentence::Sentence(const Sentence& sentence, std::shared_ptr<Symbol> symbol): symbols(sentence.symbols) {
    this->symbols.push_back(symbol);
}

Sentence::Sentence(const Sentence& sentence, const std::vector<std::shared_ptr<Symbol>>& symbols): symbols(sentence.symbols) {
    this->symbols.insert(this->symbols.end(), symbols.begin(), symbols.end());
}

Sentence::Sentence(const Sentence& sentence1, const Sentence& sentence2): symbols(sentence1.symbols) {
    this->symbols.insert(this->symbols.end(), sentence2.symbols.begin(), sentence2.symbols.end());
}

void Sentence::AddSymbol(std::shared_ptr<Symbol> symbol) {
    symbols.push_back(symbol);
}

const std::vector<std::shared_ptr<Symbol>>& Sentence::Symbols() const {
    return symbols;
}

Sentence Sentence::operator+(const Sentence& other) const {
    std::vector<std::shared_ptr<Symbol>> combinedSymbols = symbols;
    combinedSymbols.insert(combinedSymbols.end(), other.symbols.begin(), other.symbols.end());
    return Sentence(combinedSymbols);
}

Sentence Sentence::operator+(const std::vector<std::shared_ptr<Symbol>>& symbols) const {
    std::vector<std::shared_ptr<Symbol>> combinedSymbols = this->symbols;
    combinedSymbols.insert(combinedSymbols.end(), symbols.begin(), symbols.end());
    return Sentence(combinedSymbols);
}

Sentence Sentence::operator+(std::shared_ptr<Symbol> other) const {
    std::vector<std::shared_ptr<Symbol>> combinedSymbols = symbols;
    combinedSymbols.push_back(other);
    return Sentence(combinedSymbols);
}