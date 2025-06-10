#include "symbol.h"
#include "sentence.h"
#include "production.h"
#include "grammar.h"
#include <stdexcept>
#include <memory>

Production::Production(std::shared_ptr<NonTerminal> left, const Sentence& right) : left(left), right(right) {}

std::shared_ptr<NonTerminal> Production::Left() const { return left; }
const Sentence& Production::Right() const { return right; }

std::string Production::ToString() const {
    std::string result = left->ToString() + " -> ";
    for (const auto& symbol : right.Symbols()) {
        result += symbol->ToString() + " ";
    }
    return result;
}

AttrProd::AttrProd(std::shared_ptr<NonTerminal> left, const Sentence& right, SemanticAction attr)
    : Production(left, right), attribute(attr) {}

const AttrProd::SemanticAction& AttrProd::Attribute() const { return attribute; }

// AttrProd::NodePtr AttrProd::Execute(const vector<NodePtr>& inherited, const vector<NodePtr>& synthesized) const {
//     if (!action) return {};
//     return action(inherited, synthesized);
// }

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

// void NonTerminal::operator%=(AttributeProduction::ProdDef rhs) {
//     auto production = std::make_shared<AttributeProduction>(this->grammar.GetStartSymbol(), rhs.sentence, rhs.actions);
//     productions.push_back(*production);
//     this->grammar.AddProduction(*production);
// }
// void NonTerminal::operator%=(AttrProd::ProdDef rhs) {
//     auto prod = std::make_shared<AttrProd>(
//         std::static_pointer_cast<NonTerminal>(shared_from_this()),
//         rhs.sentence,
//         rhs.actions
//     );
//     grammar.AddProduction(*prod);
// }
// void NonTerminal::operator%=(std::initializer_list<std::variant<Sentence, std::vector<AttrProd::SemanticAction>>> list) {
//     if (list.size() != 2) {
//         throw std::runtime_error("Initializer list must have exactly two elements");
//     }

//     // Extraer los elementos de la lista
//     auto it = list.begin();
//     Sentence sentence = std::get<Sentence>(*it);
//     ++it;
//     std::vector<AttrProd::SemanticAction> actions = std::get<std::vector<AttrProd::SemanticAction>>(*it);

//     // Crear y agregar la producción
//     auto prod = std::make_shared<AttrProd>(
//         std::static_pointer_cast<NonTerminal>(shared_from_this()),
//         sentence,
//         actions
//     );
//     grammar.AddProduction(*prod);
// }