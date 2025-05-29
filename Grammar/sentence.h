
#include <vector>
#include <memory>

#ifndef SENTENCE_H
#define SENTENCE_H
using namespace std;
class Symbol;

class Sentence {
public:
    Sentence() = default;
    explicit Sentence(std::shared_ptr<const Symbol> symbol);
    explicit Sentence(std::shared_ptr<const Symbol> symbol1, std::shared_ptr<const Symbol> symbol2);
    explicit Sentence(std::shared_ptr<const Symbol> symbol, const std::vector<std::shared_ptr<const Symbol>>& symbols);
    explicit Sentence(std::shared_ptr<const Symbol> symbol, const Sentence& sentence);
    explicit Sentence(const std::vector<std::shared_ptr<const Symbol>>& symbols);
    explicit Sentence(const std::vector<std::shared_ptr<const Symbol>>& symbols, std::shared_ptr<const Symbol> symbol);
    explicit Sentence(const std::vector<std::shared_ptr<const Symbol>>& symbols1, const std::vector<std::shared_ptr<const Symbol>>& symbols2);
    explicit Sentence(const std::vector<std::shared_ptr<const Symbol>>& symbols1, const Sentence& sentence);
    explicit Sentence(const Sentence& sentence, std::shared_ptr<const Symbol> symbol);
    explicit Sentence(const Sentence& sentence, const std::vector<std::shared_ptr<const Symbol>>& symbols);
    explicit Sentence(const Sentence& sentence1, const Sentence& sentence2);

    void AddSymbol(std::shared_ptr<const Symbol> symbol);
    const std::vector<std::shared_ptr<const Symbol>>& Symbols() const;
    
    Sentence operator+(const Sentence& other) const;
    Sentence operator+(const std::vector<std::shared_ptr<const Symbol>>& symbols) const;
    Sentence operator+(std::shared_ptr<const Symbol> other) const;

private:
    std::vector<std::shared_ptr<const Symbol>> symbols; 
};
#endif