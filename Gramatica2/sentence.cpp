#include "sentence.h"
#include "symbol.h"

Sentence::Sentence(const Symbol* symbol): symbols{symbol} {}

Sentence::Sentence(const Symbol* symbol1, const Symbol* symbol2): symbols{symbol1, symbol2} {}
/**
 * @brief Inserta al inicio el simbolo `symbol`, luego los demas simbolos de `symbols`.
 */
Sentence::Sentence(const Symbol* symbol, const vector<const Symbol*>& symbols): symbols{symbol} {
    this->symbols.insert(this->symbols.end(), symbols.begin(), symbols.end());
}

Sentence::Sentence(const Symbol* symbol, const Sentence* sentence): symbols{symbol} {
    this->symbols.insert(this->symbols.end(), sentence->symbols.begin(), sentence->symbols.end());
}

Sentence::Sentence(const vector<const Symbol*>& symbols): symbols(symbols) {}

Sentence::Sentence(const vector<const Symbol*>& symbols, const Symbol* symbol): symbols(symbols) {
    this->symbols.push_back(symbol);
}

Sentence::Sentence(const vector<const Symbol*>& symbols1, const vector<const Symbol*>& symbols2): symbols(symbols1) {
    this->symbols.insert(this->symbols.end(), symbols2.begin(), symbols2.end());
}

Sentence::Sentence(const vector<const Symbol*>& symbols1, const Sentence* sentence): symbols(symbols1) {
    this->symbols.insert(this->symbols.end(), sentence->symbols.begin(), sentence->symbols.end());
}

Sentence::Sentence(const Sentence* sentence, const Symbol* symbol): symbols(sentence->symbols) {
    this->symbols.push_back(symbol);
}

Sentence::Sentence(const Sentence* sentence, const vector<const Symbol*>& symbols): symbols(sentence->symbols) {
    this->symbols.insert(this->symbols.end(), symbols.begin(), symbols.end());
}

Sentence::Sentence(const Sentence* sentence1, const Sentence* sentence2): symbols(sentence1->symbols) {
    this->symbols.insert(this->symbols.end(), sentence2->symbols.begin(), sentence2->symbols.end());
}

void Sentence::AddSymbol(const Symbol* symbol) {
    // if(!symbol->IsEpsilon()) {
    //     symbols.push_back(symbol);
    // }
    symbols.push_back(symbol);
}

const std::vector<const Symbol*>& Sentence::Symbols() const {
    return symbols;
}
Sentence Sentence::operator+(const Sentence& other) const {
    std::vector<const Symbol*> combinedSymbols = symbols;
    combinedSymbols.insert(combinedSymbols.end(), other.symbols.begin(), other.symbols.end());
    return Sentence(combinedSymbols);
}
Sentence Sentence::operator+(const vector<const Symbol*>& symbols) {
    std::vector<const Symbol*> combinedSymbols = this->symbols;
    combinedSymbols.insert(combinedSymbols.end(), symbols.begin(), symbols.end());
    return Sentence(combinedSymbols);
}
Sentence Sentence::operator+(const Symbol& other) const {
    std::vector<const Symbol*> combinedSymbols = symbols;
    combinedSymbols.push_back(&other);
    return Sentence(combinedSymbols);
}