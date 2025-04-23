#include "../dfa.h"
#include "../nfa.h"
#include "ContainerSet.h"
#include <algorithm>
#include <map>
#include "aut_manipulation.h"

// namespace manipulation {
//The move function computes the set of states that can be reached from the given set of states with the given symbol.
set<NFA::State> move(const NFA& automaton, const unordered_set<NFA::State>& states, NFA::Symbol symbol) {
    set<NFA::State> moves;
    for (const auto& state : states) {
        try {
            const auto& next_states = automaton.getTransitions().at(state).at(symbol);
            moves.insert(next_states.begin(), next_states.end());
        } catch (const std::out_of_range&) {
            // No transition for this state and symbol
        }
    }
    return moves;
}
set<NFA::State> move(const NFA& automaton, const NFA::State& state, const NFA::Symbol& symbol) {
    set<NFA::State> moves;
    try {
        const auto& next_states = automaton.getTransitions().at(state).at(symbol);
        moves.insert(next_states.begin(), next_states.end());
    } catch (const std::out_of_range&) {
        // No transition for this state and symbol
    }
    return moves;
}

ContainerSet epsilon_closure(const NFA& automaton, const set<NFA::State>& states) {
    vector<NFA::State> pending (states.begin(), states.end());
    unordered_set<NFA::State> closure(states.begin(), states.end());
    while (!pending.empty()) {
        NFA::State state = pending.back();
        pending.pop_back();
    
        for (NFA::State next_state : automaton.epsilon_transitions(state)) {
            if (closure.insert(next_state).second) { // Si se inserta, significa que no estaba en closure
                pending.push_back(next_state);
            }
        }
    }
    return ContainerSet(closure);
}

DFA nfa_to_dfa(const NFA& automaton) {
    // Crear un conjunto de estados para el DFA
    vector<DFAState> dfa_states;
    int dfa_state_id = 0;

    // Crear el estado inicial del DFA
    ContainerSet initial_states = epsilon_closure(automaton, {automaton.startState()});
    bool is_final = false;
    for (const auto& final_state : automaton.finalStates()) {
        if (initial_states.contains(final_state)) {
            is_final = true;
            break;
        }
    }
    dfa_states.emplace_back(initial_states, dfa_state_id++, is_final);

    // Crear un mapa para las transiciones del DFA
    map<pair<int, NFA::Symbol>, int> dfa_transitions;
    // Crear un conjunto de estados pendientes para procesar
    vector<DFAState> pending_states = {dfa_states[0]};
    while (!pending_states.empty()) {
        DFAState current_dfa_state = pending_states.back();
        pending_states.pop_back();

        // Iterar sobre el vocabulario del NFA
        for (const auto& symbol : automaton.getVocab()) {
            // Obtener los estados alcanzables desde el conjunto de estados del DFA actual
            set<NFA::State> next_states = move(automaton, current_dfa_state.states.get_set(), symbol);
            ContainerSet next_closure = epsilon_closure(automaton, next_states);

            if (next_closure.empty()) {
                continue; // No hay transiciones para este símbolo
            }

            // Verificar si el conjunto de estados ya existe en el DFA
            auto it = find_if(dfa_states.begin(), dfa_states.end(),
                [&next_closure](const DFAState& state) { return state.states == next_closure; });

            if (it == dfa_states.end()) {
                // Si no existe, crear un nuevo estado en el DFA
                bool is_final = false;
                for (const auto& final_state : automaton.finalStates()) {
                    if (next_closure.contains(final_state)) {
                        is_final = true;
                        break;
                    }
                }
                dfa_states.emplace_back(next_closure, dfa_state_id++, is_final);
                it = dfa_states.end() - 1; // Obtener el iterador del nuevo estado
                pending_states.push_back(*it); // Agregar el nuevo estado a los pendientes
            }
            else {
                // Si ya existe, actualizar el estado actual
                current_dfa_state = *it;
            }

            // Agregar la transición al mapa de transiciones del DFA
            dfa_transitions[{current_dfa_state.id, std::string(symbol)}] = it->id;
        }
    }

    set<int> final_states;
    for (const auto& dfa_state : dfa_states) {
        if (dfa_state.is_final) {
            final_states.insert(dfa_state.id);
        }
    }
    // Crear el DFA
    map<pair<int, NFA::Symbol>, vector<int>> dfa_transitions_map;
    for (const auto& transition : dfa_transitions) {
        dfa_transitions_map[transition.first].push_back(transition.second);
    }
    // Crear el DFA
    DFA dfa(dfa_states.size(), final_states, dfa_transitions_map, 0);
    return dfa;
}

bool nfa_recognize(const NFA& automaton, const string& input) {
    // Crear un conjunto de estados iniciales
    ContainerSet current_states = epsilon_closure(automaton, {automaton.startState()});

    // Procesar cada símbolo de la cadena de entrada
    for (const auto& symbol : input) {
        // Moverse a los siguientes estados
        set<NFA::State> next_states = move(automaton, current_states.get_set(), string(1, symbol));
        current_states = epsilon_closure(automaton, next_states);
    }

    // Verificar si alguno de los estados finales está en el conjunto actual
    for (const auto& final_state : automaton.finalStates()) {
        if (current_states.contains(final_state)) {
            return true; // La cadena es aceptada
        }
    }
    return false; // La cadena no es aceptada
}
// }
