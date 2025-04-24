
#include "../dfa.h"
#include "../nfa.h"
#include "../utils/ContainerSet.h"

// namespace manipulation {
//The move function computes the set of states that can be reached from the given set of states with the given symbol.
set<NFA::State> move(const NFA& automaton, const unordered_set<NFA::State>& states, NFA::Symbol symbol);
set<NFA::State> move(const NFA& automaton, const NFA::State& state, const NFA::Symbol& symbol);
ContainerSet epsilon_closure(const NFA& automaton, const set<NFA::State>& states);
struct DFAStates {
    ContainerSet states;
    int id;
    bool is_final;

    DFAStates(const ContainerSet& states, int id, bool is_final)
        : states(states), id(id), is_final(is_final) {}
};
DFA nfa_to_dfa(const NFA& automaton);

bool nfa_recognize(const NFA& automaton, const string& input);

// }