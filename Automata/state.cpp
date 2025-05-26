#include "state.h"
#include "nfa.h"
#include <vector>
#include <memory>
#include <stack>

State::State(int id, bool is_final) : id_(id), is_final_(is_final) {}

State::~State() {
    std::unordered_set<State*> deleted;
    std::stack<State*> to_delete;
    
    to_delete.push(this);
    
    while (!to_delete.empty()) {
        State* current = to_delete.top();
        to_delete.pop();
        
        if (deleted.insert(current).second) {
            for (auto& pair : current->transitions_) {
                for (State* state : pair.second) {
                    to_delete.push(state);
                }
            }
            for (State* state : current->epsilon_transitions_) {
                to_delete.push(state);
            }
            delete current;
        }
    }
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