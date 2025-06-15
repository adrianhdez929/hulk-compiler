#pragma once
#include <memory>
#include <vector>
#include "../Grammar/grammar.hpp"
#include "../Automata/state.hpp"

class SLR1Parser {
public:
    static constexpr const char* SHIFT = "SHIFT";
    static constexpr const char* REDUCE = "REDUCE";
    static constexpr const char* OK = "OK";

    SLR1Parser(Grammar& G, bool verbose = false);
    virtual ~SLR1Parser();

    std::pair<std::vector<int>, std::vector<std::string>> 
    Parse(const std::vector<Terminal>& tokens);
    std::pair<std::vector<int>, std::vector<std::string>>
    Parse(const std::vector<std::string>& tokens);

    void BuildParsingTable();

    map<Sentence, ContainerSet<string>> compute_firsts();
    ContainerSet<string> compute_local_firsts(const Sentence& sentence, const map<Sentence, ContainerSet<string>>& symbol_firsts);
    map<Sentence, ContainerSet<string>> compute_follows(const map<Sentence, ContainerSet<string>>& symbol_firsts);

    State BuildLR0Automaton();

    void Register(std::map<std::pair<int, Symbol>, std::pair<std::string, int>>& table, 
                  const std::pair<int, Symbol>& key, 
                  const std::pair<std::string, int>& value);
    void Register(std::map<std::pair<int, Symbol>, int>& table,
                  const std::pair<int, Symbol>& key, 
                  int value);
    void CleanupAutomatonStates();

private:
    Grammar& G_;
    bool verbose_;
    std::map<std::pair<int, Symbol>, std::pair<std::string, int>> action_;
    std::map<std::pair<int, Symbol>, int> goto_;
    std::vector<State*> automaton_states_;
};
