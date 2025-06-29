#pragma once
#include <memory>
#include <set>
#include "../Grammar/production.h"
#include "../Automata/utils/ContainerSet.h"

/**
 * @class Item
 * @brief Representa un ítem LR(0) para el análisis sintáctico.
 *
 * Un ítem es una producción con un punto (posición) que indica el avance en el reconocimiento de la producción.
 * Se utiliza para construir autómatas LR(0), SLR(1), etc.
 */
class Item {
public:
    /**
     * @brief Constructor de un ítem LR(0).
     * @param production_ Producción asociada al ítem.
     * @param pos_ Posición del punto en la producción.
     */
    Item(std::shared_ptr<Production> production_, int pos_, ContainerSet<string> lookaheads_);

    /**
     * @brief Devuelve la producción asociada al ítem.
     */
    std::shared_ptr<Production> production() const { return production_; }
    /**
     * @brief Devuelve la posición del punto en la producción.
     */
    int pos() const { return pos_; }

    /**
     * @brief Devuelve los lookaheads asociados al ítem.
     * 
     * Los lookaheads son los símbolos que pueden aparecer después del ítem en la entrada.
     */
    const ContainerSet<string>& lookaheads() const { return lookaheads_; }

    /**
     * @brief Fusiona los lookaheads de otro ítem en este ítem.
     * @param other El ítem del cual se fusionarán los lookaheads.
     */
    void merge_lookaheads(const ContainerSet<string>& other) {
        lookaheads_.add(other.get_values());
    }

    /**
     * @brief Indica si el ítem es de reducción (el punto está al final).
     */
    bool IsReduceItem() const;
    /**
     * @brief Devuelve el siguiente símbolo después del punto.
     */
    std::shared_ptr<Symbol> NextSymbol() const;
    /**
     * @brief Devuelve el siguiente ítem (avanza el punto).
     */
    std::shared_ptr<Item> NextItem() const;

    /**
     * @brief Devuelve una lista de posibles secuencias de símbolos que siguen al ítem.
     * 
     * @param skip Número de símbolos a saltar después del punto (default es 1).
     * @return Un vector de vectores de strings representando las secuencias.
     */
    std::vector<std::vector<string>> Preview(int skip = 1) const;
    /**
     * @brief Devuelve el ítem centrado (sin lookaheads).
     */
    std::shared_ptr<Item> Center() const;

    /**
     * @brief Compara si dos ítems son iguales.
     */
    bool operator==(std::shared_ptr<Item> other) const;
    /**
     * @brief Calcula el hash del ítem.
     */
    size_t hash() const;
    /**
     * @brief Devuelve una representación en string del ítem.
     */
    std::string ToString() const;
    /**
     * @brief Devuelve los símbolos beta (los que siguen al punto en la producción).
     * 
     * Los símbolos beta son aquellos que están después del punto en la producción.
     * 
     * @return Un vector de shared_ptr a Symbol representando los símbolos beta.
     */
    vector<shared_ptr<Symbol>> GetBetaSymbols() const {
        vector<shared_ptr<Symbol>> beta;
        const auto& right = production_->Right().Symbols();
        if (pos_ + 1 < right.size()) {
            for (size_t i = pos_ + 1; i < right.size(); ++i) {
                beta.push_back(right[i]);
            }
        }
        return beta;
    }


private:
    std::shared_ptr<Production> production_;
    int pos_;
    ContainerSet<string> lookaheads_;
};

// Non-member operator== for Item
inline bool operator==(const Item& lhs, const Item& rhs) {
    // Comparamos el contenido de las producciones, no solo los punteros
    return (lhs.production() && rhs.production() && 
            lhs.production()->ToString() == rhs.production()->ToString()) &&
            lhs.production()->get_id() == rhs.production()->get_id() &&
           (lhs.pos() == rhs.pos()) &&
           (lhs.lookaheads() == rhs.lookaheads());
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
    if (lhs.pos() != rhs.pos()) {
        return lhs.pos() < rhs.pos();
    }
    // return lhs.pos() < rhs.pos();
    
    // Si todo lo anterior es igual, comparamos los lookaheads
    // Ahora utilizamos el operador < de ContainerSet
    return lhs.lookaheads() < rhs.lookaheads();
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