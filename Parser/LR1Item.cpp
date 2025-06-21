#include "LR1Item.h"
#include <sstream>
#include <algorithm>
#include <numeric>

LR1Item::LR1Item(std::shared_ptr<Production> production, int pos, const std::string& lookahead)
    : item_(std::make_shared<Item>(production, pos)), lookahead_(lookahead) {}

LR1Item::LR1Item(const AttrProd& production, int pos, const std::string& lookahead)
    : item_(std::make_shared<Item>(std::make_shared<Production>(production), pos)), lookahead_(lookahead) {}

LR1Item::LR1Item(const std::shared_ptr<Item>& item, const std::string& lookahead)
    : item_(item), lookahead_(lookahead) {}

std::shared_ptr<LR1Item> LR1Item::NextItem() const {
    if (!IsReduceItem()) {
        return std::make_shared<LR1Item>(production(), pos() + 1, lookahead_);
    }
    return nullptr;
}

bool LR1Item::operator==(const LR1Item& other) const {
    // Dos ítems LR(1) son iguales si tienen la misma producción, posición y lookahead
    return *item_ == other.item_ && lookahead_ == other.lookahead_;
}

bool LR1Item::operator==(const std::shared_ptr<LR1Item>& other) const {
    return *this == *other;
}

std::string LR1Item::ToString() const {
    std::stringstream ss;
    
    // Parte izquierda de la producción
    ss << production()->Left()->Name() << " -> ";
    
    // Parte derecha con el punto en la posición correcta
    const auto& symbols = production()->Right().Symbols();
    for (int i = 0; i < symbols.size(); ++i) {
        if (i == pos()) {
            ss << "• ";
        }
        ss << symbols[i]->Name() << " ";
    }
    
    // Si el punto está al final
    if (pos() == symbols.size()) {
        ss << "•";
    }
    
    // Añadir el lookahead
    ss << ", " << lookahead_;
    
    return ss.str();
}

size_t LR1Item::hash() const {
    // Combinar el hash del ítem base con el hash del lookahead
    size_t h1 = item_->hash();
    size_t h2 = std::hash<std::string>()(lookahead_);
    return h1 ^ (h2 << 1);
}

// Implementación de LR1ItemSet
LR1ItemSet::LR1ItemSet(const std::vector<std::shared_ptr<LR1Item>>& items) {
    for (const auto& item : items) {
        Add(item);
    }
}

bool LR1ItemSet::Add(const std::shared_ptr<LR1Item>& item) {
    // Verificar si el ítem ya existe en el conjunto
    for (const auto& existingItem : items_) {
        if (*existingItem == *item) {
            return false; // El ítem ya está en el conjunto
        }
    }
    
    // Añadir el ítem al conjunto
    items_.push_back(item);
    return true;
}

bool LR1ItemSet::AddAll(const LR1ItemSet& other) {
    bool changed = false;
    for (const auto& item : other.items_) {
        if (Add(item)) {
            changed = true;
        }
    }
    return changed;
}

bool LR1ItemSet::Contains(const std::shared_ptr<LR1Item>& item) const {
    return std::find_if(items_.begin(), items_.end(),
                       [&item](const std::shared_ptr<LR1Item>& i) { return *i == *item; }) != items_.end();
}

bool LR1ItemSet::operator==(const LR1ItemSet& other) const {
    if (items_.size() != other.items_.size()) {
        return false;
    }
    
    // Verificar que cada ítem de un conjunto está en el otro
    for (const auto& item : items_) {
        if (!other.Contains(item)) {
            return false;
        }
    }
    
    return true;
}

std::string LR1ItemSet::ToString() const {
    std::stringstream ss;
    ss << "{\n";
    for (const auto& item : items_) {
        ss << "  " << item->ToString() << "\n";
    }
    ss << "}";
    return ss.str();
}

size_t LR1ItemSet::hash() const {
    // Calcular el hash combinando los hashes de todos los ítems
    return std::accumulate(items_.begin(), items_.end(), size_t(0),
                          [](size_t acc, const std::shared_ptr<LR1Item>& item) {
                              return acc ^ (item->hash() << 1);
                          });
}
