#include <string>
#include <stdexcept>
#include "nfa.h"

using namespace std;


/**
 * @class DFA
 * @brief Representa un autómata finito determinista (DFA), derivado de NFA.
 * 
 * Se utiliza para el análisis léxico y la conversión de NFA a DFA.
 */

#ifndef DFA_H
#define DFA_H

class DFA : public NFA {
    public:

        // using DFA_Transitions = map<pair<State, Symbol>, State>;
        DFA(int states,
            const set<State>& finalStates,
            const Transitions& transitions,
            State initialState);

        void reset();
        bool recognize(const string& input);
        NFA::State getTransition(State state, Symbol symbol) const;

    private:
        State current_;

        void move(Symbol symbol);
        void validate_transitions(Transitions& transitions) const;

};
#endif // DFA_H