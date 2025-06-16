#include <vector>
#include <map>
#include <set>
#include <string>
#include <stdexcept>

using namespace std;


/**
 * @class NFA
 * @brief Representa un autómata finito no determinista (NFA).
 * 
 * Permite construir y manipular autómatas NFA para expresiones regulares y análisis léxico.
 */

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

    // Constructor de un automata no determinista
    // Recibe la cantidad de estados, el conjunto de estados finales, el conjunto de transiciones y el estado inicial
    // states: cantidad de estados del automata
    // finalStates: conjunto de estados finales del automata
    // transitions: conjunto de transiciones del automata
    // initialState: estado inicial del automata
    NFA(int states,
        const set<State>& finalStates,
        const Transitions& transitions,
        State initialState);
    ~NFA();

    vector<State> epsilon_transitions(State state) const;
    const set<Symbol>& getVocab() const;

    // Cantidad de estados del automata
    int states() const;
    // Estado inicial del automata
    State startState() const;
    // Conjunto de estados finales del automata. Tipo State = int
    const set<State>& finalStates() const;
    // Conjunto de transiciones del automata. Devuelve: <State, <Symbol, vector<State>>> Se indexa por un estado para saber que transiciones tiene con que simbolos hace que conjunto de estados.
    const map<State, map<Symbol, vector<State>>>& getTransitions() const;
    // Conjunto de transiciones del automata. Transitions = map<pair<State, Symbol>, vector<State>> Se indexa por un par <State, Symbol> para saber el conjunto de estados a los que se puede llegar desde el State con el Symbol.
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

