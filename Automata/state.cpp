#include "state.h"
#include "nfa.h"
#include <vector>
#include <memory>
#include <stack>
#include <iostream> // Para debug

State::State(int id, bool is_final) : id_(id), is_final_(is_final) {}

State::~State() {
    // El destructor no debería eliminar recursivamente todos los estados
    // ya que esto puede causar problemas con ciclos y doble liberación
    // Simplemente limpiamos nuestros propios datos
    transitions_.clear();
    epsilon_transitions_.clear();
    items_.clear();
    
    // No eliminamos otros estados aquí - esto debería ser manejado por
    // el dueño del autómata completo
}

int State::id() const { return id_; }
bool State::is_final() const { return is_final_; }
std::string State::tag() const { return tag_; }

void State::add_transition(const Symbol& symbol, State* state) {
    transitions_[symbol].push_back(state);
}

bool State::has_transition(const Symbol& symbol) const {
    return transitions_.find(symbol) != transitions_.end();
}

void State::add_epsilon_transition(State* state) {
    epsilon_transitions_.insert(state);
}

const std::map<State::Symbol, std::vector<State*>>& State::transitions() const {
    return transitions_;
}

const std::set<State*>& State::epsilon_transitions() const {
    return epsilon_transitions_;
}

// Cálculo de epsilon-clausura
std::unordered_set<const State*> State::epsilon_closure() const {
    std::unordered_set<const State*> closure;
    std::stack<const State*> stack;
    
    stack.push(const_cast<State*>(this));
    
    while (!stack.empty()) {
        const State* current = stack.top();
        stack.pop();
        
        if (closure.insert(current).second) { // Si no estaba ya en closure
            for (const State* epsilon_state : current->epsilon_transitions_) {
                stack.push(epsilon_state);
            }
        }
    }
    
    return closure;
}
std::unordered_set<const State*> State::epsilon_closure(const std::unordered_set<const State*>& states) const {
    std::unordered_set<const State*> closure;
    std::stack<const State*> stack;

    for (const State* state : states) {
        stack.push(state);
    }

    while (!stack.empty()) {
        const State* current = stack.top();
        stack.pop();

        if (closure.insert(current).second) {
            for (const State* epsilon_state : current->epsilon_transitions()) {
                stack.push(epsilon_state);
            }
        }
    }

    return closure;
}

std::unordered_set<const State*> State::move(const Symbol& symbol) const {
    std::unordered_set<const State*> result;
    
    auto it = transitions_.find(symbol);
    if (it != transitions_.end()) {
        for (const State* state : it->second) {
            result.insert(state);
        }
    }
    
    return result;
}

bool State::recognizes(const std::vector<State::Symbol>& input) const {
    std::unordered_set<const State*> current_states = epsilon_closure();
    
    for (const auto& symbol : input) {
        std::unordered_set<const State*> next_states;
        
        // Mover a los siguientes estados según el símbolo actual
        for (const auto* state : current_states) {
            auto moved_states = state->move(symbol);
            next_states.insert(moved_states.begin(), moved_states.end());
        }
        
        // Calcular la epsilon clausura de los siguientes estados
        current_states = State::epsilon_closure(next_states);
        // en el peor de los casos, epsilon_closure retorna los mismos next_states.
        // Siempre retorna next_states o un conjunto con los next_states y mas estados.
        if (current_states.empty()) break;
    }
    
    // Verificar si alguno de los estados finales está en current_states
    for (const auto* state : current_states) {
        if (state->is_final()) {
            return true;
        }
    }
    
    return false;
}



State* State::from_nfa(const NFA& nfa) {
    // Crear todos los estados
    std::vector<std::unique_ptr<State>> states;
    for (int i = 0; i < nfa.states(); ++i) {
        bool is_final = nfa.finalStates().count(i);
        states.emplace_back(std::make_unique<State>(i, is_final));
    }
    
    // Construir transiciones
    for (const auto& transition : nfa.getTransitionsMap()) {
        const auto& origin_symbol = transition.first;
        const auto& destinations = transition.second;
        
        int origin_id = origin_symbol.first;
        const std::string& symbol = origin_symbol.second;
        
        State* origin = states[origin_id].get();
        
        for (int dest_id : destinations) {
            State* dest = states[dest_id].get();
            
            if (symbol.empty()) {
                origin->add_epsilon_transition(dest);
            } else {
                origin->add_transition(symbol, dest);
            }
        }
    }
    
    // Devolver estado inicial (transferir ownership del primer elemento)
    State* start = states[nfa.startState()].release();
    
    // Retener el resto de estados en memoria
    for (auto& state : states) {
        if (state) state.release();
    }
    
    return start;
}

State* State::to_deterministic() {
    // Comparador para ordenar estados por ID
    struct StatePtrCompare {
        bool operator()(const State* a, const State* b) const {
            return a->id() < b->id();
        }
    };
    using StateSet = std::set<const State*, StatePtrCompare>;

    // Función para convertir unordered_set a StateSet
    auto set_from_unordered = [](const std::unordered_set<const State*>& uset) -> StateSet {
        StateSet sset;
        for (const State* ptr : uset) {
            sset.insert(ptr);
        }
        return sset;
    };

    static int dfa_id_counter = 0;  // Contador de IDs para estados DFA

    // Mapeos: StateSet <-> Estado DFA
    std::map<StateSet, State*> state_map;
    std::map<State*, StateSet> dfa_to_set_map;
    std::stack<State*> pending;

    // Paso 1: Calcular épsilon-clausura del estado inicial del NFA
    std::unordered_set<const State*> initial_closure = epsilon_closure();
    StateSet initial_set = set_from_unordered(initial_closure);

    // Determinar si el estado inicial DFA es final
    bool is_final = false;
    for (const State* s : initial_set) {
        if (s->is_final()) {
            is_final = true;
            break;
        }
    }

    // Crear estado inicial DFA
    State* start_dfa = new State(dfa_id_counter, is_final);
    dfa_id_counter++;
    for (const State* s : initial_set) {
        // Añadir ítems del NFA al estado DFA
        for (const Item& item : s->get_items()) {
            start_dfa->add_item(item);
        }
    }
    state_map[initial_set] = start_dfa;
    dfa_to_set_map[start_dfa] = initial_set;
    pending.push(start_dfa);

    // Procesar estados pendientes
    while (!pending.empty()) {
        State* current_dfa = pending.top();
        pending.pop();
        const StateSet& current_set = dfa_to_set_map[current_dfa];

        // Recolectar símbolos de transición
        std::set<Symbol> symbols;
        for (const State* state_nfa : current_set) {
            for (const auto& pair : state_nfa->transitions()) {
                symbols.insert(pair.first);
            }
        }

        // Procesar cada símbolo
        for (const Symbol& sym : symbols) {
            std::unordered_set<const State*> moved_set;
            // Calcular move(symbol)
            for (const State* state_nfa : current_set) {
                std::unordered_set<const State*> moved = state_nfa->move(sym);
                moved_set.insert(moved.begin(), moved.end());
            }

            // Calcular épsilon-clausura del move
            std::unordered_set<const State*> closure = moved_set.empty() ? 
                moved_set : epsilon_closure(moved_set);
            StateSet new_state_set = set_from_unordered(closure);

            // Crear nuevo estado DFA si es necesario
            State* next_dfa = nullptr;
            auto it = state_map.find(new_state_set);
            if (it == state_map.end()) {
                bool new_final = false;
                for (const State* s : new_state_set) {
                    if (s->is_final()) {
                        new_final = true;
                        break;
                    }
                }
                next_dfa = new State(dfa_id_counter++, new_final);
                
                //Actualizar los items del nuevo estado DFA
                for (const State* s : new_state_set) {
                    for (const Item& item : s->get_items()) {
                        next_dfa->add_item(item);
                    }
                }

                state_map[new_state_set] = next_dfa;
                dfa_to_set_map[next_dfa] = new_state_set;
                pending.push(next_dfa);
            } else {
                next_dfa = it->second;
            }

            // Añadir transición determinista
            current_dfa->add_transition(sym, next_dfa);
        }
    }

    return start_dfa;
}

std::string State::ToString() const {
    std::string result = "State ID: " + std::to_string(id_) + ", Final: " + (is_final_ ? "Yes" : "No");
    if (!tag_.empty()) {
        result += ", Tag: " + tag_;
    }
    
    result += "\nTransitions:\n";
    for (const auto& pair : transitions_) {
        result += "  Symbol: " + pair.first + " -> States: ";
        for (const State* state : pair.second) {
            result += std::to_string(state->id()) + " ";
        }
        result += "\n";
    } 
    return result;
}