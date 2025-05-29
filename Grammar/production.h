
#include "symbol.h"
#include "sentence.h"
#include <functional>
#include <vector>
#include <any>
#include <variant>
#include <initializer_list>

#ifndef PRODUCTION_H
#define PRODUCTION_H

using namespace std;

class Node;
class NonTerminal;

class Production {
public:
    Production(std::shared_ptr<NonTerminal> left, const Sentence& right);
    std::shared_ptr<NonTerminal> Left() const;
    const Sentence& Right() const;

    std::string ToString() const;
private:
    std::shared_ptr<NonTerminal> left;
    Sentence right;
};

class AttrProd : public Production {
public:
    // Funcion semántica que toma un vector de atributos heredados y un vector de atributos sintetizados
    // using SemanticAction = function<any(const vector<any>& inherited, const vector<any>& synthesized)>;

    using NodePtr = std::shared_ptr<Node>;
    using SemanticAction = std::function<
        NodePtr(const std::vector<NodePtr>& h,
                const std::vector<NodePtr>& s)
        >;
    
    AttrProd(std::shared_ptr<NonTerminal> left,
                        const Sentence& right, 
                        const vector<SemanticAction>& actions);

    const vector<SemanticAction>& Attributes() const;//CONVERTIR EN UN SOLO ATRIBUTO
    NodePtr Execute(const vector<NodePtr>& inherited, //const any& inherited, 
                    const vector<NodePtr>& synthesized) const;

    // ProdDef es una estructura que contiene una producción y sus acciones semánticas
    // Para mejorar la legibilidad y evitar el uso de std::pair y std::vector<std::pair<Sentence, std::vector<AttributeProduction::SemanticAction>>>
    struct ProdDef {
        Sentence sentence;
        vector<SemanticAction> actions;
    };

private:
    vector<SemanticAction> attributes; //CONVERTIR EN UN SOLO ATRIBUTO
};

class NonTerminal : public Symbol {
    public:
        NonTerminal(const string& name, Grammar& grammar);
        bool IsNonTerminal() const override;
     
        vector<Production> productions;
        // NonTerminal& operator%=(const std::pair<Sentence, std::vector<AttributeProduction::SemanticAction>>& rhs);
        // NonTerminal& operator%=(const Sentence& rhs);

        // Sobrecarga del operador %= para agregar producciones con atributos
        // void operator%=(AttrProd::ProdDef rhs);
        // void operator%=(std::initializer_list<std::variant<Sentence, std::vector<AttrProd::SemanticAction>>> list);
};

#endif