#include "symbol.h"
#include "sentence.h"
#include <functional>
#include <vector>
// #include <any>
#include <variant>
#include <initializer_list>
#include "../Lexer/SpecialTypes.h"

#ifndef PRODUCTION_H
#define PRODUCTION_H

using namespace std;

/**
 * @class Production
 * @brief Representa una producción de la gramática (regla de derivación).
 * 
 * Una producción tiene un lado izquierdo (no terminal) y un lado derecho (sentencia de símbolos).
 */

class Node;
class NonTerminal;

class Production {
public:
    Production();  // Default constructor
    Production(std::shared_ptr<NonTerminal> left, const Sentence& right);
    std::shared_ptr<NonTerminal> Left() const;
    const Sentence& Right() const;

    bool operator<(const Production& other) const {
        string this_str = ToString();
        string other_str = other.ToString();
        return this_str < other_str;
    }
    bool operator==(const Production& other) const {
        string this_str = ToString();
        string other_str = other.ToString();
        return this_str == other_str;
    }

    std::string ToString() const;
private:
    std::shared_ptr<NonTerminal> left;
    Sentence right;
    int id_;

public:
    void set_id(int id) { id_ = id; }
    int get_id() const { return id_; }
};

/**
 * @class AttrProd
 * @brief Producción con acción semántica asociada (atributos).
 * 
 * Permite asociar una función semántica a la producción para gramáticas atribuidas.
 */

class AttrProd : public Production {
public:
    // Funcion semántica que toma un vector de atributos heredados y un vector de atributos sintetizados
    // using SemanticAction = function<any(const vector<any>& inherited, const vector<any>& synthesized)>;
    using SemanticAction = std::function<ElementType(const std::vector<ElementType>& synthesized)>;

    // using NodePtr = std::shared_ptr<Node>;
    // using SemanticAction = std::function<
    //     NodePtr(const std::vector<NodePtr>& h,
    //             const std::vector<NodePtr>& s)
    //     >;
    
    AttrProd(std::shared_ptr<NonTerminal> left,
                        const Sentence& right, 
                        SemanticAction attr);

    const SemanticAction& Attribute() const;
    // const vector<SemanticAction>& Attributes() const;//CONVERTIR EN UN SOLO ATRIBUTO
    // NodePtr Execute(const vector<NodePtr>& inherited, //const any& inherited, 
    //                 const vector<NodePtr>& synthesized) const;

    // ProdDef es una estructura que contiene una producción y sus acciones semánticas
    // Para mejorar la legibilidad y evitar el uso de std::pair y std::vector<std::pair<Sentence, std::vector<AttributeProduction::SemanticAction>>>
    // struct ProdDef {
    //     Sentence sentence;
    //     vector<SemanticAction> actions;
    // };

private:
    SemanticAction attribute; //CONVERTIR EN UN SOLO ATRIBUTO
};

/**
 * @class NonTerminal
 * @brief Representa un símbolo no terminal de la gramática.
 * 
 * Los no terminales se usan para definir las reglas de la gramática.
 */

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