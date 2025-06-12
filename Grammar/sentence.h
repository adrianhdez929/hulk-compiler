
#include <vector>
#include <memory>
#include "symbol.h"

#ifndef SENTENCE_H
#define SENTENCE_H
using namespace std;

class Sentence {
public:
    Sentence() = default;
    explicit Sentence(std::shared_ptr<Symbol> symbol);
    explicit Sentence(std::shared_ptr<Symbol> symbol1, std::shared_ptr<Symbol> symbol2);
    explicit Sentence(std::shared_ptr<Symbol> symbol, const std::vector<std::shared_ptr<Symbol>>& symbols);
    explicit Sentence(std::shared_ptr<Symbol> symbol, const Sentence& sentence);
    explicit Sentence(const std::vector<std::shared_ptr<Symbol>>& symbols);
    explicit Sentence(const std::vector<std::shared_ptr<Symbol>>& symbols, std::shared_ptr<Symbol> symbol);
    explicit Sentence(const std::vector<std::shared_ptr<Symbol>>& symbols1, const std::vector<std::shared_ptr<Symbol>>& symbols2);
    explicit Sentence(const std::vector<std::shared_ptr<Symbol>>& symbols1, const Sentence& sentence);
    explicit Sentence(const Sentence& sentence, std::shared_ptr<Symbol> symbol);
    explicit Sentence(const Sentence& sentence, const std::vector<std::shared_ptr<Symbol>>& symbols);
    explicit Sentence(const Sentence& sentence1, const Sentence& sentence2);

    void AddSymbol(std::shared_ptr<Symbol> symbol);
    const std::vector<std::shared_ptr<Symbol>>& Symbols() const;
    size_t Size() const;
    
    Sentence operator+(const Sentence& other) const;
    Sentence operator+(const std::vector<std::shared_ptr<Symbol>>& symbols) const;
    Sentence operator+(std::shared_ptr<Symbol> other) const;

    // ToString
    std::string ToString() const;
    bool operator<(const Sentence& other) const {
        return symbols < other.symbols;
}

private:
    std::vector<std::shared_ptr<Symbol>> symbols;  // Cambiado a Symbol
};
#endif