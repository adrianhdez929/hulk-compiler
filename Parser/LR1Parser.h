#pragma once
#include <cassert>
#include "../Grammar/grammar.h"
#include "Item.h"
#include "../Automata/utils/ContainerSet.h"
#include "../Automata/state.h"
#include <map>
#include <queue>

class ShiftReduceParser {
public:
    static constexpr const char* SHIFT = "SHIFT";
    static constexpr const char* REDUCE = "REDUCE";
    static constexpr const char* OK = "OK";
    
    ShiftReduceParser(Grammar& G, bool verbose = false);
    virtual ~ShiftReduceParser() = default;
    
    virtual void BuildParsingTable() = 0;
    std::pair<std::vector<std::shared_ptr<Production>>, std::vector<std::string>> 
    Parse(const std::vector<std::shared_ptr<Terminal>>& tokens);
    
protected:
    Grammar& G_;
    bool verbose_;
    std::map<std::pair<int, std::shared_ptr<Terminal>>, std::pair<std::string, int>> action_;
    std::map<std::pair<int, std::shared_ptr<NonTerminal>>, int> goto_;
    
    void Register(std::map<std::pair<int, std::shared_ptr<Terminal>>, std::pair<std::string, int>>& table, 
                 const std::pair<int, std::shared_ptr<Terminal>>& key, 
                 const std::pair<std::string, int>& value);
    
    void Register(std::map<std::pair<int, std::shared_ptr<NonTerminal>>, int>& table, 
                 const std::pair<int, std::shared_ptr<NonTerminal>>& key, 
                 int value);
};

class LR1Parser : public ShiftReduceParser {
public:
    using ShiftReduceParser::ShiftReduceParser;
    void BuildParsingTable() override;

private:
    
    pair<map<shared_ptr<Symbol>, ContainerSet<shared_ptr<Symbol>>>, map<Sentence, ContainerSet<shared_ptr<Symbol>>>> compute_firsts();
    ContainerSet<shared_ptr<Symbol>> compute_local_firsts(const Sentence& sentence, const map<shared_ptr<Symbol>, ContainerSet<shared_ptr<Symbol>>>& symbol_firsts);

    std::map<std::shared_ptr<Symbol>, ContainerSet<shared_ptr<Symbol>>> compute_follows(
        const std::map<std::shared_ptr<Symbol>, ContainerSet<shared_ptr<Symbol>>>& symbol_firsts);

    std::vector<Item> expand(
        const Item& item,
        const std::map<std::shared_ptr<Symbol>, ContainerSet<shared_ptr<Symbol>>>& symbol_firsts);
    
    std::set<Item> compress(const std::vector<Item>& items);
    
    std::vector<Item> closure_lr1(
        const std::vector<Item>& items,
        const std::map<std::shared_ptr<Symbol>, ContainerSet<shared_ptr<Symbol>>>& firsts);

    std::vector<Item> goto_lr1(
        const std::vector<Item>& items,
        std::shared_ptr<Symbol> symbol,
        const std::map<std::shared_ptr<Symbol>, ContainerSet<shared_ptr<Symbol>>>& firsts,
        bool just_kernel = false);

    State BuildLR1Automaton();
};