#include "symbol.h"
#include "sentence.h"
#include "production.h"
#include "grammar.h"
#include <stdexcept>

Production::Production(const NonTerminal& left, const Sentence& right) : left(left), right(right) {}

const NonTerminal& Production::Left() const { return left; }
const Sentence& Production::Right() const { return right; }

std::string Production::ToString() const {
    std::string result = left.ToString() + " -> ";
    for (const auto& symbol : right.Symbols()) {
        result += symbol->ToString() + " ";
    }
    return result;
}

AttributeProduction::AttributeProduction(const NonTerminal& left, const Sentence& right, const vector<SemanticAction>& actions)
    : Production(left, right), attributes(actions) {}

const vector<AttributeProduction::SemanticAction>& AttributeProduction::Attributes() const { return attributes; }

any AttributeProduction::Execute(const vector<any>& inherited, const vector<any>& synthesized) const {
    // if(attributes.empty()) return {};
    // return attributes[0](inherited, synthesized);
    throw std::runtime_error("Not implemented");
}

NonTerminal::NonTerminal(const std::string& name, Grammar& grammar) : Symbol(name, grammar) {}
bool NonTerminal::IsNonTerminal() const { return true; }

// NonTerminal& NonTerminal::operator%=(const std::pair<Sentence, std::vector<AttributeProduction::SemanticAction>>& rhs) {
//     auto production = std::make_unique<AttributeProduction>(this->grammar.GetStartSymbol(), rhs.first, rhs.second);
//     productions.push_back(*production);
//     this->grammar.AddProduction(*production);
//     return *this;
// }

// NonTerminal& NonTerminal::operator%=(const Sentence& rhs) {
//     auto production = std::make_unique<Production>(this->grammar.GetStartSymbol(), rhs);
//     productions.push_back(*production);
//     this->grammar.AddProduction(*production);
//     return *this;
// }

void NonTerminal::operator%=(AttributeProduction::ProdDef rhs) {
    auto production = std::make_unique<AttributeProduction>(this->grammar.GetStartSymbol(), rhs.sentence, rhs.actions);
    productions.push_back(*production);
    this->grammar.AddProduction(*production);
}