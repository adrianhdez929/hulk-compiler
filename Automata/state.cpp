#include "state.h"
#include "nfa.h"
#include <vector>
#include <memory>
#include <stack>
#include <iostream> // Para debug
#include <fstream>  // Para serialización
#include <climits>  // Para INT_MAX

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
    dfa_id_counter = 0;  // Resetear el contador al inicio de cada determinización

    // Mapeos: StateSet <-> Estado DFA
    std::map<StateSet, State*> state_map;
    std::map<State*, StateSet> dfa_to_set_map;
    std::stack<State*> pending;

    // Paso 1: Calcular épsilon-clausura del estado inicial del NFA
    std::unordered_set<const State*> initial_closure = epsilon_closure();
    StateSet initial_set = set_from_unordered(initial_closure);

    // Determinar si el estado inicial DFA es final
    bool is_final = false;
    std::string dfa_tag = "";
    int min_n = INT_MAX;  // Para determinar la prioridad del tag
    
    for (const State* s : initial_set) {
        if (s->is_final()) {
            is_final = true;
            // // Usar el tag del estado con menor número n (mayor prioridad)
            // if (s->get_n() < min_n && !s->tag().empty()) {
            //     min_n = s->get_n();
            //     dfa_tag = s->tag();
            // }
            break;
        }
    }

    // Crear estado inicial DFA
    State* start_dfa = new State(dfa_id_counter, is_final);
    if (is_final && !dfa_tag.empty()) {
        start_dfa->set_tag(dfa_tag, 0);  // Usar 0 como índice por defecto
    }
    dfa_id_counter++;
    for (const State* s : initial_set) {
        // Añadir ítems del NFA al estado DFA
        for (const Item& item : s->get_items()) {
            start_dfa->add_item(item);
        }
    }
    // Guardar los estados NFA correspondientes
    // Convert initial_set (set<const State*>) to set<State*>
    start_dfa->state_.clear();
    for (const State* s : initial_set) {
        start_dfa->state_.insert(const_cast<State*>(s));
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
                std::string new_dfa_tag = "";
                int new_min_n = INT_MAX;
                
                for (const State* s : new_state_set) {
                    if (s->is_final()) {
                        new_final = true;
                        // Usar el tag del estado con menor número n (mayor prioridad)
                        if (s->get_n() < new_min_n && !s->tag().empty()) {
                            new_min_n = s->get_n();
                            new_dfa_tag = s->tag();
                        }
                    }
                }
                
                next_dfa = new State(dfa_id_counter++, new_final);
                if (new_final && !new_dfa_tag.empty()) {
                    next_dfa->set_tag(new_dfa_tag, 0);  // Usar 0 como índice por defecto
                }
                
                //Actualizar los items del nuevo estado DFA
                for (const State* s : new_state_set) {
                    for (const Item& item : s->get_items()) {
                        next_dfa->add_item(item);
                    }
                }
                // Guardar los estados NFA correspondientes
                next_dfa->state_.clear();
                for (const State* s : new_state_set) {
                    next_dfa->state_.insert(const_cast<State*>(s));
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
    std::string result = "State " + std::to_string(id_) + " (";
    if (is_final_) result += "final";
    else result += "non-final";
    result += ")";
    if (!tag_.empty()) {
        result += " [tag: " + tag_ + "]";
    }
    return result;
}

// ============= MÉTODOS DE SERIALIZACIÓN =============

// Método auxiliar para crear la ruta completa en la carpeta hulk
std::string State::get_hulk_path(const std::string& filename) {
    return "hulk/" + filename;
}

// Método auxiliar para asegurar que existe la carpeta hulk
bool State::ensure_hulk_directory() {
    // Verificar si la carpeta hulk existe, si no, crearla
    std::ifstream test("hulk/");
    if (!test.good()) {
        // En sistemas Unix/Linux
        int result = system("mkdir -p hulk");
        if (result != 0) {
            std::cerr << "Error: No se pudo crear la carpeta hulk" << std::endl;
            return false;
        }
    }
    test.close();
    return true;
}

// Métodos auxiliares para carpetas personalizadas
std::string State::get_custom_path(const std::string& filename, const std::string& directory) {
    if (directory.empty()) {
        return filename;
    }
    
    // Asegurar que el directorio termine con /
    std::string dir = directory;
    if (dir.back() != '/' && dir.back() != '\\') {
        dir += "/";
    }
    
    return dir + filename;
}

bool State::ensure_directory(const std::string& directory) {
    if (directory.empty()) {
        return true;
    }
    
    // Verificar si la carpeta existe, si no, crearla
    std::ifstream test(directory + "/");
    if (!test.good()) {
        // En sistemas Unix/Linux
        std::string command = "mkdir -p " + directory;
        int result = system(command.c_str());
        if (result != 0) {
            std::cerr << "Error: No se pudo crear la carpeta " << directory << std::endl;
            return false;
        }
    }
    test.close();
    return true;
}

// Métodos de serialización con carpeta personalizada
bool State::serialize_to_file(const std::string& filename, const std::string& directory) const {
    try {
        // Asegurar que existe la carpeta
        if (!ensure_directory(directory)) {
            return false;
        }
        
        // Crear la ruta completa
        std::string full_path = get_custom_path(filename, directory);
        
        SerializedAutomaton serialized = serialize_automaton();
        
        std::ofstream file(full_path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: No se pudo abrir el archivo para escritura: " << full_path << std::endl;
            return false;
        }
        
        // Escribir número de estados
        size_t num_states = serialized.states.size();
        file.write(reinterpret_cast<const char*>(&num_states), sizeof(num_states));
        
        // Escribir ID del estado inicial
        file.write(reinterpret_cast<const char*>(&serialized.start_state_id), sizeof(serialized.start_state_id));
        
        // Escribir cada estado (mismo código que el método principal)
        for (const auto& state : serialized.states) {
            // Escribir datos básicos del estado
            file.write(reinterpret_cast<const char*>(&state.id), sizeof(state.id));
            file.write(reinterpret_cast<const char*>(&state.is_final), sizeof(state.is_final));
            file.write(reinterpret_cast<const char*>(&state.n), sizeof(state.n));
            
            // Escribir tag
            size_t tag_length = state.tag.length();
            file.write(reinterpret_cast<const char*>(&tag_length), sizeof(tag_length));
            file.write(state.tag.c_str(), tag_length);
            
            // Escribir transiciones normales
            size_t num_transitions = state.transitions.size();
            file.write(reinterpret_cast<const char*>(&num_transitions), sizeof(num_transitions));
            for (const auto& [symbol, dest_ids] : state.transitions) {
                // Escribir símbolo
                size_t symbol_length = symbol.length();
                file.write(reinterpret_cast<const char*>(&symbol_length), sizeof(symbol_length));
                file.write(symbol.c_str(), symbol_length);
                
                // Escribir destinos
                size_t num_destinations = dest_ids.size();
                file.write(reinterpret_cast<const char*>(&num_destinations), sizeof(num_destinations));
                for (int dest_id : dest_ids) {
                    file.write(reinterpret_cast<const char*>(&dest_id), sizeof(dest_id));
                }
            }
            
            // Escribir transiciones epsilon
            size_t num_epsilon = state.epsilon_transitions.size();
            file.write(reinterpret_cast<const char*>(&num_epsilon), sizeof(num_epsilon));
            for (int epsilon_id : state.epsilon_transitions) {
                file.write(reinterpret_cast<const char*>(&epsilon_id), sizeof(epsilon_id));
            }
        }
        
        file.close();
        std::cout << "Autómata serializado exitosamente en: " << full_path << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error durante la serialización: " << e.what() << std::endl;
        return false;
    }
}

State* State::deserialize_from_file(const std::string& filename, const std::string& directory) {
    try {
        // Crear la ruta completa
        std::string full_path = get_custom_path(filename, directory);
        
        std::ifstream file(full_path, std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Error: No se pudo abrir el archivo para lectura: " << full_path << std::endl;
            return nullptr;
        }
        
        SerializedAutomaton serialized;
        
        // Leer número de estados
        size_t num_states;
        file.read(reinterpret_cast<char*>(&num_states), sizeof(num_states));
        
        // Leer ID del estado inicial
        file.read(reinterpret_cast<char*>(&serialized.start_state_id), sizeof(serialized.start_state_id));
        
        // Leer cada estado (mismo código que el método principal)
        serialized.states.resize(num_states);
        for (size_t i = 0; i < num_states; ++i) {
            SerializedState& state = serialized.states[i];
            
            // Leer datos básicos del estado
            file.read(reinterpret_cast<char*>(&state.id), sizeof(state.id));
            file.read(reinterpret_cast<char*>(&state.is_final), sizeof(state.is_final));
            file.read(reinterpret_cast<char*>(&state.n), sizeof(state.n));
            
            // Leer tag
            size_t tag_length;
            file.read(reinterpret_cast<char*>(&tag_length), sizeof(tag_length));
            state.tag.resize(tag_length);
            file.read(&state.tag[0], tag_length);
            
            // Leer transiciones normales
            size_t num_transitions;
            file.read(reinterpret_cast<char*>(&num_transitions), sizeof(num_transitions));
            for (size_t j = 0; j < num_transitions; ++j) {
                // Leer símbolo
                size_t symbol_length;
                file.read(reinterpret_cast<char*>(&symbol_length), sizeof(symbol_length));
                std::string symbol(symbol_length, '\0');
                file.read(&symbol[0], symbol_length);
                
                // Leer destinos
                size_t num_destinations;
                file.read(reinterpret_cast<char*>(&num_destinations), sizeof(num_destinations));
                std::vector<int> dest_ids(num_destinations);
                for (size_t k = 0; k < num_destinations; ++k) {
                    file.read(reinterpret_cast<char*>(&dest_ids[k]), sizeof(dest_ids[k]));
                }
                state.transitions[symbol] = dest_ids;
            }
            
            // Leer transiciones epsilon
            size_t num_epsilon;
            file.read(reinterpret_cast<char*>(&num_epsilon), sizeof(num_epsilon));
            for (size_t j = 0; j < num_epsilon; ++j) {
                int epsilon_id;
                file.read(reinterpret_cast<char*>(&epsilon_id), sizeof(epsilon_id));
                state.epsilon_transitions.insert(epsilon_id);
            }
        }
        
        file.close();
        
        State* result = deserialize_automaton(serialized);
        if (result) {
            std::cout << "Autómata deserializado exitosamente desde: " << full_path << std::endl;
        }
        return result;
        
    } catch (const std::exception& e) {
        std::cerr << "Error durante la deserialización: " << e.what() << std::endl;
        return nullptr;
    }
}

State* State::deserialize_from_file(const std::string& filename) {
    // Usar automáticamente la carpeta hulk
    return deserialize_from_file(filename, "hulk");
}

State::SerializedAutomaton State::serialize_automaton() const {
    SerializedAutomaton result;
    result.start_state_id = this->id_;
    
    // Obtener todos los estados del autómata
    std::vector<State*> all_states = const_cast<State*>(this)->get_all_states();
    
    // Crear mapeo de punteros a IDs para asegurar consistencia
    std::unordered_map<State*, int> state_to_id;
    for (State* state : all_states) {
        state_to_id[state] = state->id_;
    }
    
    // Serializar cada estado
    for (State* state : all_states) {
        SerializedState serialized_state;
        serialized_state.id = state->id_;
        serialized_state.is_final = state->is_final_;
        serialized_state.tag = state->tag_;
        serialized_state.n = state->n_;
        
        // Serializar transiciones normales
        for (const auto& [symbol, next_states] : state->transitions_) {
            std::vector<int> dest_ids;
            for (State* next_state : next_states) {
                dest_ids.push_back(state_to_id[next_state]);
            }
            serialized_state.transitions[symbol] = dest_ids;
        }
        
        // Serializar transiciones epsilon
        for (State* epsilon_state : state->epsilon_transitions_) {
            serialized_state.epsilon_transitions.insert(state_to_id[epsilon_state]);
        }
        
        result.states.push_back(serialized_state);
    }
    
    return result;
}

State* State::deserialize_automaton(const SerializedAutomaton& serialized) {
    if (serialized.states.empty()) {
        std::cerr << "Error: No hay estados para deserializar" << std::endl;
        return nullptr;
    }
    
    // Crear todos los estados primero
    std::unordered_map<int, State*> id_to_state;
    for (const auto& serialized_state : serialized.states) {
        State* state = new State(serialized_state.id, serialized_state.is_final);
        state->tag_ = serialized_state.tag;
        state->n_ = serialized_state.n;
        id_to_state[serialized_state.id] = state;
    }
    
    // Reconstruir las transiciones
    for (const auto& serialized_state : serialized.states) {
        State* current_state = id_to_state[serialized_state.id];
        
        // Reconstruir transiciones normales
        for (const auto& [symbol, dest_ids] : serialized_state.transitions) {
            for (int dest_id : dest_ids) {
                if (id_to_state.find(dest_id) != id_to_state.end()) {
                    current_state->add_transition(symbol, id_to_state[dest_id]);
                } else {
                    std::cerr << "Advertencia: Estado destino " << dest_id << " no encontrado" << std::endl;
                }
            }
        }
        
        // Reconstruir transiciones epsilon
        for (int epsilon_id : serialized_state.epsilon_transitions) {
            if (id_to_state.find(epsilon_id) != id_to_state.end()) {
                current_state->add_epsilon_transition(id_to_state[epsilon_id]);
            } else {
                std::cerr << "Advertencia: Estado epsilon " << epsilon_id << " no encontrado" << std::endl;
            }
        }
    }
    
    // Retornar el estado inicial
    if (id_to_state.find(serialized.start_state_id) != id_to_state.end()) {
        return id_to_state[serialized.start_state_id];
    } else {
        std::cerr << "Error: Estado inicial " << serialized.start_state_id << " no encontrado" << std::endl;
        // Limpiar memoria antes de retornar nullptr
        for (auto& [id, state] : id_to_state) {
            delete state;
        }
        return nullptr;
    }
}

bool State::serialize_to_file(const std::string& filename) const {
    // Usar automáticamente la carpeta hulk
    return serialize_to_file(filename, "hulk");
}