
#include "symbol.h"
#include "sentence.h"
#include <functional>
#include <vector>
#include <any>

#ifndef PRODUCTION_H
#define PRODUCTION_H

using namespace std;

class NonTerminal;

class Production {
public:
    Production(const NonTerminal& left, const Sentence& right);
    const NonTerminal& Left() const;
    const Sentence& Right() const;

    std::string ToString() const;
private:
    const NonTerminal& left;
    Sentence right;
};

class AttributeProduction : public Production {
public:
    using SemanticAction = function<any(const vector<any> inherited, const vector<any>& synthesized)>;
    
    AttributeProduction(const NonTerminal& left, 
                        const Sentence& right, 
                        const vector<SemanticAction>& actions);

    const vector<SemanticAction>& Attributes() const;
    any Execute(const vector<any>& inherited, //const any& inherited, 
        const vector<any>& synthesized) const;
private:
    vector<SemanticAction> attributes;
};

class NonTerminal : public Symbol {
    public:
        NonTerminal(const string& name, Grammar& grammar);
        bool IsNonTerminal() const override;
     
        vector<Production> productions;
        NonTerminal& operator%=(const std::pair<Sentence, std::vector<AttributeProduction::SemanticAction>>& rhs);
        NonTerminal& operator%=(const Sentence& rhs);
};


// class NonTerminal {
// public:
//     NonTerminal(const std::string& name, Grammar& grammar);
//     bool IsTerminal() const override;
//     NonTerminal& operator%=(const std::pair<Sentence, std::vector<AttributeProduction::SemanticAction>>& rhs);
//     std::vector<Production> productions;
// private:
//     std::string name;
//     Grammar& grammar;
// }

#endif