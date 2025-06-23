#pragma once
#include <string>
#include <vector>
#include <cassert>
#include "../Automata/state.h"
#include "Regex.h"
#include "../Automata/nfa.h"
#include "Token.h"

/**
 * @class Lexer
 * @brief Analizador léxico que convierte texto en una secuencia de tokens.
 * 
 * Utiliza expresiones regulares y autómatas para identificar los tokens definidos en la gramática.
 */

class Lexer {
public:
    Lexer(std::vector<std::pair<std::string, std::string>> token_table,  
          Grammar& grammar, LALR1Parser& parser, bool verbose = false)
          : regexs_(build_regex(token_table, grammar, parser, verbose)), 
            owns_automaton_(false),
            verbose_(verbose) {
            if (verbose_) {
                std::cout << "Construyendo autómata del lexer..." << std::endl;
            }
            State* deterministic = build_automaton();
            automaton_ = deterministic;
            owns_automaton_ = true;
            if (verbose_) {
                std::cout << "Autómata del lexer construido exitosamente." << std::endl;
            }
        }
        
    // Destructor para limpiar memoria si es necesario
    ~Lexer() {
        if (owns_automaton_ && automaton_) {
            // Limpiar todos los estados del autómata
            auto all_states = automaton_->get_all_states();
            for (State* state : all_states) {
                delete state;
            }
        }
    }

    std::vector<State> build_regex(std::vector<std::pair<std::string, std::string>> table, 
                                  Grammar& grammar_, LALR1Parser& parser, bool verbose = false) {
        std::vector<State> states;
        int index = 0;
        if (verbose) {
            std::cout << "Construyendo expresiones regulares para " << table.size() << " tokens:" << std::endl;
        }
        for (const auto& [name, pattern] : table) {
            if (verbose) {
                std::cout << "  Procesando token '" << name << "' con patrón '" << pattern << "'" << std::endl;
            }
            NFA nfa = Regex(pattern, grammar_, parser, verbose).Automaton();
            State state = *State::from_nfa(nfa);
            for (const auto& s : state.get_all_states()) {
                if (s->is_final()) {
                    s->set_tag(name, index++);
                }
            }
            index++;
            states.push_back(state);
            if (verbose) {
                std::cout << "  Expresión regular para token '" << name << "' construida exitosamente" << std::endl;
            }
        }
        return states;
    }

    State* build_automaton() {
        int id = 1;
        if (verbose_) {
            std::cout << "Configurando IDs para los estados del autómata..." << std::endl;
        }
        for (auto& state : regexs_) {
            for (auto& s : state.get_all_states()) {
                s->set_id(id++);
            }
        }
        if (verbose_) {
            std::cout << "Creando estado inicial y agregando transiciones epsilon..." << std::endl;
        }
        State start = State(0);
        for (auto& state : regexs_) {
            start.add_epsilon_transition(&state);
        }
        if (verbose_) {
            std::cout << "Convirtiendo NFA a DFA..." << std::endl;
            std::cout << "Número total de estados en el NFA: " << id << std::endl;
        }
        State* deterministic = start.to_deterministic();
        if (verbose_) {
            std::cout << "DFA construido exitosamente con " << deterministic->get_all_states().size() << " estados." << std::endl;
        }
        return deterministic;
    }

    std::pair<State, std::string> walk(const std::string& input) {
        State* current_state = automaton_;
        State final_state;// final_state: id = -1, is_final = false
                        // CUIDADO CON EL ID. Ningun id al final debe ser -1.
        if (current_state->is_final()) {
            final_state = *current_state;
        }
        std::string token;
        std::string lex;
        for (char c : input) {
            bool found_transition = false;
            for (const auto& [symbol, next_states] : current_state->transitions()) {
                if (symbol == std::string(1, c)) {
                    // assert (next_states.size() == 1) && "Automaton should be deterministic.";
                    if (next_states.size() != 1) {
                        throw std::runtime_error("Automaton is not deterministic, multiple transitions found for symbol: " + std::string(1, c));
                    }
                    current_state = next_states[0];
                    lex += c;
                    found_transition = true;
                    if (current_state->is_final()) {
                        token = lex;
                        final_state = *current_state;
                    }
                }
                if (found_transition) break;
            }
            if (!found_transition) {
                if (!token.empty()) {
                    // If we found a token, return it
                    return {final_state, token};
                }
                // If no token found, return error state
                return {State(-1), ""}; // No transition found, return error state
            }
        }
        return {final_state, token};
    }

    std::vector<std::pair<std::string, std::string>> tokenize(const std::string& input) {
        std::vector<std::pair<std::string, std::string>> tokens;
        std::string remaining_text = input;
        while (!remaining_text.empty()) {
            auto [state, token] = walk(remaining_text);
            //Si hay un estado
            if (state.id() != -1) {
                // Para autómatas deserializados, usar directamente el tag del estado final
                std::string token_type = state.tag();
                
                // Si el tag está vacío, intentar buscar en los estados NFA (para compatibilidad con autómatas originales)
                if (token_type.empty()) {
                    std::vector<State> sorted_states;
                    for (const State* s : state.get_nfa_states()) {
                        if (s != nullptr) {  // Verificar que el puntero no sea nulo
                            sorted_states.push_back(*s);
                        }
                    }
                    if (!sorted_states.empty()) {
                        // Ordenarlos de menor a mayor segun el State.n_
                        std::sort(sorted_states.begin(), sorted_states.end(), [](const State& a, const State& b) {
                            return a.get_n() < b.get_n();
                        });
                        token_type = sorted_states[0].tag();
                    }
                }
                
                tokens.push_back(std::make_pair(token, token_type));
                remaining_text = remaining_text.substr(token.size());
            }
            else {
                tokens.push_back(std::make_pair(std::string(1, remaining_text[0]), "ERROR"));
                remaining_text = remaining_text.substr(1); // Skip one character on error
            }
        }
        tokens.push_back(std::make_pair("EOF","EOF"));
        return tokens;
    }

    /**
     * @brief Tokeniza un texto rastreando líneas y columnas para reportes de error precisos.
     * @param input Texto a tokenizar
     * @param grammar Gramática asociada
     * @return Vector de objetos Token con información de posición
     * @throw std::runtime_error Si hay un error de tokenización
     */
    std::vector<Token> tokenize_with_positions(const std::string& input, Grammar& grammar) {
        std::vector<Token> result;
        
        int line = 1;
        int column = 1;
        
        std::string remaining = input;
        
        while (!remaining.empty()) {
            auto [state, token] = walk(remaining);
            
            if (token.empty()) {
                // Si no hay token, es un error léxico
                char invalid_char = remaining[0];
                std::string error_msg = "Error léxico en línea " + std::to_string(line) + 
                                       ", columna " + std::to_string(column) + 
                                       ": carácter inválido '" + invalid_char + "'";
                throw std::runtime_error(error_msg);
            }
            
            // Guardar posición actual antes de actualizarla
            int token_line = line;
            int token_column = column;
            
            // Actualizar posición para el siguiente token
            for (char c : token) {
                if (c == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
            }
            
            // Filtrar espacios, tabulaciones, saltos de línea y comentarios
            if (state.tag() == "space" || state.tag() == "tab" || state.tag() == "newline" || state.tag() == "comment") {
                // Ignorar estos tokens pero seguir rastreando posición
            } else {
                // Crear un token con información de posición
                Token pos_token(state.tag(), token, grammar, token_line, token_column);
                result.push_back(pos_token);
            }
            
            // Descartar el token procesado
            remaining = remaining.substr(token.length());
        }
        
        // Añadir token de fin de archivo
        Token eof_token("EOF", "EOF", grammar, line, column);
        result.push_back(eof_token);
        
        return result;
    }

    // ============= MÉTODOS DE SERIALIZACIÓN DEL LEXER =============
    
    /**
     * Serializa el Lexer completo a un archivo
     * @param filename Nombre del archivo donde guardar el lexer
     * @return true si la serialización fue exitosa, false en caso contrario
     */
    bool serialize_lexer(const std::string& filename) const {
        if (verbose_) {
            std::cout << "Serializando lexer en archivo '" << filename << "'..." << std::endl;
        }
        bool result = automaton_->serialize_to_file(filename);
        if (verbose_) {
            if (result) {
                std::cout << "Lexer serializado exitosamente." << std::endl;
            } else {
                std::cout << "Error al serializar el lexer." << std::endl;
            }
        }
        return result;
    }
    
    /**
     * Serializa el Lexer completo a un archivo en una carpeta específica
     * @param filename Nombre del archivo donde guardar el lexer
     * @param directory Carpeta donde guardar el archivo (por defecto "hulk")
     * @return true si la serialización fue exitosa, false en caso contrario
     */
    bool serialize_lexer(const std::string& filename, const std::string& directory) const {
        if (verbose_) {
            std::cout << "Serializando lexer en '" << directory << "/" << filename << "'..." << std::endl;
        }
        bool result = automaton_->serialize_to_file(filename, directory);
        if (verbose_) {
            if (result) {
                std::cout << "Lexer serializado exitosamente." << std::endl;
            } else {
                std::cout << "Error al serializar el lexer." << std::endl;
            }
        }
        return result;
    }
    
    /**
     * Crea un nuevo Lexer desde un archivo serializado
     * @param filename Nombre del archivo con el lexer serializado
     * @return Puntero a un nuevo Lexer deserializado, o nullptr si hay error
     */
    static Lexer* deserialize_lexer(const std::string& filename, bool verbose = false) {
        if (verbose) {
            std::cout << "Deserializando lexer desde archivo '" << filename << "'..." << std::endl;
        }
        State* deserialized_automaton = State::deserialize_from_file(filename);
        if (!deserialized_automaton) {
            if (verbose) {
                std::cout << "Error al deserializar el lexer." << std::endl;
            }
            return nullptr;
        }
        
        if (verbose) {
            std::cout << "Lexer deserializado exitosamente." << std::endl;
            std::cout << "Número de estados en el DFA: " << deserialized_automaton->get_all_states().size() << std::endl;
        }
        
        // Crear un nuevo lexer con el autómata deserializado
        return new Lexer(deserialized_automaton, verbose);
    }
    
    /**
     * Crea un nuevo Lexer desde un archivo serializado en una carpeta específica
     * @param filename Nombre del archivo con el lexer serializado
     * @param directory Carpeta donde buscar el archivo
     * @return Puntero a un nuevo Lexer deserializado, o nullptr si hay error
     */
    static Lexer* deserialize_lexer(const std::string& filename, const std::string& directory, bool verbose = false) {
        if (verbose) {
            std::cout << "Deserializando lexer desde '" << directory << "/" << filename << "'..." << std::endl;
        }
        State* deserialized_automaton = State::deserialize_from_file(filename, directory);
        if (!deserialized_automaton) {
            if (verbose) {
                std::cout << "Error al deserializar el lexer." << std::endl;
            }
            return nullptr;
        }
        
        if (verbose) {
            std::cout << "Lexer deserializado exitosamente." << std::endl;
            std::cout << "Número de estados en el DFA: " << deserialized_automaton->get_all_states().size() << std::endl;
        }
        
        // Crear un nuevo lexer con el autómata deserializado
        return new Lexer(deserialized_automaton, verbose);
    }

private:
    std::vector<State> regexs_;
    State* automaton_;  // Cambiar a puntero para manejar autómatas deserializados
    bool owns_automaton_; // Flag para saber si debemos limpiar la memoria
    bool verbose_; // Flag para modo verbose
    
    // Constructor privado para deserialización
    explicit Lexer(State* automaton, bool verbose = false) 
        : automaton_(automaton), owns_automaton_(true), verbose_(verbose) {
        // Los regexs_ no son necesarios para un lexer deserializado
        // ya que toda la información está en el autómata final
    }
};
