#pragma once
#include <memory>
#include <set>
#include "../Grammar/production.h"

class Item {
public:
    Item(std::shared_ptr<Production> production_, int pos_, 
        std::set<std::shared_ptr<Terminal>> lookahead_ = {});

    std::shared_ptr<Production> production() const { return production_; }
    int pos() const { return pos_; }
    const std::set<std::shared_ptr<Terminal>>& lookaheads() const { return lookaheads_; }

    bool IsReduceItem() const;
    std::shared_ptr<Symbol> NextSymbol() const;
    std::shared_ptr<Item> NextItem() const;
    std::vector<std::vector<std::shared_ptr<Symbol>>> Preview(int skip = 1) const;
    std::shared_ptr<Item> Center() const;

    bool operator==(const Item& other) const;
    size_t hash() const;
    std::string ToString() const;


private:
    std::shared_ptr<Production> production_;
    int pos_;
    std::set<std::shared_ptr<Terminal>> lookaheads_;
};
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