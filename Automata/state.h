#include <map>
#include <set>
#include <vector>
#include <string>
#include <memory>
#include <unordered_set>
#include <queue>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <cstdlib>
#include "nfa.h"
#include "../Parser/Item.h"

#ifndef STATE_H
#define STATE_H

/**
 * @class State
 * @brief Representa un estado de un autómata finito (DFA/NFA), este ultimo definido como una serie de estados enlazados.
 * 
 * Contiene transiciones, información de aceptación y métodos para manipulación de autómatas.
 */

class State {
public:
    using Symbol = std::string;
    
    // Constructor
    State() : id_(-1), is_final_(false) {}
    State(int id, bool is_final = false);
    ~State();
    
    // Getters
    int id() const;
    bool is_final() const;
    //tag setter
    void set_tag(const std::string& tag, const int n) { tag_ = tag; n_ = n; }
    // tag getter
    string tag() const;

    // Cálculo de epsilon-clausura
    std::unordered_set<const State*> epsilon_closure() const;
    std::unordered_set<const State*> epsilon_closure(const std::unordered_set<const State*>& states) const;

    // Movimientos
    std::unordered_set<const State*> move(const Symbol& symbol) const;
    
    // Transiciones
    void add_transition(const Symbol& symbol, State* state);
    void add_epsilon_transition(State* state);
    bool has_transition(const Symbol& symbol) const;
    
    // Acceso a transiciones
    const std::map<Symbol, std::vector<State*>>& transitions() const;
    const std::set<State*>& epsilon_transitions() const;
    
    // Conversión desde NFA
    static State* from_nfa(const class NFA& nfa);

    // Conversion a determinista
    State* to_deterministic();

    // Saber si reconoce una cadena
    bool recognizes(const std::vector<Symbol>& input) const;

    // ToString
    std::string ToString() const;

    void set_id(int id) { id_ = id; }
    int get_id() const { return id_; }

    // Métodos para manejar ítems de LR
    void add_item(const Item& item) { items_.push_back(item); }
    const std::vector<Item>& get_items() const { return items_; }

    // Method to retrieve all states in the automaton
    std::vector<State*> get_all_states() {
        std::vector<State*> all_states;
        std::set<State*> visited;
        std::queue<State*> pending;
        
        pending.push(this);
        visited.insert(this);
        
        while (!pending.empty()) {
            State* current = pending.front();
            pending.pop();
            
            all_states.push_back(current);
            
            for (const auto& [symbol, states] : current->transitions_) {
                for (State* next_state : states) {
                    if (visited.find(next_state) == visited.end()) {
                        pending.push(next_state);
                        visited.insert(next_state);
                    }
                }
            }
        }
        
        return all_states;
    }
    const int get_n() const { return n_; }

    // Métodos de Serialización
    struct SerializedState {
        int id;
        bool is_final;
        std::string tag;
        int n;
        std::map<std::string, std::vector<int>> transitions; // symbol -> vector of state IDs
        std::set<int> epsilon_transitions; // set of state IDs
    };
    
    struct SerializedAutomaton {
        int start_state_id;
        std::vector<SerializedState> states;
    };
    
    // Serializar el autómata completo a un archivo
    bool serialize_to_file(const std::string& filename) const;
    
    // Deserializar un autómata desde un archivo
    static State* deserialize_from_file(const std::string& filename);
    
    // Métodos con carpeta personalizada
    bool serialize_to_file(const std::string& filename, const std::string& directory) const;
    static State* deserialize_from_file(const std::string& filename, const std::string& directory);
    
    // Métodos auxiliares para serialización
    SerializedAutomaton serialize_automaton() const;
    static State* deserialize_automaton(const SerializedAutomaton& serialized);
    
    // Método auxiliar para crear la ruta completa en la carpeta hulk
    static std::string get_hulk_path(const std::string& filename);
    static bool ensure_hulk_directory();
    
    // Métodos auxiliares para carpetas personalizadas
    static std::string get_custom_path(const std::string& filename, const std::string& directory);
    static bool ensure_directory(const std::string& directory);

private:
    int id_;
    bool is_final_;
    std::string tag_;
    std::map<Symbol, std::vector<State*>> transitions_;
    std::set<State*> epsilon_transitions_;
    std::vector<Item> items_;
    int n_;
    // Set of NFA states represented by this DFA state
    std::set<const State*> state_;
public:
    // Getter for the NFA states set
    const std::set<const State*>& get_nfa_states() const { return state_; }
};

#endif