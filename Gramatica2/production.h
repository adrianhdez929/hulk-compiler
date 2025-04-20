
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
    // Funcion semántica que toma un vector de atributos heredados y un vector de atributos sintetizados
    using SemanticAction = function<any(const vector<any> inherited, const vector<any>& synthesized)>;
    
    AttributeProduction(const NonTerminal& left, 
                        const Sentence& right, 
                        const vector<SemanticAction>& actions);

    const vector<SemanticAction>& Attributes() const;
    any Execute(const vector<any>& inherited, //const any& inherited, 
        const vector<any>& synthesized) const;

    // ProdDef es una estructura que contiene una producción y sus acciones semánticas
    // Para mejorar la legibilidad y evitar el uso de std::pair y std::vector<std::pair<Sentence, std::vector<AttributeProduction::SemanticAction>>>
    struct ProdDef {
        Sentence sentence;
        vector<SemanticAction> actions;
    };

private:
    vector<SemanticAction> attributes;
};

class NonTerminal : public Symbol {
    public:
        NonTerminal(const string& name, Grammar& grammar);
        bool IsNonTerminal() const override;
     
        vector<Production> productions;
        // NonTerminal& operator%=(const std::pair<Sentence, std::vector<AttributeProduction::SemanticAction>>& rhs);
        // NonTerminal& operator%=(const Sentence& rhs);

        // Sobrecarga del operador %= para agregar producciones con atributos
        void operator%=(AttributeProduction::ProdDef rhs);
};

#endif