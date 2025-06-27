#include "SLR1Parser.h"
#include "../Automata/utils/ContainerSet.h"
#include <stack>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "../Lexer/Token.h"
#include "../Logger/Logger.h"

// ============= IMPLEMENTACIÓN DE SERIALIZACIÓN DEL PARSER =============

// Funciones auxiliares para manejo de directorios (reutilizadas del State)
namespace {
    std::string get_hulk_path(const std::string& filename) {
        return "hulk/" + filename;
    }

    bool ensure_hulk_directory() {
        try {
            if (!std::filesystem::exists("hulk")) {
                return std::filesystem::create_directories("hulk");
            }
            return true;
        } catch (const std::exception& e) {
            LogError("Error creando directorio hulk: " + std::string(e.what()));
            return false;
        }
    }

    std::string get_custom_path(const std::string& filename, const std::string& directory) {
        return directory + "/" + filename;
    }

    bool ensure_directory(const std::string& directory) {
        try {
            if (!std::filesystem::exists(directory)) {
                return std::filesystem::create_directories(directory);
            }
            return true;
        } catch (const std::exception& e) {
            LogError("Error creando directorio " + directory + ": " + std::string(e.what()));
            return false;
        }
    }
}

SLR1Parser::SLR1Parser(Grammar& G, bool verbose)
    : G_(G), verbose_(verbose) {
    // Initialize action and goto tables
    action_ = std::map<std::pair<int, Symbol>, std::pair<std::string, int>>();
    goto_ = std::map<std::pair<int, Symbol>, int>();
    
    if (verbose_) {
        LogInfo("Inicializando parser SLR1 con gramática de " + std::to_string(G_.Productions().size()) + " producciones");
        LogDebug("Detalle de la gramática:");
        for (const auto& prod : G_.Productions()) {
            LogDebug("  " + prod.ToString() + " (ID: " + std::to_string(prod.get_id()) + ")");
        }
    }
    
    BuildParsingTable();
    
    if (verbose_) {
        LogInfo("Tablas de análisis sintáctico creadas: " + std::to_string(action_.size()) + " entradas en ACTION, " 
                + std::to_string(goto_.size()) + " entradas en GOTO");
    }
}

//Parse method
std::pair<std::vector<int>, std::vector<std::string>> SLR1Parser::Parse(const std::vector<std::string>& tokens) {
    // Convert string tokens to Terminal objects
    std::vector<Terminal> terminal_tokens;
    
    LogInfo("Iniciando análisis sintáctico SLR1 con " + std::to_string(tokens.size()) + " tokens");
    
    if (verbose_) {
        LogDebug("Lista de tokens recibidos por el parser:");
        for (size_t i = 0; i < tokens.size(); i++) {
            LogDebug("[" + std::to_string(i) + "] Token string: " + tokens[i]);
        }
    }
    
    for (const auto& token : tokens) {
        terminal_tokens.push_back(Terminal(token, G_));
    }
    return Parse(terminal_tokens);
}
std::pair<std::vector<int>, std::vector<std::string>> SLR1Parser::Parse(const std::vector<Terminal>& tokens) {
    std::vector<int> production_ids;
    std::vector<std::string> actions;
    
    // Initialize stack and state
    std::stack<int> state_stack;
    state_stack.push(0);
    std::stack<Terminal> symbol_stack;

    int index = 0;
    // Guardar el índice actual para proporcionar contexto en caso de error
    int current_position = 0;

    try {
        while (index < tokens.size() || !symbol_stack.empty()) {
            current_position = index;
            if (index < tokens.size()) {
                auto current_token = tokens[index];
                auto action_key = std::make_pair(state_stack.top(), current_token);

                if (action_.find(action_key) != action_.end()) {
                    auto action_value = action_[action_key];
                    if (action_value.first == SHIFT) {
                        // Shift action
                        state_stack.push(action_value.second);
                        symbol_stack.push(current_token);
                        actions.push_back(SHIFT);
                        
                        if (verbose_) {
                            LogDebug("SHIFT: Token '" + current_token.Name() + "' al estado " + 
                                    std::to_string(action_value.second));
                        }
                        
                        index++;
                    } else if (action_value.first == REDUCE) {
                        // Reduce action
                        auto production = G_.Productions()[action_value.second];
                        
                        if (verbose_) {
                            LogDebug("REDUCE por producción: " + production.ToString() + 
                                    " (ID: " + std::to_string(production.get_id()) + ")");
                        } else {
                            // Log básico incluso sin verbose
                            LogDebug("REDUCE: " + production.ToString());
                        }
                        
                        production_ids.push_back(production.get_id());
                        actions.push_back(REDUCE);
                        
                        int symbols_to_pop = production.Right().Symbols().size();
                        if (verbose_) {
                            LogDebug("Pop de " + std::to_string(symbols_to_pop) + 
                                    " símbolos de las pilas de estados y símbolos");
                        }
                        
                        for (int i = 0; i < symbols_to_pop; i++) {
                            state_stack.pop();
                            if (!symbol_stack.empty()) {
                                symbol_stack.pop();
                            }
                        }

                        auto goto_key = std::make_pair(state_stack.top(), *(production.Left()));
                        if (goto_.find(goto_key) != goto_.end()) {
                            int new_state = goto_[goto_key];
                            state_stack.push(new_state);
                            
                            if (verbose_) {
                                LogDebug("GOTO[" + std::to_string(state_stack.top()) + ", " + 
                                        production.Left()->Name() + "] = " + std::to_string(new_state));
                            }
                        } else {
                            // Error interno del parser - no se encontró una transición goto
                            std::string error_msg = "Error interno del parser: no se encontró transición GOTO para el no terminal '" + 
                                                production.Left()->Name() + "' en el estado " + std::to_string(state_stack.top());
                            LogError(error_msg);
                            throw std::runtime_error(error_msg);
                        }
                    } else if (action_value.first == OK) {
                        // Accept action
                        actions.push_back(OK);
                        
                        LogInfo("Parser SLR1: Entrada aceptada");
                        
                        if (verbose_) {
                            LogDebug("ACCEPT: La entrada ha sido reconocida correctamente por la gramática");
                        }
                        if (G_.IsAugmented()) {
                            // If the grammar is augmented, we can consider the production as accepted
                            for (const auto& production : G_.Productions()) {
                                if (production.Left() == G_.GetStartSymbol()) {
                                    production_ids.push_back(production.get_id());
                                }
                            }
                        } else {
                            throw std::runtime_error("Grammar is not augmented, cannot accept.");
                        }
                        break;
                    } else {
                        throw std::runtime_error("Unknown action: " + action_value.first);
                    }
                } else {
                    // No se encontró una acción para este estado y token
                    auto [error_msg, expected_tokens] = generateErrorMessage(state_stack.top(), current_token.Name());
                    
                    // // Enriquecer el mensaje con información de posición si el token es un Token
                    // std::string enhanced_error = error_msg;
                    
                    // // Intentar hacer un cast dinámico a Token para obtener la información de línea y columna
                    // const Token* token_with_position = dynamic_cast<const Token*>(&current_token);
                    // if (token_with_position) {
                    //     std::string position_info = " en línea " + std::to_string(token_with_position->Line()) + 
                    //                               ", columna " + std::to_string(token_with_position->Column());
                    //     enhanced_error += position_info;
                    // }
                    
                    // Si estamos en modo verbose, mostramos información de depuración
                    if (false && verbose_) {
                        // Código omitido para evitar impresiones en consola
                    }
                    
                    // Lanzar una excepción especializada con detalles del error
                    throw ParsingError(error_msg, state_stack.top(), current_token.Name(), expected_tokens);
                }
            } else {
                // Si no quedan más tokens, comprobar si se puede reducir o aceptar
                bool found_action = false;
                for (const auto& [key, value] : action_) {
                    if (key.first == state_stack.top() && value.first == REDUCE) {
                        auto production = G_.Productions()[value.second];
                        production_ids.push_back(production.get_id());
                        actions.push_back(REDUCE);
                        
                        for (int i = 0; i < production.Right().Symbols().size(); i++) {
                            state_stack.pop();
                            if (!symbol_stack.empty()) {
                                symbol_stack.pop();
                            }
                        }
                        
                        auto goto_key = std::make_pair(state_stack.top(), *(production.Left()));
                        if (goto_.find(goto_key) != goto_.end()) {
                            state_stack.push(goto_[goto_key]);
                            found_action = true;
                            break;
                        }
                    } else if (key.first == state_stack.top() && value.first == OK && symbol_stack.empty()) {
                        actions.push_back(OK);
                        found_action = true;
                        break;
                    }
                }
                
                if (!found_action) {
                    // No se pudo reducir ni aceptar
                    auto expected_tokens = getExpectedTokens(state_stack.top());
                    std::string error_msg = "Error de sintaxis: fin de entrada inesperado";
                    throw ParsingError(error_msg, state_stack.top(), "EOF", expected_tokens);
                }
            }
        }
    } catch (const ParsingError& e) {
        // Mejorar el mensaje de error con contexto visual
        std::string enhanced_message = e.what();
        enhanced_message += "\n\nContexto del error:";
        
        // Crear representación visual del contexto del error usando tokens con posición
        int position = current_position;
        int context_start = std::max(0, position - 3);
        int context_end = std::min(static_cast<int>(tokens.size()) - 1, position + 3);
        
        // Mostrar contexto con información de posición
        enhanced_message += "\n\n";
        for (int i = context_start; i <= context_end; ++i) {
            // Intentar obtener información detallada si es un Token
            const Token* token_with_position = dynamic_cast<const Token*>(&tokens[i]);
            std::string token_repr;
            
            if (token_with_position) {
                // Si es un Token, usar su método ToString que incluye línea y columna
                token_repr = token_with_position->ToString();
            } else {
                // Si es solo un Terminal, usar su nombre
                token_repr = tokens[i].Name();
            }
            
            if (i == position) {
                enhanced_message += ">> " + token_repr + " <<\n";
            } else {
                enhanced_message += "   " + token_repr + "\n";
            }
        }
        
        throw ParsingError(enhanced_message, e.getState(), e.getToken(), e.getExpectedTokens());
    } catch (const std::exception& e) {
        // Re-lanzar excepciones normales
        throw;
    }
    
    return std::make_pair(production_ids, actions);
}
void SLR1Parser::BuildParsingTable() {
    G_.Augment();
    if (!G_.IsAugmented()) {
        throw std::runtime_error("Grammar is not augmented");
    }
    auto firsts = compute_firsts();
    // Los conjuntos FIRST ya están registrados dentro de compute_firsts()
    
    auto follows = compute_follows(firsts);
    // Los conjuntos FOLLOW ya están registrados dentro de compute_follows()
    
    State* automaton = BuildLR0Automaton().to_deterministic();
    for (const auto& state : automaton->get_all_states()) {
        int state_id = state->id();
        for (const auto& item : state->get_items()) {
            if (item.IsReduceItem()) {
                // Reduce action
                auto production = item.production();
                if (production->Left() == G_.GetStartSymbol()) {
                    // Accept action
                    Register(action_, {state_id, *(G_.GetEndOfFile())}, {OK, 0});
                } else {
                    // Regular reduce action
                    auto lookaheads = follows[Sentence(production->Left())].get_values();
                    for (const auto& lookahead : lookaheads) {
                        auto terminal_ptr = G_.GetSymbol(lookahead);
                        if (terminal_ptr) {
                            Register(action_, {state_id, *terminal_ptr}, {REDUCE, production->get_id()});
                        }
                    }
                }
            } else {
                // Shift action
                auto next_symbol = item.NextSymbol();
                if (next_symbol && next_symbol->IsTerminal()) {
                    auto next_state = state->move(next_symbol->Name());
                    if (!next_state.empty()) {
                        Register(action_, {state_id, *next_symbol}, {SHIFT, (*next_state.begin())->id()});
                    }
                } else if (next_symbol && next_symbol->IsNonTerminal()) {
                    // Goto action
                    auto next_state = state->move(next_symbol->Name());
                    if (!next_state.empty()) {
                        Register(goto_, {state_id, *next_symbol}, (*next_state.begin())->id());
                    }
                }
            }
        }
    }
}
void SLR1Parser::Register(std::map<std::pair<int, Symbol>, std::pair<std::string, int>>& table, 
                                 const std::pair<int, Symbol>& key, 
                                 const std::pair<std::string, int>& value) {
    if (verbose_) {
        std::string action_type = value.first;
        std::string log_msg = "ACTION[" + std::to_string(key.first) + ", " + key.second.Name() + "] = ";
        
        if (action_type == SHIFT) {
            log_msg += "SHIFT " + std::to_string(value.second);
        } else if (action_type == REDUCE) {
            auto production = G_.Productions()[value.second];
            log_msg += "REDUCE by " + production.ToString() + " (ID: " + std::to_string(production.get_id()) + ")";
        } else if (action_type == OK) {
            log_msg += "ACCEPT";
        } else {
            log_msg += action_type + " " + std::to_string(value.second);
        }
        
        LogDebug(log_msg);
    }
    table[key] = value;
}
void SLR1Parser::Register(std::map<std::pair<int, Symbol>, int>& table, 
                                 const std::pair<int, Symbol>& key, 
                                 int value) {
    if (verbose_) {
        LogDebug("GOTO[" + std::to_string(key.first) + ", " + key.second.Name() + "] = " + std::to_string(value));
    }
    table[key] = value;
}


State SLR1Parser::BuildLR0Automaton() {
    assert(G_.GetStartSymbol()->productions.size() == 1 && "Grammar must be augmented");

    auto start_production = G_.GetStartSymbol()->productions[0];
    auto start_item = Item(std::make_shared<Production>(start_production), 0, ContainerSet<string>());
    State automaton(0, true);
    automaton.add_item(start_item);

    std::queue<Item> pending;
    pending.push(start_item);

    std::map<Item, State*> visited;
    visited[start_item] = &automaton;

    int state_id = 0; // Start state ID from 0

    while (!pending.empty()) {
        auto current_item = pending.front();
        pending.pop();
        if (current_item.IsReduceItem()) {
            continue; // Skip reduced items
        }
        auto next_symbol = current_item.NextSymbol();
        // if (next_symbol == nullptr) {
        //     continue; // No next symbol, skip
        // }
        Item next_item = *current_item.NextItem();
        if (visited.find(next_item) == visited.end()) {
            visited[next_item] = new State(state_id, true);
            state_id++;
            visited[next_item]->add_item(next_item);
            // automaton.add_item(next_item);
            pending.push(next_item);
        }

        vector<Item> epsilon_transition_states;
        if (next_symbol->IsNonTerminal()) {
            for (const auto& production : G_.Productions()) {
                if (production.Left() == next_symbol) {
                    Item new_item(std::make_shared<Production>(production), 0, ContainerSet<string>());
                    if (visited.find(new_item) == visited.end()) {
                        visited[new_item] = new State(state_id++, true);
                        visited[new_item]->add_item(new_item);
                        pending.push(new_item);
                    }
                    // Add transition for the non-terminal
                    epsilon_transition_states.push_back(new_item);
                }
            }
        }

        State* current_state = visited[current_item];
        current_state->add_transition(next_symbol->Name(), visited[next_item]);
        for (const auto& epsilon_state : epsilon_transition_states) {
            current_state->add_epsilon_transition(visited[epsilon_state]);
        }
    }

    // Guardar todos los estados creados para liberarlos después
    for (auto& [items, state] : visited) {
        if (state != &automaton) {  // No añadimos el estado automaton ya que se devuelve por valor
            automaton_states_.push_back(state);
        }
    }
    return automaton;

}

map<Sentence, ContainerSet<string>> SLR1Parser::compute_firsts() {
    map<Sentence, ContainerSet<string>> firsts;
    bool changed = true;
    
    LogDebug("Iniciando cálculo de conjuntos FIRST");
    
    // Inicializar primeros para terminales
    for (const auto& terminal : G_.Terminals()) {
        if (terminal->IsEndOfFile()) {
            continue; // Skip EndOfFile terminal
        }
        ContainerSet<string> cs;
        cs.add(terminal->Name());
        firsts[Sentence(terminal)] = cs;
        
        if (verbose_) {
            LogDebug("FIRST(" + terminal->Name() + ") = {" + terminal->Name() + "} (terminal)");
        }
    }
    // firsts[Sentence(G_.GetEndOfFile())] = ContainerSet<string>().add(G_.GetEndOfFile()->Name());
    
    // Inicializar primeros para no terminales
    for (const auto& nonterminal : G_.NonTerminals()) {
        firsts[Sentence(nonterminal)] = ContainerSet<string>();
        
        if (verbose_) {
            LogDebug("FIRST(" + nonterminal->Name() + ") inicializado como conjunto vacío");
        }
    }

    // for (const auto& prod : G_.Productions()) {
    //     auto right = prod.Right();
    //     firsts[right] = ContainerSet<string>();
    // }
    
    int iteration = 0;
    while (changed == true) {
        changed = false;
        iteration++;
        
        if (verbose_) {
            LogDebug("Iteración " + std::to_string(iteration) + " del algoritmo de cálculo de FIRST");
        }

        for (const auto& prod : G_.Productions()) {
            const auto& X = Sentence(prod.Left());
            const auto& alpha = prod.Right();

            // Firsts de X
            auto& first_X = firsts[X];

            // Firsts de alpha
            auto& first_alpha = firsts[alpha];
            
            // Si estamos en modo verbose, registramos la producción que estamos analizando
            if (verbose_) {
                LogDebug("Analizando producción: " + prod.ToString());
            }

            ContainerSet<string> local_first = compute_local_firsts(alpha, firsts);

            // Registramos los cambios si hay alguno
            bool changed_alpha = first_alpha.hard_update(local_first);
            bool changed_X = first_X.hard_update(local_first);
            changed = changed || changed_alpha || changed_X;
            
            if (verbose_ && (changed_alpha || changed_X)) {
                std::string first_str = "{";
                bool first = true;
                for (const auto& val : local_first.get_values()) {
                    if (!first) first_str += ", ";
                    first_str += val;
                    first = false;
                }
                if (local_first.contains_epsilon()) {
                    if (!first) first_str += ", ";
                    first_str += "ε";
                }
                first_str += "}";
                
                if (changed_alpha) {
                    LogDebug("FIRST de la parte derecha actualizado: FIRST(" + alpha.ToString() + ") = " + first_str);
                }
                
                if (changed_X) {
                    LogDebug("FIRST del no terminal actualizado: FIRST(" + X.ToString() + ") = " + first_str);
                }
            }
        }
    }
    
    // Registrar los conjuntos FIRST finales
    if (verbose_) {
        LogDebug("=== CONJUNTOS FIRST CALCULADOS ===");
        for (const auto& [sentence, set] : firsts) {
            std::string values = "{";
            bool first = true;
            for (const auto& val : set.get_values()) {
                if (!first) values += ", ";
                values += val;
                first = false;
            }
            if (set.contains_epsilon()) {
                if (!first) values += ", ";
                values += "ε";
            }
            values += "}";
            LogDebug("FIRST(" + sentence.ToString() + ") = " + values);
        }
    } else {
        // Versión resumida para log siempre (no verbose)
        LogInfo("Conjuntos FIRST calculados");
    }
    
    return firsts;
};

ContainerSet<string> SLR1Parser::compute_local_firsts(const Sentence& alpha, const map<Sentence, ContainerSet<string>>& firsts) {
    //Compute local first
    ContainerSet<string> local_first = ContainerSet<string>();
    auto symbols = alpha.Symbols();
    // If alpha is epsilon, add epsilon to local first
    bool alpha_is_epsilon = false;
    for (const auto& symbol : symbols) {
        if (symbol->IsEpsilon()) {
            alpha_is_epsilon = true;
            break;
        }
    }
    if (alpha_is_epsilon) {
        local_first.set_epsilon();
    } else {
        // local_first.update(firsts.at(symbols[0]));
        if (symbols[0]->IsEndOfFile()){
            auto EOFile = G_.GetEndOfFile();
            local_first.add(EOFile->Name());
        } else {
            local_first.update(firsts.at(Sentence(symbols[0])));
        }
        // update_container_set(local_first, firsts.at(symbols[0]));
        int i = 0;
        // std::shared_ptr<Symbol> s = symbols[i];
        Sentence s = Sentence(symbols[i]);
        while (firsts.at(s).contains_epsilon()) {
            if (i == symbols.size() - 1) {
                local_first.set_epsilon();
                break;
            }
            i++;
            s = Sentence(symbols[i]);
                // i++;
                // s = symbols[i];
                // if (!firsts.at(Sentence(s)).contains_epsilon()) {
                //     update_container_set(local_first, firsts.at(Sentence(s)));
                //     break;
                // }
            
            if (!firsts.at(s).contains_epsilon()) {
                local_first.update(firsts.at(s));
                break;
            }
            // } else {
            //     local_first.add(G_.GetEpsilon());
            //     local_first.set_epsilon(true);
            //     break;
            // }
        }
    }
    return local_first;
}

std::map<Sentence, ContainerSet<string>> SLR1Parser::compute_follows(const map<Sentence, ContainerSet<string>>& symbol_firsts) {
    std::map<Sentence, ContainerSet<string>> follows;
    bool changed = true;

    LogDebug("Iniciando cálculo de conjuntos FOLLOW");

    // Inicializar FOLLOW para todos los no terminales
    for (const auto& nonterminal : G_.NonTerminals()) {
        follows[Sentence(nonterminal)] = ContainerSet<string>();
        if (verbose_) {
            LogDebug("FOLLOW(" + nonterminal->Name() + ") inicializado como conjunto vacío");
        }
    }
    
    // Agregar EOF al FOLLOW del símbolo inicial
    Sentence start_sentence = Sentence(G_.GetStartSymbol());
    shared_ptr<Symbol> EOFile = G_.GetEndOfFile();
    follows[start_sentence] = ContainerSet<string>();
    follows[start_sentence].add(EOFile->Name());
    
    if (verbose_) {
        LogDebug("FOLLOW(" + start_sentence.ToString() + ") = {" + EOFile->Name() + "} (símbolo inicial)");
    }

    int iteration = 0;
    while (changed == true) {
        changed = false;
        iteration++;
        
        if (verbose_) {
            LogDebug("Iteración " + std::to_string(iteration) + " del algoritmo de cálculo de FOLLOW");
        }
        
        for (const auto& prod : G_.Productions()) {
            const auto& X = Sentence(prod.Left());
            const auto& alpha = prod.Right();

            if (verbose_) {
                LogDebug("Analizando producción: " + prod.ToString());
            }

            auto& follows_x = follows[X];

            // Si la parte derecha es epsilon, no hay nada que procesar
            bool is_epsilon = false;
            for (const auto& symbol : alpha.Symbols()) {
                if (symbol->IsEpsilon()) {
                    is_epsilon = true;
                    break;
                }
            }
            
            if (is_epsilon) {
                if (verbose_) {
                    LogDebug("La parte derecha es epsilon, saltando esta producción");
                }
                continue;
            }
            
            int n = alpha.Symbols().size() - 1;
            if (n == 0) {
                auto& current_symbol = alpha.Symbols()[0];
                if (current_symbol->IsNonTerminal()) {
                    bool updated = follows[Sentence(current_symbol)].update(follows_x);
                    
                    if (updated && verbose_) {
                        std::string follow_str = "{";
                        bool first = true;
                        for (const auto& val : follows_x.get_values()) {
                            if (!first) follow_str += ", ";
                            follow_str += val;
                            first = false;
                        }
                        follow_str += "}";
                        
                        LogDebug("FOLLOW(" + current_symbol->Name() + ") actualizado con FOLLOW(" + X.ToString() + ") = " + follow_str);
                    }
                    
                    changed |= updated;
                }
            }
            else {
                for (int i = 0; i < n; i++) {
                    auto& Y = alpha.Symbols()[i];
                    auto& beta = alpha.Symbols()[i + 1];
                    Sentence Y_sentence = Sentence(Y);
                    Sentence beta_sentence = Sentence(beta);
                    
                    if (Y->IsNonTerminal()) {
                        if (symbol_firsts.find(beta_sentence) != symbol_firsts.end()) {
                            bool updated_first = follows[Y_sentence].update(symbol_firsts.at(beta_sentence));
                            
                            if (updated_first && verbose_) {
                                std::string first_str = "{";
                                bool first = true;
                                for (const auto& val : symbol_firsts.at(beta_sentence).get_values()) {
                                    if (!first) first_str += ", ";
                                    first_str += val;
                                    first = false;
                                }
                                first_str += "}";
                                
                                LogDebug("FOLLOW(" + Y->Name() + ") actualizado con FIRST(" + beta->Name() + ") = " + first_str);
                            }
                            
                            changed |= updated_first;
                            
                            if (symbol_firsts.at(beta_sentence).contains_epsilon()) {
                                bool updated_follow = follows[Y_sentence].update(follows_x);
                                
                                if (updated_follow && verbose_) {
                                    LogDebug("FIRST(" + beta->Name() + ") contiene epsilon, agregando FOLLOW(" + X.ToString() + ") a FOLLOW(" + Y->Name() + ")");
                                }
                                
                                changed |= updated_follow;
                            }
                        }
                    }
                    
                    if (i == n-1 && beta->IsNonTerminal()) {
                        bool updated = follows[beta_sentence].update(follows_x);
                        
                        if (updated && verbose_) {
                            LogDebug("Último símbolo en la parte derecha: agregando FOLLOW(" + X.ToString() + ") a FOLLOW(" + beta->Name() + ")");
                        }
                        
                        changed |= updated;
                    }
                }
            }
        }
    }
    
    // Registrar los conjuntos FOLLOW finales
    if (verbose_) {
        LogDebug("=== CONJUNTOS FOLLOW CALCULADOS ===");
        for (const auto& [sentence, set] : follows) {
            std::string values = "{";
            bool first = true;
            for (const auto& val : set.get_values()) {
                if (!first) values += ", ";
                values += val;
                first = false;
            }
            values += "}";
            LogDebug("FOLLOW(" + sentence.ToString() + ") = " + values);
        }
    } else {
        // Versión resumida para log siempre (no verbose)
        LogInfo("Conjuntos FOLLOW calculados");
    }
    
    return follows;
};
// Método para limpiar todos los estados creados
void SLR1Parser::CleanupAutomatonStates() {
    // Crear un conjunto para evitar eliminar el mismo estado más de una vez
    std::unordered_set<State*> visited;
    
    for (auto* state : automaton_states_) {
        if (visited.find(state) == visited.end()) {
            visited.insert(state);
            delete state;
        }
    }
    
    automaton_states_.clear();
}

// Destructor de SLR1Parser
SLR1Parser::~SLR1Parser() {
    CleanupAutomatonStates();
}

// ============= IMPLEMENTACIÓN DE SERIALIZACIÓN DEL PARSER =============

// Constructor privado para deserialización
SLR1Parser::SLR1Parser(Grammar& G, 
                       const std::map<std::pair<int, Symbol>, std::pair<std::string, int>>& action,
                       const std::map<std::pair<int, Symbol>, int>& goto_table,
                       bool verbose)
    : G_(G), verbose_(verbose), action_(action), goto_(goto_table) {
    // Asegurar que la gramática esté aumentada para el parser deserializado
    G_.Augment();
    if (!G_.IsAugmented()) {
        throw std::runtime_error("Error: No se pudo aumentar la gramática para el parser deserializado");
    }
    
    // No necesitamos automaton_states_ para un parser deserializado ya que
    // toda la información está en las tablas action_ y goto_
}

bool SLR1Parser::serialize_parser(const std::string& filename) const {
    if (!ensure_hulk_directory()) {
        return false;
    }
    return serialize_parser(filename, "hulk");
}

bool SLR1Parser::serialize_parser(const std::string& filename, const std::string& directory) const {
    if (!ensure_directory(directory)) {
        return false;
    }
    
    std::string filepath = get_custom_path(filename, directory);
    std::ofstream file(filepath, std::ios::binary);
    
    if (!file.is_open()) {
        LogError("No se pudo abrir el archivo para escritura: " + filepath);
        return false;
    }
    
    try {
        // Escribir firma del archivo
        const char* signature = "SLR1PARSER";
        file.write(signature, 10);
        
        // Escribir versión
        uint32_t version = 1;
        file.write(reinterpret_cast<const char*>(&version), sizeof(version));
        
        // Escribir flag verbose_
        file.write(reinterpret_cast<const char*>(&verbose_), sizeof(verbose_));
        
        // Serializar tabla action_
        size_t action_size = action_.size();
        file.write(reinterpret_cast<const char*>(&action_size), sizeof(action_size));
        
        for (const auto& [key, value] : action_) {
            // Escribir key: pair<int, Symbol>
            file.write(reinterpret_cast<const char*>(&key.first), sizeof(key.first));
            
            // Escribir Symbol (nombre y tipo)
            std::string symbol_name = key.second.Name();
            size_t name_length = symbol_name.length();
            file.write(reinterpret_cast<const char*>(&name_length), sizeof(name_length));
            file.write(symbol_name.c_str(), name_length);
            
            bool is_terminal = key.second.IsTerminal();
            file.write(reinterpret_cast<const char*>(&is_terminal), sizeof(is_terminal));
            
            // Escribir value: pair<string, int>
            std::string action_type = value.first;
            size_t action_length = action_type.length();
            file.write(reinterpret_cast<const char*>(&action_length), sizeof(action_length));
            file.write(action_type.c_str(), action_length);
            
            file.write(reinterpret_cast<const char*>(&value.second), sizeof(value.second));
        }
        
        // Serializar tabla goto_
        size_t goto_size = goto_.size();
        file.write(reinterpret_cast<const char*>(&goto_size), sizeof(goto_size));
        
        for (const auto& [key, value] : goto_) {
            // Escribir key: pair<int, Symbol>
            file.write(reinterpret_cast<const char*>(&key.first), sizeof(key.first));
            
            // Escribir Symbol
            std::string symbol_name = key.second.Name();
            size_t name_length = symbol_name.length();
            file.write(reinterpret_cast<const char*>(&name_length), sizeof(name_length));
            file.write(symbol_name.c_str(), name_length);
            
            bool is_terminal = key.second.IsTerminal();
            file.write(reinterpret_cast<const char*>(&is_terminal), sizeof(is_terminal));
            
            // Escribir value: int
            file.write(reinterpret_cast<const char*>(&value), sizeof(value));
        }
        
        file.close();

        return true;
        
    } catch (const std::exception& e) {
        LogError("Error durante la serialización del parser: " + std::string(e.what()));
        file.close();
        return false;
    }
}

SLR1Parser* SLR1Parser::deserialize_parser(const std::string& filename, Grammar& grammar) {
    return deserialize_parser(filename, "hulk", grammar);
}

SLR1Parser* SLR1Parser::deserialize_parser(const std::string& filename, const std::string& directory, Grammar& grammar) {
    std::string filepath = get_custom_path(filename, directory);
    std::ifstream file(filepath, std::ios::binary);
    
    if (!file.is_open()) {
        LogError("No se pudo abrir el archivo para lectura: " + filepath);
        return nullptr;
    }
    
    try {
        // Verificar firma
        char signature[11] = {0};
        file.read(signature, 10);
        if (std::string(signature) != "SLR1PARSER") {
            LogError("Archivo no es un parser serializado válido");
            file.close();
            return nullptr;
        }
        
        // Leer versión
        uint32_t version;
        file.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (version != 1) {
            LogError("Versión de parser no soportada: " + std::to_string(version));
            file.close();
            return nullptr;
        }
        
        // Leer flag verbose_
        bool verbose;
        file.read(reinterpret_cast<char*>(&verbose), sizeof(verbose));
        
        // Deserializar tabla action_
        std::map<std::pair<int, Symbol>, std::pair<std::string, int>> action;
        size_t action_size;
        file.read(reinterpret_cast<char*>(&action_size), sizeof(action_size));
        
        for (size_t i = 0; i < action_size; ++i) {
            // Leer key
            int state_id;
            file.read(reinterpret_cast<char*>(&state_id), sizeof(state_id));
            
            // Leer Symbol
            size_t name_length;
            file.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));
            std::string symbol_name(name_length, '\0');
            file.read(&symbol_name[0], name_length);
            
            bool is_terminal;
            file.read(reinterpret_cast<char*>(&is_terminal), sizeof(is_terminal));
            
            // Buscar el símbolo en la gramática
            auto symbol_ptr = grammar.GetSymbol(symbol_name);
            if (!symbol_ptr) {
                std::cerr << "Error: Símbolo no encontrado en gramática: " << symbol_name << std::endl;
                file.close();
                return nullptr;
            }
            
            // Leer value
            size_t action_length;
            file.read(reinterpret_cast<char*>(&action_length), sizeof(action_length));
            std::string action_type(action_length, '\0');
            file.read(&action_type[0], action_length);
            
            int action_value;
            file.read(reinterpret_cast<char*>(&action_value), sizeof(action_value));
            
            action[{state_id, *symbol_ptr}] = {action_type, action_value};
        }
        
        // Deserializar tabla goto_
        std::map<std::pair<int, Symbol>, int> goto_table;
        size_t goto_size;
        file.read(reinterpret_cast<char*>(&goto_size), sizeof(goto_size));
        
        for (size_t i = 0; i < goto_size; ++i) {
            // Leer key
            int state_id;
            file.read(reinterpret_cast<char*>(&state_id), sizeof(state_id));
            
            // Leer Symbol
            size_t name_length;
            file.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));
            std::string symbol_name(name_length, '\0');
            file.read(&symbol_name[0], name_length);
            
            bool is_terminal;
            file.read(reinterpret_cast<char*>(&is_terminal), sizeof(is_terminal));
            
            // Buscar el símbolo en la gramática
            auto symbol_ptr = grammar.GetSymbol(symbol_name);
            if (!symbol_ptr) {
                std::cerr << "Error: Símbolo no encontrado en gramática: " << symbol_name << std::endl;
                file.close();
                return nullptr;
            }
            
            // Leer value
            int goto_value;
            file.read(reinterpret_cast<char*>(&goto_value), sizeof(goto_value));
            
            goto_table[{state_id, *symbol_ptr}] = goto_value;
        }
        
        file.close();

        
        // Crear nuevo parser con las tablas deserializadas
        return new SLR1Parser(grammar, action, goto_table, verbose);
        
    } catch (const std::exception& e) {
        LogError("Error durante la deserialización del parser: " + std::string(e.what()));
        file.close();
        return nullptr;
    }
}

std::vector<std::string> SLR1Parser::getExpectedTokens(int state_id) const {
    std::vector<std::string> expected_tokens;
    
    // Buscar todas las acciones válidas para este estado
    for (const auto& [key, value] : action_) {
        if (key.first == state_id) {
            expected_tokens.push_back(key.second.Name());
        }
    }
    
    return expected_tokens;
}

std::pair<std::string, std::vector<std::string>> SLR1Parser::generateErrorMessage(int state_id, const std::string& token) const {
    std::vector<std::string> expected_tokens = getExpectedTokens(state_id);
    
    std::string error_msg = "Error de sintaxis: token inesperado '" + token + "'";
    if (!expected_tokens.empty()) {
        error_msg += ". Se esperaba: ";
        for (size_t i = 0; i < expected_tokens.size(); ++i) {
            if (i > 0) {
                error_msg += (i == expected_tokens.size() - 1) ? " o " : ", ";
            }
            error_msg += "'" + expected_tokens[i] + "'";
        }
    }
    
    return {error_msg, expected_tokens};
}

std::string SLR1Parser::formatErrorWithContext(const std::vector<Terminal>& tokens, 
                                                     int error_position, 
                                                     const std::string& error_message) {
    std::string result = error_message + "\n\n";
    
    // Mostrar contexto (los tokens alrededor del error)
    const int context_size = 5;  // Número de tokens a mostrar antes y después del error
    
    int start = std::max(0, error_position - context_size);
    int end = std::min(static_cast<int>(tokens.size()), error_position + context_size + 1);
    
    // Construir la línea con los tokens
    std::string tokens_line;
    for (int i = start; i < end; ++i) {
        std::string token_str = tokens[i].Name();
        tokens_line += token_str + " ";
    }
    
    // Construir la línea con el marcador de error
    std::string marker_line;
    int position = 0;
    for (int i = start; i < end; ++i) {
        std::string token_str = tokens[i].Name();
        
        if (i < error_position) {
            // Añadir espacios para alinear con los tokens anteriores
            for (size_t j = 0; j < token_str.length() + 1; ++j) {
                marker_line += " ";
            }
        } else if (i == error_position) {
            // Marcar el token erróneo
            marker_line += "^";
            for (size_t j = 1; j < token_str.length(); ++j) {
                marker_line += "~";
            }
            marker_line += " ";
        }
    }
    
    result += tokens_line + "\n" + marker_line + "\n";
    return result;
}

std::string SLR1Parser::formatErrorWithContext(const std::vector<Token>& tokens, 
                                              int error_position, 
                                              const std::string& error_message) {
    std::string result = error_message + "\n\n";
    
    // Mostrar contexto (los tokens alrededor del error)
    const int context_size = 5;  // Número de tokens a mostrar antes y después del error
    
    int start = std::max(0, error_position - context_size);
    int end = std::min(static_cast<int>(tokens.size()), error_position + context_size + 1);
    
    // Información de posición del error
    int error_line = -1;
    int error_column = -1;
    if (error_position < tokens.size()) {
        error_line = tokens[error_position].Line();
        error_column = tokens[error_position].Column();
    }
    
    result += "Línea " + std::to_string(error_line) + ", Columna " + std::to_string(error_column) + ":\n\n";
    
    // Construir la línea con los tokens y sus lexemas
    std::string tokens_line;
    for (int i = start; i < end; ++i) {
        std::string lexeme = tokens[i].Lexeme();
        if (lexeme.empty()) lexeme = tokens[i].Name(); // Si no hay lexema, usar el nombre
        tokens_line += lexeme + " ";
    }
    
    // Construir la línea con el marcador de error
    std::string marker_line;
    int position = 0;
    for (int i = start; i < end; ++i) {
        std::string lexeme = tokens[i].Lexeme();
        if (lexeme.empty()) lexeme = tokens[i].Name();
        
        if (i < error_position) {
            // Añadir espacios para alinear con los tokens anteriores
            for (size_t j = 0; j < lexeme.length() + 1; ++j) {
                marker_line += " ";
            }
        } else if (i == error_position) {
            // Marcar el token erróneo
            marker_line += "^";
            for (size_t j = 1; j < lexeme.length(); ++j) {
                marker_line += "~";
            }
            marker_line += " ";
        }
    }
    
    result += tokens_line + "\n" + marker_line + "\n";
    return result;
}

// Implementación completa del método Parse para objetos Token
std::pair<std::vector<int>, std::vector<std::string>> SLR1Parser::Parse(const std::vector<Token>& tokens) {
    std::vector<int> production_ids;
    std::vector<std::string> actions;
    
    // Initialize stack and state
    std::stack<int> state_stack;
    state_stack.push(0);
    std::stack<Token> symbol_stack;

    int index = 0;
    int current_position = 0;
    
    LogInfo("SLR1Parser: Analizando " + std::to_string(tokens.size()) + " tokens");
    
    
    if (verbose_) {
        LogInfo("Iniciando análisis sintáctico con " + std::to_string(tokens.size()) + " tokens");
        LogDebug("Lista de tokens a analizar:");
        for (size_t i = 0; i < tokens.size(); ++i) {
            LogDebug("  [" + std::to_string(i) + "] " + tokens[i].ToString() + 
                     " (línea " + std::to_string(tokens[i].Line()) + 
                     ", columna " + std::to_string(tokens[i].Column()) + 
                     ", lexema: '" + tokens[i].Lexeme() + "')");
        }
    }

    try {
        while (index < tokens.size() || !symbol_stack.empty()) {
            current_position = index;
            
            if (index < tokens.size()) {
                const Token& current_token = tokens[index];
                auto action_key = std::make_pair(state_stack.top(), current_token);

                if (action_.find(action_key) != action_.end()) {
                    auto action_value = action_[action_key];
                    
                    if (action_value.first == SHIFT) {
                        // Shift action
                        state_stack.push(action_value.second);
                        symbol_stack.push(current_token);
                        actions.push_back(SHIFT);

                        index++;
                    } 
                    else if (action_value.first == REDUCE) {
                        // Reduce action
                        auto production = G_.Productions()[action_value.second];
                        if (verbose_) {
                            LogDebug("REDUCE por producción: " + production.ToString() + 
                                    " (ID: " + std::to_string(production.get_id()) + ")");
                        }
                        production_ids.push_back(production.get_id());
                        actions.push_back(REDUCE);
                        
                        for (int i = 0; i < production.Right().Symbols().size(); i++) {
                            state_stack.pop();
                            if (!symbol_stack.empty()) {
                                symbol_stack.pop();
                            }
                        }

                        auto goto_key = std::make_pair(state_stack.top(), *(production.Left()));
                        if (goto_.find(goto_key) != goto_.end()) {
                            state_stack.push(goto_[goto_key]);
                        } 
                        else {
                            // Error interno del parser - no se encontró una transición goto
                            if (verbose_) {
                                LogError("Error interno: No se encontró transición GOTO para el no terminal '" + 
                                       production.Left()->Name() + "' en el estado " + std::to_string(state_stack.top()));
                            }
                            std::string error_msg = "Error interno del parser: no se encontró transición GOTO para el no terminal '" + 
                                                production.Left()->Name() + "' en el estado " + std::to_string(state_stack.top());
                            throw std::runtime_error(error_msg);
                        }
                    } 
                    else if (action_value.first == OK) {
                        // Accept action
                        if (verbose_) {
                            LogInfo("Análisis sintáctico completado con éxito (ACCEPT)");
                        }
                        actions.push_back(OK);
                        break;
                    } 
                    else {
                        throw std::runtime_error("Unknown action: " + action_value.first);
                    }
                } 
                else {
                    // No se encontró una acción para este estado y token
                    auto [error_msg, expected_tokens] = generateErrorMessage(state_stack.top(), current_token.Name());
                    
                    // Enriquecer el mensaje con información de posición
                    std::string position_info = " en línea " + std::to_string(current_token.Line()) + 
                                               ", columna " + std::to_string(current_token.Column());
                    
                    std::string enhanced_error = error_msg + position_info;
                    
                    // Lanzar ParsingError con toda la información
                    throw ParsingError(enhanced_error, state_stack.top(), current_token.Name(), expected_tokens);
                }
            } 
            else {
                // Si no quedan más tokens, comprobar si se puede reducir o aceptar
                bool found_action = false;
                
                for (const auto& [key, value] : action_) {
                    if (key.first == state_stack.top() && value.first == REDUCE) {
                        auto production = G_.Productions()[value.second];
                        production_ids.push_back(production.get_id());
                        actions.push_back(REDUCE);
                        
                        for (int i = 0; i < production.Right().Symbols().size(); i++) {
                            state_stack.pop();
                            if (!symbol_stack.empty()) {
                                symbol_stack.pop();
                            }
                        }
                        
                        auto goto_key = std::make_pair(state_stack.top(), *(production.Left()));
                        if (goto_.find(goto_key) != goto_.end()) {
                            state_stack.push(goto_[goto_key]);
                            found_action = true;
                            break;
                        }
                    } 
                    else if (key.first == state_stack.top() && value.first == OK && symbol_stack.empty()) {
                        actions.push_back(OK);
                        found_action = true;
                        break;
                    }
                }
                
                if (!found_action) {
                    // No se pudo reducir ni aceptar
                    auto expected_tokens = getExpectedTokens(state_stack.top());
                    std::string error_msg = "Error de sintaxis: fin de entrada inesperado";
                    throw ParsingError(error_msg, state_stack.top(), "EOF", expected_tokens);
                }
            }
        }
    } 
    catch (const ParsingError& e) {
        // Mejorar el mensaje de error con contexto visual
        std::string enhanced_message = e.what();
        enhanced_message += "\n\nContexto del error:";
        
        // Crear representación visual del contexto del error
        int position = current_position;
        int context_start = std::max(0, position - 3);
        int context_end = std::min(static_cast<int>(tokens.size()) - 1, position + 3);
        
        // Mostrar contexto con información de posición
        enhanced_message += "\n\n";
        for (int i = context_start; i <= context_end; ++i) {
            if (i == position) {
                enhanced_message += ">> " + tokens[i].ToString() + " <<\n";
            } else {
                enhanced_message += "   " + tokens[i].ToString() + "\n";
            }
        }
        
        throw ParsingError(enhanced_message, e.getState(), e.getToken(), e.getExpectedTokens());
    }
    
    return std::make_pair(production_ids, actions);
}