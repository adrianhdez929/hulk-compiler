#include "dfa.h"

DFA::DFA(int states,
         const set<State>& finalStates,
         const Transitions& transitions,
         State initialState)
    : NFA(states, finalStates, transitions, initialState) {
        
        //verificar que cada transicion lleve a un solo estado y q no tenga epsilon-transiciones
        for (const auto& [key, destinations] : transitions) {
            if (destinations.size() > 1) {
                throw invalid_argument("Each transition must have a single destination state.");
            }
            if (key.second.empty()) {
                throw invalid_argument("DFA cannot have epsilon transitions.");
            }
        }

        this->current_ = initialState;
    }

void DFA::reset() {
    current_ = startState();
}

void DFA::move(Symbol symbol) {
    if (current_ == -1) return; 
    try
    {
        current_ = this->getTransitions().at(current_).at(symbol)[0];
    } 
    catch (const std::out_of_range&) 
    {
        current_ = -1;
    }
}

bool DFA::recognize(const string& input) {
    reset();

    for (char c : input) {
        //Convertir char a string
        string symbol(1, c);
        move(symbol);
        if (current_ == -1) {
            break; // No hay transición válida
        }
    }
    return finalStates().count(current_) > 0;
}