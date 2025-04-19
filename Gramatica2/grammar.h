
#include <vector>
#include <unordered_map>
#include <memory>
#include <variant>
#include <typeindex>
#include "symbol.h"
#include "production.h"

#ifndef GRAMMAR_H
#define GRAMMAR_H

using namespace std;

// class Terminal;
// class NonTerminal;
class Epsilon : public Symbol {
    public:
        Epsilon(Grammar& grammar);
        bool IsEpsilon() const override;
};

class EndOfFile : public Symbol {
    public:
        EndOfFile(Grammar& grammar);
        bool IsEndOfFile() const override;
};

class Grammar {
    public:
        Grammar();
        NonTerminal& SetNonTerminal(const std::string& name, bool isStart = false);
        vector<NonTerminal>& NonTerminals();
        Terminal& SetTerminal(const std::string& name);
        vector<Terminal>& Terminals();
        // void AddProduction(const AttributeProduction& production);
        using ProductionVariant = std::variant<Production, AttributeProduction>;
        void AddProduction(const AttributeProduction& production);
        void AddProduction(const Production& production);
        const std::vector<ProductionVariant>& Productions() const;
        const NonTerminal& GetStartSymbol() const;
        const Epsilon& GetEpsilon() const;
        const EndOfFile& GetEndOfFile() const;
        //Metodo para obtener todos los simbolos
        // const std::vector<const Symbol*>& Symbols() const;
        //Sobreescribir el .tostring
        std::string ToString() const;
    private:
        std::vector<std::unique_ptr<Symbol>> symbols;
        std::vector<NonTerminal> nonTerminals;
        std::vector<Terminal> terminals;
        std::vector<ProductionVariant> productions;
        std::type_index productionType;
        const NonTerminal* startSymbol = nullptr;
        std::unique_ptr<Epsilon> epsilon;
        std::unique_ptr<EndOfFile> eof;
        std::unordered_map<std::string, Symbol*> symbolMap;
};

#endif