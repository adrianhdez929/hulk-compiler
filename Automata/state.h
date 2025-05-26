#include <map>
#include <set>
#include <vector>
#include <string>
#include <memory>
#include <unordered_set>
#include "nfa.h"

#ifndef STATE_H
#define STATE_H

class State {
public:
    using Symbol = std::string;
    
    // Constructor
    State(int id, bool is_final = false);
    ~State();
    
    // Getters
    int id() const;
    bool is_final() const;
    //tag setter
    void set_tag(const std::string& tag) { tag_ = tag; }
    // tag getter
    string tag() const;

    // Cálculo de epsilon-clausura
    std::unordered_set<const State*> epsilon_closure() const;
    std::unordered_set<const State*> epsilon_closure(const std::unordered_set<const State*>& states) const;

    // Movimientos
    std::unordered_set<const State*> move(const Symbol& symbol) const;
    
    // Transiciones
    void add_transition(const Symbol& symbol, State* state);
    void add_epsilon_transition(State* state);
    bool has_transition(const Symbol& symbol) const;
    
    // Acceso a transiciones
    const std::map<Symbol, std::vector<State*>>& transitions() const;
    const std::set<State*>& epsilon_transitions() const;
    
    // Conversión desde NFA
    static State* from_nfa(const class NFA& nfa);

    // Conversion a determinista
    static State* to_deterministic(State* nfa_start);

    // Saber si reconoce una cadena
    bool recognizes(const std::vector<Symbol>& input) const;

    // ToString
    std::string ToString() const;

private:
    int id_;
    bool is_final_;
    std::string tag_;
    std::map<Symbol, std::vector<State*>> transitions_;
    std::set<State*> epsilon_transitions_;
};

#endif