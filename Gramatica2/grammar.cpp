#include "grammar.h"
#include "symbol.h"
#include <typeindex>
#include <stdexcept>

Epsilon::Epsilon(Grammar& grammar) : Symbol("epsilon", grammar) {}
bool Epsilon::IsEpsilon() const { return true; }

EndOfFile::EndOfFile(Grammar& grammar) : Symbol("EOF", grammar) {}
bool EndOfFile::IsEndOfFile() const { return true; }

Grammar::Grammar() : productionType(typeid(void)) {
    // Initialize epsilon and EOF symbols
    epsilon = std::make_unique<Epsilon>(*this);
    eof = std::make_unique<EndOfFile>(*this);
    
    // Add epsilon and EOF to the symbol map
    symbolMap["epsilon"] = epsilon.get();
    symbolMap["EOF"] = eof.get();
    
    // Add epsilon and EOF to the symbols vector
    symbols.push_back(std::move(epsilon));
    symbols.push_back(std::move(eof));
};

const Epsilon& Grammar::GetEpsilon() const { return *epsilon; }
const EndOfFile& Grammar::GetEndOfFile() const { return *eof; }

NonTerminal& Grammar::SetNonTerminal(const std::string& name, bool isStart) {
    auto ptr = std::make_unique<NonTerminal>(name, *this);
    NonTerminal& ref = *ptr;
    
    if(isStart) {
        if(startSymbol) throw std::runtime_error("Start symbol already defined");
        startSymbol = &ref;
    }
    
    this->symbolMap[name] = &ref;
    this->symbols.push_back(std::move(ptr));
    this->nonTerminals.push_back(ref);
    return ref;
}

Terminal& Grammar::SetTerminal(const std::string& name) {
    auto ptr = std::make_unique<Terminal>(name, *this);
    Terminal& ref = *ptr;
    
    this->symbolMap[name] = &ref;
    this->symbols.push_back(std::move(ptr));
    this->terminals.push_back(ref);
    return ref;
}

void Grammar::AddProduction(const Production& production) {
    if(productions.empty()) {
        productionType = typeid(Production);
    }
    productions.emplace_back(production);
}

void Grammar::AddProduction(const AttributeProduction& production) {
    if(productions.empty()) {
        productionType = typeid(AttributeProduction);
    }
    productions.emplace_back(production);
}

const std::vector<Grammar::ProductionVariant>& Grammar::Productions() const {
    return productions;
}

const NonTerminal& Grammar::GetStartSymbol() const {
    if(!startSymbol) throw std::runtime_error("Start symbol not defined");
    return *startSymbol;
}

// const std::vector<const Symbol*>& Grammar::Symbols() const {
//     return symbols;
// }

std::vector<NonTerminal>& Grammar::NonTerminals() {
    return nonTerminals;
}
std::vector<Terminal>& Grammar::Terminals() {
    return terminals;
}
std::string Grammar::ToString() const {
    //Formato:
    // NonTerminals:
    //     NonTerminal1, NonTerminal2, ...
    // Terminals:
    //     Terminal1, Terminal2, ...
    // Productions:
    //     Production1, Production2, ...
    std::string result;
    result += "NonTerminals:\n";
    for (const auto& nt : nonTerminals) {
        result += nt.ToString() + ", ";
    }
    result += "\n";
    result += "Terminals:\n";
    for (const auto& t : terminals) {
        result += t.ToString() + ", ";
    }
    result += "\n";
    result += "Productions:\n";
    for (const auto& p : productions) {
        if (std::holds_alternative<Production>(p)) {
            result += "  " + std::get<Production>(p).ToString() + ", ";
        } else {
            result += "  " + std::get<AttributeProduction>(p).ToString() + ", ";
        }
    }
    result += "\n";
    return result;
}
