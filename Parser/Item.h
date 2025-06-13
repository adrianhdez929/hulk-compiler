#pragma once
#include <memory>
#include <set>
#include "../Grammar/production.h"
#include "../Automata/utils/ContainerSet.h"

class Item {
public:
    Item(std::shared_ptr<Production> production_, int pos_);//, ContainerSet<string> lookaheads_);

    std::shared_ptr<Production> production() const { return production_; }
    int pos() const { return pos_; }
    //const ContainerSet<string>& lookaheads() const { return lookaheads_; }

    bool IsReduceItem() const;
    std::shared_ptr<Symbol> NextSymbol() const;
    std::shared_ptr<Item> NextItem() const;
    // std::vector<std::vector<string>> Preview(int skip = 1) const;
    std::shared_ptr<Item> Center() const;

    bool operator==(std::shared_ptr<Item> other) const;
    size_t hash() const;
    std::string ToString() const;


private:
    std::shared_ptr<Production> production_;
    int pos_;
    //ContainerSet<string> lookaheads_;
};

// Non-member operator== for Item
inline bool operator==(const Item& lhs, const Item& rhs) {
    // Comparamos el contenido de las producciones, no solo los punteros
    return (lhs.production() && rhs.production() && 
            lhs.production()->ToString() == rhs.production()->ToString()) &&
           (lhs.pos() == rhs.pos()); //&&
           //(lhs.lookaheads() == rhs.lookaheads());
}

// Non-member operator< for Item
inline bool operator<(const Item& lhs, const Item& rhs) {
    // Primero comparamos las producciones por su contenido
    if (lhs.production() && rhs.production()) {
        string lhs_prod = lhs.production()->ToString();
        string rhs_prod = rhs.production()->ToString();
        if (lhs_prod != rhs_prod) {
            return lhs_prod < rhs_prod;
        }
    } else if (lhs.production().get() != rhs.production().get()) {
        return lhs.production().get() < rhs.production().get();
    }
    
    // Si son iguales, comparamos las posiciones
    // if (lhs.pos() != rhs.pos()) {
    //     return lhs.pos() < rhs.pos();
    // }
    return lhs.pos() < rhs.pos();
    
    // Si todo lo anterior es igual, comparamos los lookaheads
    // Ahora utilizamos el operador < de ContainerSet
    //return lhs.lookaheads() < rhs.lookaheads();
}

namespace std {
    template<> struct hash<Item> {
        size_t operator()(const Item& item) const {
            return item.hash();
        }
    };

    template<> struct equal_to<Item> {
        bool operator()(const Item& lhs, const Item& rhs) const {
            return lhs == rhs;
        }
    };
}