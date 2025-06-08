#include "operations.h"
#include "../nfa.h"
#include <map>
#include <set>
#include <vector>
#include <string>

using namespace std;

NFA union_nfa(const NFA& a1, const NFA& a2) {
    NFA::Transitions new_transitions;
    
    const NFA::State start = 0;
    const NFA::State d1 = 1;
    const NFA::State d2 = a1.states() + d1;
    const NFA::State final_state = a2.states() + d2;
    
    // Relocalizar transiciones de a1
    for (const auto& transition : a1.getTransitionsMap()) {
        const auto& origin_symbol = transition.first;
        const auto& destinations = transition.second;
        
        NFA::State new_origin = origin_symbol.first + d1;
        NFA::Symbol symbol = origin_symbol.second;
        
        vector<NFA::State> new_destinations;
        for (NFA::State dest : destinations) {
            new_destinations.push_back(dest + d1);
        }
        
        new_transitions[{new_origin, symbol}] = new_destinations;
    }
    
    // Relocalizar transiciones de a2
    for (const auto& transition : a2.getTransitionsMap()) {
        const auto& origin_symbol = transition.first;
        const auto& destinations = transition.second;
        
        NFA::State new_origin = origin_symbol.first + d2;
        NFA::Symbol symbol = origin_symbol.second;
        
        vector<NFA::State> new_destinations;
        for (NFA::State dest : destinations) {
            new_destinations.push_back(dest + d2);
        }
        
        new_transitions[{new_origin, symbol}] = new_destinations;
    }
    
    // Añadir transiciones epsilon iniciales
    new_transitions[{start, ""}] = {d1, d2};
    
    // Añadir transiciones a estado final desde a1
    for (NFA::State state : a1.finalStates()) {
        NFA::State adjusted_state = state + d1;
        new_transitions[{adjusted_state, ""}].push_back(final_state);
    }
    
    // Añadir transiciones a estado final desde a2
    for (NFA::State state : a2.finalStates()) {
        NFA::State adjusted_state = state + d2;
        new_transitions[{adjusted_state, ""}].push_back(final_state);
    }
    
    // Calcular estados totales
    const int total_states = a1.states() + a2.states() + 2;
    set<NFA::State> finals = {final_state};
    
    return NFA(total_states, finals, new_transitions, start);
}

using namespace std;

NFA concat_nfa(const NFA& a1, const NFA& a2) {
    NFA::Transitions new_transitions;

    const NFA::State start = 0;
    const NFA::State d1 = 0;  // No offset para a1
    const NFA::State d2 = a1.states();
    const NFA::State final_state = d2 + a2.states();

    // Copiar transiciones de a1 sin modificar
    for (const auto& transition : a1.getTransitionsMap()) {
        new_transitions[transition.first] = transition.second;
    }

    // Relocalizar transiciones de a2
    for (const auto& transition : a2.getTransitionsMap()) {
        const auto& [origin_symbol, destinations] = transition;
        NFA::State new_origin = origin_symbol.first + d2;
        NFA::Symbol symbol = origin_symbol.second;

        vector<NFA::State> new_destinations;
        for (NFA::State dest : destinations) {
            new_destinations.push_back(dest + d2);
        }
        
        new_transitions[{new_origin, symbol}] = new_destinations;
    }

    // Conectar finales de a1 con inicio de a2
    NFA::State a2_start = a2.startState() + d2;
    for (NFA::State state : a1.finalStates()) {
        new_transitions[{state, ""}].push_back(a2_start);
    }

    // Conectar finales de a2 con nuevo estado final
    for (NFA::State state : a2.finalStates()) {
        NFA::State adjusted_state = state + d2;
        new_transitions[{adjusted_state, ""}].push_back(final_state);
    }

    // Calcular estados totales y finales
    const int total_states = a1.states() + a2.states() + 1;
    set<NFA::State> finals = {final_state};

    return NFA(total_states, finals, new_transitions, start);
}

NFA closure_nfa(const NFA& a1) {
    NFA::Transitions new_transitions;

    const NFA::State start = 0;
    const NFA::State d1 = 1;
    const NFA::State final_state = a1.states() + d1;

    // Relocalizar transiciones de a1
    for (const auto& transition : a1.getTransitionsMap()) {
        const auto& [origin_symbol, destinations] = transition;
        NFA::State new_origin = origin_symbol.first + d1;
        NFA::Symbol symbol = origin_symbol.second;

        vector<NFA::State> new_destinations;
        for (NFA::State dest : destinations) {
            new_destinations.push_back(dest + d1);
        }
        
        new_transitions[{new_origin, symbol}] = new_destinations;
    }

    // Transiciones desde nuevo estado inicial
    NFA::State a1_start = a1.startState() + d1;
    new_transitions[{start, ""}] = {a1_start, final_state};

    // Conectar finales de a1 con estado final y inicio
    for (NFA::State state : a1.finalStates()) {
        NFA::State adjusted_state = state + d1;
        new_transitions[{adjusted_state, ""}].push_back(final_state);
        new_transitions[{adjusted_state, ""}].push_back(a1_start);
    }

    // Calcular estados totales y finales
    const int total_states = a1.states() + 2;
    set<NFA::State> finals = {start, final_state};

    return NFA(total_states, finals, new_transitions, start);
}

// Función auxiliar para distinguir estados
vector<vector<DFA::State>> distinguish_states(
    const vector<DFA::State>& group, 
    const DFA& automaton, 
    const DisjointSet<DFA::State>& partition) {
    
    map<vector<DFA::State>, vector<DFA::State>> split;
    set<DFA::Symbol> vocabulary = automaton.getVocab();

    for (DFA::State state : group) {
        vector<DFA::State> destinations;
        for (const auto& symbol : vocabulary) {
            DFA::State dest = automaton.getTransition(state, symbol);
            if (dest == -1) {//ATENCION PARCHE. REVISAR SI OCURRE ERROR##############################################################################################
                continue; // Si no hay transición, omitimos
            }
            DFA::State representative = partition.find(dest)->representative()->data;
            destinations.push_back(representative);
        }
        
        split[destinations].push_back(state);
    }

    vector<vector<DFA::State>> result;
    for (const auto& entry : split) {
        result.push_back(entry.second);
    }
    return result;
}

// Minimización de estados
DisjointSet<DFA::State> state_minimization(const DFA& automaton) {
    DisjointSet<DFA::State> partition;
    for (DFA::State s = 0; s < automaton.states(); ++s) {
        partition.addItem(s);
    }

    // Partición inicial: finales vs no finales
    set<DFA::State> finals(automaton.finalStates().begin(), automaton.finalStates().end());
    vector<DFA::State> non_finals;
    for (DFA::State s = 0; s < automaton.states(); ++s) {
        if (finals.find(s) == finals.end()) {
            non_finals.push_back(s);
        }
    }

    partition.merge(vector<DFA::State>(finals.begin(), finals.end()));
    partition.merge(non_finals);

    // Refinar partición
    while (true) {
        DisjointSet<DFA::State> new_partition;
        for (DFA::State s = 0; s < automaton.states(); ++s) {
            new_partition.addItem(s);
        }

        auto groups = partition.getGroups();
        for (const auto& group_entry : groups) {
            const auto& group = group_entry.second;
            auto new_groups = distinguish_states(group, automaton, partition);
            for (const auto& new_group : new_groups) {
                new_partition.merge(new_group);
            }
        }

        if (new_partition.size() == partition.size()) break;
        partition = std::move(new_partition);
    }

    return partition;
}

// Construcción del DFA minimizado
DFA automata_minimization(const DFA& automaton) {
    auto partition = state_minimization(automaton);
    auto groups = partition.getGroups();

    // Mapeo de estados antiguos a nuevos
    map<DFA::State, DFA::State> state_map;
    vector<DFA::State> new_states;
    DFA::State new_state_id = 0;
    for (const auto& group : groups) {
        for (DFA::State s : group.second) {
            state_map[s] = new_state_id;
        }
        new_states.push_back(new_state_id++);
    }

    // Construir transiciones
    map<pair<DFA::State, DFA::Symbol>, vector<NFA::State>> new_transitions;
    for (const auto& group : groups) {
        DFA::State representative = group.first;
        for (const auto& symbol : automaton.getVocab()) {
            DFA::State old_dest = automaton.getTransition(representative, symbol);
            if (old_dest == -1) continue; // Si no hay transición, omitimos //ATENCION PARCHE. REVISAR SI OCURRE ERROR##############################################################################
            new_transitions[{state_map[representative], symbol}].push_back(state_map[partition.find(old_dest)->representative()->data]); //[old_dest].findRepresentative()->value];
        }
    }

    // Calcular estados finales
    set<DFA::State> new_finals;
    for (const auto& group : groups) {
        for (DFA::State s : group.second) {
            if (automaton.finalStates().find(s) != automaton.finalStates().end()) {
                new_finals.insert(state_map[group.first]);
                break; // Solo necesitamos agregar una vez el estado final
            }
        }
    }

    // Estado inicial
    DFA::State old_start = automaton.startState();
    DFA::State new_start = state_map[partition.find(old_start)->representative()->data];

    return DFA(new_states.size(), new_finals, new_transitions, new_start);
}