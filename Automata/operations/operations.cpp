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