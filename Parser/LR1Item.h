#pragma once
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "Item.h"
#include "../Automata/utils/ContainerSet.h"

/**
 * @class LR1Item
 * @brief Representa un ítem LR(1) para el análisis sintáctico.
 *
 * Un ítem LR(1) es una extensión del ítem LR(0) que incluye un conjunto de tokens de anticipación
 * (lookahead). Estos tokens indican qué terminales pueden seguir a la producción en una derivación.
 */
class LR1Item {
public:
    /**
     * @brief Constructor de un ítem LR(1).
     * @param production Producción asociada al ítem.
     * @param pos Posición del punto en la producción.
     * @param lookahead Token de anticipación para este ítem.
     */
    LR1Item(std::shared_ptr<Production> production, int pos, const std::string& lookahead);
    
    /**
     * @brief Constructor alternativo que acepta AttrProd directamente.
     * @param production Producción con atributos.
     * @param pos Posición del punto en la producción.
     * @param lookahead Token de anticipación para este ítem.
     */
    LR1Item(const AttrProd& production, int pos, const std::string& lookahead);

    /**
     * @brief Constructor a partir de un ítem LR(0) y un lookahead.
     * @param item Item LR(0) base.
     * @param lookahead Token de anticipación.
     */
    LR1Item(const std::shared_ptr<Item>& item, const std::string& lookahead);

    /**
     * @brief Devuelve la producción asociada al ítem.
     */
    std::shared_ptr<Production> production() const { return item_->production(); }
    
    /**
     * @brief Devuelve la posición del punto en la producción.
     */
    int pos() const { return item_->pos(); }
    
    /**
     * @brief Devuelve el ítem LR(0) base.
     */
    std::shared_ptr<Item> baseItem() const { return item_; }
    
    /**
     * @brief Devuelve el token de anticipación.
     */
    const std::string& lookahead() const { return lookahead_; }

    /**
     * @brief Indica si el ítem es de reducción (el punto está al final).
     */
    bool IsReduceItem() const { return item_->IsReduceItem(); }
    
    /**
     * @brief Devuelve el siguiente símbolo después del punto.
     */
    std::shared_ptr<Symbol> NextSymbol() const { return item_->NextSymbol(); }
    
    /**
     * @brief Devuelve el siguiente ítem LR(1) (avanza el punto).
     */
    std::shared_ptr<LR1Item> NextItem() const;
    
    /**
     * @brief Compara si dos ítems LR(1) son iguales.
     */
    bool operator==(const LR1Item& other) const;
    bool operator==(const std::shared_ptr<LR1Item>& other) const;
    
    /**
     * @brief Devuelve una representación en string del ítem.
     */
    std::string ToString() const;
    
    /**
     * @brief Calcula el hash del ítem LR(1).
     */
    size_t hash() const;
    
    /**
     * @brief Devuelve el ítem LR(0) subyacente.
     */
    std::shared_ptr<Item> getItem() const { return item_; }

private:
    std::shared_ptr<Item> item_;  // El ítem LR(0) base
    std::string lookahead_;       // Token de anticipación
};

// Definición del hash para ser utilizado en contenedores como unordered_set/map
namespace std {
    template <>
    struct hash<LR1Item> {
        size_t operator()(const LR1Item& item) const {
            return item.hash();
        }
    };
    
    template <>
    struct hash<std::shared_ptr<LR1Item>> {
        size_t operator()(const std::shared_ptr<LR1Item>& item) const {
            return item->hash();
        }
    };
}

/**
 * @class LR1ItemSet
 * @brief Conjunto de ítems LR(1).
 *
 * Representa un estado en el autómata LR(1). Se utiliza para construir las tablas de análisis.
 */
class LR1ItemSet {
public:
    /**
     * @brief Constructor por defecto.
     */
    LR1ItemSet() = default;
    
    /**
     * @brief Constructor a partir de un conjunto inicial de ítems.
     * @param items Vector de ítems LR(1).
     */
    LR1ItemSet(const std::vector<std::shared_ptr<LR1Item>>& items);
    
    /**
     * @brief Añade un ítem al conjunto.
     * @param item Ítem a añadir.
     * @return true si el ítem se añadió, false si ya estaba presente.
     */
    bool Add(const std::shared_ptr<LR1Item>& item);
    
    /**
     * @brief Añade todos los ítems de otro conjunto.
     * @param other Conjunto de ítems a añadir.
     * @return true si algún ítem se añadió, false si todos ya estaban presentes.
     */
    bool AddAll(const LR1ItemSet& other);
    
    /**
     * @brief Comprueba si un ítem está en el conjunto.
     * @param item Ítem a comprobar.
     * @return true si el ítem está presente, false en caso contrario.
     */
    bool Contains(const std::shared_ptr<LR1Item>& item) const;
    
    /**
     * @brief Compara si dos conjuntos son iguales.
     */
    bool operator==(const LR1ItemSet& other) const;
    
    /**
     * @brief Devuelve una representación en string del conjunto.
     */
    std::string ToString() const;
    
    /**
     * @brief Devuelve un iterador al inicio del conjunto.
     */
    std::vector<std::shared_ptr<LR1Item>>::const_iterator begin() const { return items_.begin(); }
    
    /**
     * @brief Devuelve un iterador al final del conjunto.
     */
    std::vector<std::shared_ptr<LR1Item>>::const_iterator end() const { return items_.end(); }
    
    /**
     * @brief Devuelve el número de ítems en el conjunto.
     */
    size_t size() const { return items_.size(); }
    
    /**
     * @brief Calcula el hash del conjunto de ítems.
     */
    size_t hash() const;
    
    /**
     * @brief Devuelve los ítems del conjunto.
     */
    const std::vector<std::shared_ptr<LR1Item>>& items() const { return items_; }

private:
    std::vector<std::shared_ptr<LR1Item>> items_;
};

// Definición del hash para LR1ItemSet
namespace std {
    template <>
    struct hash<LR1ItemSet> {
        size_t operator()(const LR1ItemSet& itemset) const {
            return itemset.hash();
        }
    };
}
