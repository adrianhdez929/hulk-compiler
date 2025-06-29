#include "Item.h"
#include <sstream>

Item::Item(std::shared_ptr<Production> production, int pos, ContainerSet<string> lookaheads)
    : production_(production), pos_(pos), lookaheads_(lookaheads) {}
    //        ContainerSet<string> lookaheads)
    // : production_(production), pos_(pos), lookaheads_(lookaheads) {}

bool Item::IsReduceItem() const {
    return pos_ >= production_->Right().Symbols().size();
}

std::shared_ptr<Symbol> Item::NextSymbol() const {
    const auto& symbols = production_->Right().Symbols();
    if (pos_ < symbols.size()) {
        return symbols[pos_];
    }
    return nullptr;
}

std::shared_ptr<Item> Item::NextItem() const {
    if (!IsReduceItem()) {
        return std::make_shared<Item>(production_, pos_ + 1, lookaheads_);
    }
    return nullptr;
}

std::vector<std::vector<string>> Item::Preview(int skip) const {
    std::vector<std::vector<string>> result;
    const auto& symbols = production_->Right().Symbols();
    
    if (pos_ + skip >= symbols.size()) {
        for (const auto& lookahead : lookaheads_) {
            result.push_back({lookahead});
        }
    } else {
        std::vector<string> sequence;
        for (int i = pos_ + skip; i < symbols.size(); i++) {
            sequence.push_back(symbols[i]->Name());
        }
        
        for (const auto& lookahead : lookaheads_) {
            auto seq_copy = sequence;
            seq_copy.push_back(lookahead);
            result.push_back(seq_copy);
        }
    }
    return result;
}

std::shared_ptr<Item> Item::Center() const {
    return std::make_shared<Item>(production_, pos_, lookaheads_);
}

bool Item::operator==(std::shared_ptr<Item> other) const {
    return production_ == other->production_ && 
           pos_ == other->pos_ && 
           lookaheads_ == other->lookaheads_;
}

size_t Item::hash() const {
    size_t seed = 0;
    seed ^= std::hash<void*>{}(production_.get()) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= std::hash<int>{}(pos_) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    for (const auto& la : lookaheads_) {
        seed ^= std::hash<std::string>{}(la) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}

std::string Item::ToString() const {
    std::stringstream ss;
    ss << production_->Left()->Name() << " -> ";
    
    const auto& right = production_->Right().Symbols();
    for (int i = 0; i < right.size(); i++) {
        if (i == pos_) ss << "•";
        ss << right[i]->Name() << " ";
    }
    if (pos_ == right.size()) ss << "•";
    
    ss << ", [";
    for (const auto& la : lookaheads_) {
        ss << la << " ";
    }
    ss << "]";
    return ss.str();
}