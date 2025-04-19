
#include <vector>
#include <memory>

#ifndef SENTENCE_H
#define SENTENCE_H
using namespace std;
class Symbol;

class Sentence {
public:
    Sentence() = default;
    explicit Sentence(const Symbol* symbol);
    explicit Sentence(const Symbol* symbol1, const Symbol* symbol2);
    explicit Sentence(const Symbol* symbol, const vector<const Symbol*>& symbols);
    explicit Sentence(const Symbol* symbol, const Sentence* sentence);
    explicit Sentence(const vector<const Symbol*>& symbols);
    explicit Sentence(const vector<const Symbol*>& symbols, const Symbol* symbol);
    explicit Sentence(const vector<const Symbol*>& symbols1, const vector<const Symbol*>& symbols2);
    explicit Sentence(const vector<const Symbol*>& symbols1, const Sentence* sentence);
    explicit Sentence(const Sentence* sentence, const Symbol* symbol);
    explicit Sentence(const Sentence* sentence, const vector<const Symbol*>& symbols);
    explicit Sentence(const Sentence* sentence1, const Sentence* sentence2);
    
    void AddSymbol(const Symbol* symbol);
    const std::vector<const Symbol*>& Symbols() const;
    
    Sentence operator+(const Sentence& other) const;
    Sentence operator+(const vector<const Symbol*>& symbols);
    Sentence operator+(const Symbol& other) const;

private:
    std::vector<const Symbol*> symbols;
};
#endif