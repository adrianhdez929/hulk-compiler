#include "grammar.h"
#include "symbol.h"
#include <typeindex>
#include <stdexcept>

Epsilon::Epsilon(Grammar& grammar) : Symbol("epsilon", grammar) {}
bool Epsilon::IsEpsilon() const { return true; }

EndOfFile::EndOfFile(Grammar& grammar) : Symbol("EOF", grammar) {}
bool EndOfFile::IsEndOfFile() const { return true; }

Grammar::Grammar() : productionType(typeid(void)) {
    // augmentedStartSymbol = nullptr;
    
    // Initialize epsilon and EOF symbols
    epsilon = std::make_shared<Epsilon>(*this);
    eof = std::make_shared<EndOfFile>(*this);
    
    // Add epsilon and EOF to the symbol map
    symbolMap["epsilon"] = epsilon.get();
    symbolMap["EOF"] = eof.get();
    
    // Add epsilon and EOF to the symbols vector
    symbols.push_back(epsilon);
    symbols.push_back(eof);
};

const std::shared_ptr<Epsilon>& Grammar::GetEpsilon() const { return epsilon; }
const std::shared_ptr<EndOfFile>& Grammar::GetEndOfFile() const { return eof; }
const std::vector<std::shared_ptr<Symbol>>& Grammar::Symbols() const {
    return symbols;
}

std::shared_ptr<NonTerminal> Grammar::SetNonTerminal(const std::string& name, bool isStart) {
    if(symbolMap.find(name) != symbolMap.end()) {
        throw std::runtime_error("Symbol already defined: " + name);
    }
    auto ptr = std::make_shared<NonTerminal>(name, *this);
    
    if(isStart) {
        // if(startSymbol) throw std::runtime_error("Start symbol already defined");
        startSymbol = ptr;
    }
    
    this->symbolMap[name] = ptr.get();
    this->symbols.push_back(ptr);
    this->nonTerminals.push_back(ptr);
    return ptr;
}

std::shared_ptr<Terminal> Grammar::SetTerminal(const std::string& name) {
    if(symbolMap.find(name) != symbolMap.end()) {
        throw std::runtime_error("Symbol already defined: " + name);
    }
    auto ptr = std::make_shared<Terminal>(name, *this);
    
    this->symbolMap[name] = ptr.get();
    this->symbols.push_back(ptr);
    this->terminals.push_back(ptr);
    return ptr;
}

std::shared_ptr<Epsilon> Grammar::SetEpsilon() {
    if(!epsilon) {
        epsilon = std::make_shared<Epsilon>(*this);
        symbols.push_back(epsilon);
        symbolMap["epsilon"] = epsilon.get();
    }
    return epsilon;
}
std::shared_ptr<EndOfFile> Grammar::SetEndOfFile() {
    if(!eof) {
        eof = std::make_shared<EndOfFile>(*this);
        symbols.push_back(eof);
        symbolMap["EOF"] = eof.get();
    }
    return eof;
}

static int production_counter = 0;

void Grammar::AddProduction(const Production& production) {
    Production new_production = production;
    new_production.set_id(production_counter++);
    productions.push_back(new_production);
    // Get the left non-terminal and add the production to its list
    auto left = production.Left();
    if (left) {
        auto it = std::find(nonTerminals.begin(), nonTerminals.end(), left);
        if (it != nonTerminals.end()) {
            (*it)->productions.push_back(new_production);
        } else {
            throw std::runtime_error("Left non-terminal not found in grammar: " + left->Name());
        }
    } else {
        throw std::runtime_error("Production left side is null");
    }
}

void Grammar::AddProduction(const AttrProd& production) {
    if(productions.empty()) {
        productionType = typeid(AttrProd);
    }
    productions.emplace_back(production);
}

// const std::vector<Grammar::ProductionVariant>& Grammar::Productions() const {
//     return productions;
// }
const std::vector<Production>& Grammar::Productions() const {
    return productions;
}

const std::shared_ptr<NonTerminal>& Grammar::GetStartSymbol() const {
    if(!startSymbol) throw std::runtime_error("Start symbol not defined");
    return startSymbol;
}

// const std::vector<const Symbol*>& Grammar::Symbols() const {
//     return symbols;
// }

std::vector<std::shared_ptr<NonTerminal>> Grammar::NonTerminals() {
    return nonTerminals;
}
std::vector<std::shared_ptr<Terminal>> Grammar::Terminals() {
    return terminals;
}

void Grammar::Augment() {
    if (augmentedStartSymbol) return;

    auto previousStartSymbol = startSymbol;
    augmentedStartSymbol = SetNonTerminal("SS", true);

    auto startProduction = std::make_shared<Production>(
        augmentedStartSymbol,
        Sentence(previousStartSymbol)
    );

    AddProduction(*startProduction);
}

bool Grammar::IsAugmented() const {
    if (!augmentedStartSymbol) return false;

    int count = 0;
    for (const auto& prod : productions) {
        if (prod.Left()->Name() == "SS") {
            count++;
        }
    }
    return count == 1;
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
        result += nt->ToString() + ", ";
    }
    result += "\n";
    result += "Terminals:\n";
    for (const auto& t : terminals) {
        result += t->ToString() + ", ";
    }
    result += "\n";
    result += "Productions:\n";
    for (const auto& p : productions) {
        result += "  " + p.ToString() + ", ";
    }
    // for (const auto& p : productions) {
    //     if (std::holds_alternative<AttrProd>(p)) {
    //         result += "  " + std::get<AttrProd>(p).ToString() + ", ";
    //     }
    // }
    // for (const auto& p : productions) {
    //     if (std::holds_alternative<Production>(p)) {
    //         result += "  " + std::get<Production>(p).ToString() + ", ";
    //     } else {
    //         result += "  " + std::get<AttrProd>(p).ToString() + ", ";
    //     }
    // }
    result += "\n";
    return result;
}
