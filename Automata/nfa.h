#include <vector>
#include <map>
#include <set>
#include <string>
#include <stdexcept>

using namespace std;


#ifndef NFA_H
#define NFA_H

class NFA {
public:
    // Constructor
    
    using State = int;
    using Symbol = string;
    // using Transition = map<pair<State, Symbol>, State>;
    //Transiciones = (0, 'a') : [1,2,...] desde el estado 0 con simbolo 'a' puedo ir a los estados 1, 2,...
    using Transitions = map<pair<State, Symbol>, vector<State>>;

    NFA(int states,
        const set<State>& finalStates,
        const Transitions& transitions,
        State initialState);

    vector<State> epsilon_transition(State state) const;
    const set<Symbol>& getVocab() const;

    // Getters
    int states() const;
    State startState() const;
    const set<State>& finalStates() const;
    const map<State, map<Symbol, vector<State>>>& getTransitions() const;
    const Transitions& getTransitionsMap() const;

private:
    int states_;
    State startState_;
    set<State> finalStates_;
    Transitions transitionsMap_;
    map<State, map<Symbol, vector<State>>> transitions_;
    set<Symbol> vocab_;

    void build_transitions();
    void validate_state(State state) const;
};

#endif // NFA_H

