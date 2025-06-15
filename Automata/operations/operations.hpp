#include "../nfa.hpp"
#include "../dfa.hpp"
#include "../utils/Disjoint_set.hpp"

// Operaciones de union, concatenacion y clausura
// union
NFA union_nfa(const NFA& a1, const NFA& a2);
// DFA union_dfa(const DFA& dfa1, const DFA& dfa2);
// concatenacion
NFA concat_nfa(const NFA& a1, const NFA& a2);
// DFA concat_dfa(const DFA& dfa1, const DFA& dfa2);
// clausura
NFA closure_nfa(const NFA& nfa);
// DFA closure_dfa(const DFA& dfa);

vector<vector<DFA::State>> distinguish_states(
    const vector<DFA::State>& group, 
    const DFA& automaton, 
    const DisjointSet<DFA::State>& partition);

DisjointSet<DFA::State> state_minimization(const DFA& automaton);

DFA automata_minimization(const DFA& automaton);