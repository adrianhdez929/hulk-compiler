#include "../dfa.hpp"
#include "../nfa.hpp"
#include "ContainerSet.hpp"
#include <algorithm>
#include <map>
#include "aut_manipulation.hpp"
#include <queue>

// namespace manipulation {
//The move function computes the set of states that can be reached from the given set of states with the given symbol.
set<NFA::State> move(const NFA& automaton, const unordered_set<NFA::State>& states, NFA::Symbol symbol) {
    set<NFA::State> moves;
    for (const auto& state : states) {
        auto& next_states = automaton.getTransitions().at(state);
        auto it = next_states.find(symbol);
        if (it != next_states.end()) {
            const auto& reachable_states = it->second;
            moves.insert(reachable_states.begin(), reachable_states.end());
        }
    }
    return moves;
}
set<NFA::State> move(const NFA& automaton, const NFA::State& state, const NFA::Symbol& symbol) {
    set<NFA::State> moves;

    auto& next_states = automaton.getTransitions().at(state);
    auto it = next_states.find(symbol);
    if (it != next_states.end()) {
        const auto& reachable_states = it->second;
        moves.insert(reachable_states.begin(), reachable_states.end());
    }
    return moves;
}

ContainerSet<NFA::State> epsilon_closure(const NFA& automaton, const set<NFA::State>& states) {
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
    vector<DFAStates> dfa_states;
    int dfa_state_id = 0;

    // Crear el estado inicial del DFA
    ContainerSet<NFA::State> initial_states = epsilon_closure(automaton, {automaton.startState()});
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
    std::queue<DFAStates> pending_states;
    pending_states.push(dfa_states[0]);
    // vector<DFAStates> pending_states = {dfa_states[0]};
    while (!pending_states.empty()) {
        DFAStates current_dfa_state = pending_states.front();
        pending_states.pop();

        // Iterar sobre el vocabulario del NFA
        for (const auto& symbol : automaton.getVocab()) {
            if (symbol.empty()) {
                continue; // Ignorar transiciones epsilon
            }
            // Obtener los estados alcanzables desde el conjunto de estados del DFA actual
            set<NFA::State> next_states = move(automaton, current_dfa_state.states.get_set(), symbol);
            ContainerSet<NFA::State> next_closure = epsilon_closure(automaton, next_states);

            if (next_closure.empty()) {
                continue; // No hay transiciones para este símbolo
            }

            // Verificar si el conjunto de estados ya existe en el DFA
            auto it = find_if(dfa_states.begin(), dfa_states.end(),
                [&next_closure](const DFAStates& state) { return state.states == next_closure; });

            if (it == dfa_states.end()) {
                // Si no existe, crear un nuevo estado en el DFA
                bool is_final = false;
                for (const auto& final_state : automaton.finalStates()) {
                    if (next_closure.contains(final_state)) {
                        is_final = true;
                        break;
                    }
                }
                DFAStates new_state = DFAStates(next_closure, dfa_state_id++, is_final);
                dfa_states.emplace_back(new_state);
                it = dfa_states.end() - 1; // Obtener el iterador del nuevo estado
                pending_states.push(*it); // Agregar el nuevo estado a los pendientes

                // Agregar la transición al mapa de transiciones del DFA
                dfa_transitions[{current_dfa_state.id, std::string(symbol)}] = new_state.id;
            }
            else {
                // Si ya existe, actualizar el estado actual
                DFAStates new_state = *it;
                
                // Agregar la transición al mapa de transiciones del DFA
                dfa_transitions[{current_dfa_state.id, std::string(symbol)}] = new_state.id;
            }

            
        }
    }

    set<int> final_states;
    for (const auto& dfa_state : dfa_states) {
        if (dfa_state.is_final) {
            final_states.insert(dfa_state.id);
        }
    }
    // Crear un conjunto de transiciones para el DFA
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
    ContainerSet<NFA::State> current_states = epsilon_closure(automaton, {automaton.startState()});

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

