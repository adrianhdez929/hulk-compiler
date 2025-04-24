
#include <string>
#include <stdexcept>
#include "nfa.h"

using namespace std;


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

    private:
        State current_;

        void move(Symbol symbol);
        void validate_transitions(Transitions& transitions) const;

};
#endif // DFA_H