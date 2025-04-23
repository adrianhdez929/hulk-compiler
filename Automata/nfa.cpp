#include "nfa.h"

NFA::NFA(int states,
         const set<State>& finalStates,
         const Transitions& transitions,
         State initialState)
    : states_(states), startState_(initialState), finalStates_(finalStates),
      transitionsMap_(transitions) {
    
    if (states <= 0) {
        throw invalid_argument("Number of states must be positive.");
    }
    for (State s = 0; s < states; ++s) {
        transitions_[s] = {};
    }
    build_transitions();
}

void NFA::build_transitions() {
    for (const auto& [key, destinations] : transitionsMap_) {
        State origin = key.first;
        const Symbol& symbol = key.second;

        validate_state(origin);
        for (State dest : destinations) validate_state(dest);
        transitions_[origin][symbol] = destinations;

        if (!symbol.empty()) { 
            vocab_.insert(symbol);
        }
    }
}

vector<NFA::State> NFA::epsilon_transitions(State state) const {
    validate_state(state);
    vector<State> result;
    for (const auto& [symbol, destinations] : transitions_.at(state)) {
        if (symbol == "") { // Epsilon transition
            result.insert(result.end(), destinations.begin(), destinations.end());
        }
    }
    return result;
}

//Getters
int NFA::states() const { return states_; }
NFA::State NFA::startState() const { return startState_; }
const set<NFA::State>& NFA::finalStates() const { return finalStates_; }
const map<NFA::State, map<NFA::Symbol, vector<NFA::State>>>& NFA::getTransitions() const { return transitions_; }
const NFA::Transitions& NFA::getTransitionsMap() const { return transitionsMap_; }
const set<NFA::Symbol>& NFA::getVocab() const { return vocab_; }
void NFA::validate_state(State state) const {
    if (state < 0 || state >= states_) {
        throw out_of_range("State out of range.");
    }
}