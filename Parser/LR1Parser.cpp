#include "LR1Parser.h"
#include "../Automata/utils/ContainerSet.h"
#include <stack>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "../Lexer/Token.h"
#include "SLR1Parser.h" // Para acceder a la clase ParsingError

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
            std::cerr << "Error creando directorio hulk: " << e.what() << std::endl;
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
            std::cerr << "Error creando directorio " << directory << ": " << e.what() << std::endl;
            return false;
        }
    }
}

LR1Parser::LR1Parser(Grammar& G, bool verbose)
    : G_(G), verbose_(verbose) {
    // Initialize action and goto tables
    action_ = std::map<std::pair<int, Symbol>, std::pair<std::string, int>>();
    goto_ = std::map<std::pair<int, Symbol>, int>();
    BuildParsingTable();
}

//Parse method
std::pair<std::vector<int>, std::vector<std::string>> LR1Parser::Parse(const std::vector<std::string>& tokens) {
    // Convert string tokens to Terminal objects
    std::vector<Terminal> terminal_tokens;
    for (const auto& token : tokens) {
        terminal_tokens.push_back(Terminal(token, G_));
    }
    return Parse(terminal_tokens);
}
std::pair<std::vector<int>, std::vector<std::string>> LR1Parser::Parse(const std::vector<Terminal>& tokens) {
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
                        index++;
                    } else if (action_value.first == REDUCE) {
                        // Reduce action
                        auto production = G_.Productions()[action_value.second];
                        if (verbose_) {
                            cout << "Reducing by production: " << production.ToString() << endl;
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
                        } else {
                            // Error interno del parser - no se encontró una transición goto
                            if (verbose_) {
                                cout << "Actions:" << endl;
                                for (const auto& action : action_) {
                                    cout << "State: " << action.first.first << ", Token: " << action.first.second.Name() 
                                        << " -> Action: " << action.second.first << ", Value: " << action.second.second << endl;
                                }
                                cout << "Goto:" << endl;
                                for (const auto& goto_action : goto_) {
                                    cout << "State: " << goto_action.first.first << ", NonTerminal: " << goto_action.first.second.Name() 
                                        << " -> Goto State: " << goto_action.second << endl;
                                }
                            }
                            
                            // Este es un error interno del parser, probablemente debido a una gramática mal construida
                            std::string error_msg = "Error interno del parser: no se encontró transición GOTO para el no terminal '" + 
                                                production.Left()->Name() + "' en el estado " + std::to_string(state_stack.top());
                            throw std::runtime_error(error_msg);
                        }
                    } else if (action_value.first == OK) {
                        // Accept action
                        actions.push_back(OK);
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
                    if (verbose_) {
                        cout << "Actions:" << endl;
                        for (const auto& action : action_) {
                            cout << "State: " << action.first.first << ", Token: " << action.first.second.Name() 
                                << " -> Action: " << action.second.first << ", Value: " << action.second.second << endl;
                        }
                        cout << "Goto:" << endl;
                        for (const auto& goto_action : goto_) {
                            cout << "State: " << goto_action.first.first << ", NonTerminal: " << goto_action.first.second.Name() 
                                << " -> Goto State: " << goto_action.second << endl;
                        }
                    }
                    
                    // Lanzar una excepción especializada con detalles del error
                    throw LR1ParsingError(error_msg, state_stack.top(), current_token.Name(), expected_tokens);
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
                    throw LR1ParsingError(error_msg, state_stack.top(), "EOF", expected_tokens);
                }
            }
        }
    } catch (const LR1ParsingError& e) {
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
        
        throw LR1ParsingError(enhanced_message, e.getState(), e.getToken(), e.getExpectedTokens());
    } catch (const std::exception& e) {
        // Re-lanzar excepciones normales
        throw;
    }
    
    return std::make_pair(production_ids, actions);
}
void LR1Parser::BuildParsingTable() {
    G_.Augment();
    if (!G_.IsAugmented()) {
        throw std::runtime_error("Grammar is not augmented");
    }
    auto firsts = compute_firsts();

    // Debug: Print FIRST sets
    if (verbose_) {
        std::cout << "FIRST sets:" << std::endl;
        for (const auto& [nt, first_set] : firsts) {
            std::cout << "FIRST(" << nt.ToString() << ") = { ";
            for (const auto& t : first_set.get_values()) {
                std::cout << t << " ";
            }
            std::cout << "}" << std::endl;
        }
    }
    
    // auto EOFile = Sentence(G_.GetEndOfFile());
    // firsts[EOFile] = ContainerSet<string>().add(G_.GetEndOfFile()->Name());

    State* automaton = BuildLR1Automaton().to_deterministic();
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
                    auto lookaheads = item.lookaheads().get_values();
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
void LR1Parser::Register(std::map<std::pair<int, Symbol>, std::pair<std::string, int>>& table, 
                                 const std::pair<int, Symbol>& key, 
                                 const std::pair<std::string, int>& value) {
    if (verbose_) {
        std::cout << "Registering action: " << key.first << ", " << key.second.Name() << " -> " << value.first << ", " << value.second << std::endl;
    }
    table[key] = value;
}
void LR1Parser::Register(std::map<std::pair<int, Symbol>, int>& table, 
                                 const std::pair<int, Symbol>& key, 
                                 int value) {
    if (verbose_) {
        std::cout << "Registering goto: " << key.first << ", " << key.second.Name() << " -> " << value << std::endl;
    }
    table[key] = value;
}

State LR1Parser::BuildLR1Automaton() {
    assert(G_.GetStartSymbol()->productions.size() == 1 && "Grammar must be augmented");

    // Compute first sets
    auto firsts = compute_firsts();
    auto EOFile = Sentence(G_.GetEndOfFile());
    firsts[EOFile] = ContainerSet<string>();
    firsts[EOFile].add(G_.GetEndOfFile()->Name());

    // Initialize start production and item
    auto start_production = G_.GetStartSymbol()->productions[0];
    ContainerSet<string> lookahead_set;
    lookahead_set.add(G_.GetEndOfFile()->Name());
    auto start_item = Item(std::make_shared<Production>(start_production), 0, lookahead_set);
    std::vector<Item> start = {start_item};

    // Compute closure for the start state
    auto closure = closure_lr1(start, firsts, G_);
    int state_id = 0;
    auto automaton = State(state_id++, true);
    
    // Añadir los ítems al estado inicial
    for (const auto& item : closure) {
        automaton.add_item(item);
    }

    // Map to associate states with their items
    std::map<std::set<Item>, State*> visited;
    visited[std::set<Item>(closure.begin(), closure.end())] = &automaton;

    // Queue for pending states
    std::queue<std::set<Item>> pending;
    pending.push(std::set<Item>(closure.begin(), closure.end()));

    while (!pending.empty()) {
        auto current = pending.front();
        pending.pop();
        auto current_state = visited[current];

        for (const auto& symbol : G_.Terminals()) {
            auto next_items = goto_lr1(std::vector<Item>(current.begin(), current.end()), symbol, firsts, true, G_);
            if (next_items.empty()) {
                continue;
            }

            std::set<Item> next_set(next_items.begin(), next_items.end());
            State* next_state;

            if (visited.find(next_set) == visited.end()) {
                auto next_closure = closure_lr1(next_items, firsts, G_);
                next_state = new State(state_id++, true);
                for (const auto& item : next_closure) {
                    next_state->add_item(item);
                }
                visited[next_set] = next_state;
                pending.push(next_set);
            } else {
                next_state = visited[next_set];
            }

            current_state->add_transition(symbol->Name(), next_state);
        }

        for (const auto& symbol : G_.NonTerminals()) {
            auto next_items = goto_lr1(std::vector<Item>(current.begin(), current.end()), symbol, firsts, true, G_);
            if (next_items.empty()) {
                continue;
            }

            std::set<Item> next_set(next_items.begin(), next_items.end());
            State* next_state;

            if (visited.find(next_set) == visited.end()) {
                auto next_closure = closure_lr1(next_items, firsts, G_);
                next_state = new State(state_id++, true);
                for (const auto& item : next_closure) {
                    next_state->add_item(item);
                }
                visited[next_set] = next_state;
                pending.push(next_set);
            } else {
                next_state = visited[next_set];
            }

            current_state->add_transition(symbol->Name(), next_state);
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


map<Sentence, ContainerSet<string>> LR1Parser::compute_firsts() {
    if (verbose_) {
        std::cout << "===== LR1Parser::compute_firsts() =====" << std::endl;
        std::cout << "Inicializando estructuras para cálculo de FIRST..." << std::endl;
    }
    
    map<Sentence, ContainerSet<string>> firsts;
    bool changed = true;
    
    // Inicializar primeros para terminales
    if (verbose_) {
        std::cout << "Inicializando FIRST para terminales..." << std::endl;
        std::cout << "Total terminales: " << G_.Terminals().size() << std::endl;
    }
    
    int terminal_count = 0;
    for (const auto& terminal : G_.Terminals()) {
        if (verbose_) {
            std::cout << "Procesando terminal #" << terminal_count << ": '" << terminal->Name() << "'";
            std::cout << (terminal->IsEndOfFile() ? " (EOF)" : "") << std::endl;
        }
        
        terminal_count++;
        if (terminal->IsEndOfFile()) {
            // For EOF terminal, add its own name to its FIRST set
            ContainerSet<string> eof_set;
            eof_set.add(terminal->Name());
            firsts[Sentence(terminal)] = eof_set;

            if (verbose_) {
                std::cout << "  FIRST(EOF) = {" << terminal->Name() << "}" << std::endl;
            }
            continue;
        }
        ContainerSet<string> cs;
        cs.add(terminal->Name());
        firsts[Sentence(terminal)] = cs;

        if (verbose_) {
            std::cout << "  FIRST(" << terminal->Name() << ") = {" << terminal->Name() << "}" << std::endl;
        }
    }
    
    // Ensure EOF is in firsts map
    auto eof = G_.GetEndOfFile();
    if (eof) {
        Sentence eof_sent(eof);
        if (firsts.find(eof_sent) == firsts.end()) {
            ContainerSet<string> eof_set;
            eof_set.add(eof->Name());
            firsts[eof_sent] = eof_set;

            if (verbose_) {
                std::cout << "Agregando EOF faltante: FIRST(EOF) = {" << eof->Name() << "}" << std::endl;
            }
        }
    } else if (verbose_) {
        std::cout << "¡ADVERTENCIA: No se encontró símbolo EOF en la gramática!" << std::endl;
    }
    
    // Inicializar primeros para no terminales
    if (verbose_) {
        std::cout << "Inicializando FIRST para no terminales..." << std::endl;
        std::cout << "Total no terminales: " << G_.NonTerminals().size() << std::endl;
    }
    
    int nonterminal_count = 0;
    for (const auto& nonterminal : G_.NonTerminals()) {
        if (verbose_) {
            std::cout << "Inicializando FIRST(" << nonterminal->Name() << ") = {}" << std::endl;
        }
        firsts[Sentence(nonterminal)] = ContainerSet<string>();
        nonterminal_count++;
    }

    // Inicializar FIRST para las partes derechas de las producciones
    if (verbose_) {
        std::cout << "Inicializando FIRST para las partes derechas de todas las producciones..." << std::endl;
        std::cout << "Total producciones: " << G_.Productions().size() << std::endl;
    }
    
    // for (const auto& prod : G_.Productions()) {
    //     auto right = prod.Right();
    //     firsts[right] = ContainerSet<string>();
    // }
    
    int iteration = 0;
    while (changed == true) {
        iteration++;
        if (verbose_) {
            std::cout << "\n==== Iteración #" << iteration << " del algoritmo FIRST ====" << std::endl;
        }
        changed = false;

        int prod_count = 0;
        for (const auto& prod : G_.Productions()) {
            prod_count++;
            const auto& X = Sentence(prod.Left());
            const auto& alpha = prod.Right();

            if (verbose_) {
                std::cout << "Procesando producción #" << prod_count << ": " << prod.ToString() << std::endl;
            }

            // Firsts de X
            auto& first_X = firsts[X];

            // Firsts de alpha
            auto& first_alpha = firsts[alpha];

            if (verbose_) {
                std::cout << "  Calculando FIRST local para el lado derecho: " << alpha.ToString() << std::endl;
            }
            ContainerSet<string> local_first = compute_local_firsts(alpha, firsts, G_, verbose_);

            if (verbose_) {
                std::cout << "  FIRST local calculado: { ";
                for (const auto& terminal : local_first.get_values()) {
                    std::cout << terminal << " ";
                }
                if (local_first.contains_epsilon()) {
                    std::cout << "epsilon ";
                }
                std::cout << "}" << std::endl;
            }

            bool changed_alpha = first_alpha.hard_update(local_first);
            bool changed_X = first_X.hard_update(local_first);

            if (verbose_ && (changed_alpha || changed_X)) {
                std::cout << "  ¡Cambio detectado!" << std::endl;
                if (changed_alpha) {
                    std::cout << "    - FIRST(" << alpha.ToString() << ") actualizado" << std::endl;
                }
                if (changed_X) {
                    std::cout << "    - FIRST(" << X.ToString() << ") actualizado" << std::endl;
                }
            }
            
            changed = changed || changed_alpha || changed_X;
        }

        if (verbose_) {
            if (!changed) {
                std::cout << "No se detectaron cambios en esta iteración. ¡Algoritmo FIRST convergió!" << std::endl;
            } else {
                std::cout << "Se detectaron cambios. Continuando con la siguiente iteración..." << std::endl;
            }
        }
    }

    if (verbose_) {
        std::cout << "\n==== Conjuntos FIRST finales ====" << std::endl;
        for (const auto& [sentence, first_set] : firsts) {
            std::cout << "FIRST(" << sentence.ToString() << ") = { ";
            for (const auto& terminal : first_set.get_values()) {
                std::cout << terminal << " ";
            }
            if (first_set.contains_epsilon()) {
                std::cout << "epsilon ";
            }
            std::cout << "}" << std::endl;
        }
        std::cout << "===== Fin de LR1Parser::compute_firsts() =====" << std::endl;
    }
    
    return firsts;
};

ContainerSet<string> LR1Parser::compute_local_firsts(const Sentence& alpha, const map<Sentence, ContainerSet<string>>& firsts, const Grammar& G, bool verbose) {
    if (verbose) {
        std::cout << "    compute_local_firsts para: \"" << alpha.ToString() << "\"" << std::endl;
    }
    
    //Compute local first
    ContainerSet<string> local_first = ContainerSet<string>();
    auto symbols = alpha.Symbols();
    
    // If alpha is empty (epsilon), set epsilon flag
    if (symbols.empty()) {
        local_first.set_epsilon();
        if (verbose) {
            std::cout << "      Alpha está vacía, retornando {epsilon}" << std::endl;
        }
        return local_first;
    }
    
    // If alpha is explicitly epsilon, add epsilon to local first
    bool alpha_is_epsilon = false;
    for (const auto& symbol : symbols) {
        if (symbol->IsEpsilon()) {
            alpha_is_epsilon = true;
            if (verbose) {
                std::cout << "      Alpha contiene explícitamente epsilon" << std::endl;
            }
            break;
        }
    }
    
    if (alpha_is_epsilon) {
        local_first.set_epsilon();
        if (verbose) {
            std::cout << "      Alpha es epsilon, retornando {epsilon}" << std::endl;
        }
        return local_first;
    }
    
    // Check if all symbols can derive epsilon
    if (verbose) {
        std::cout << "      Verificando si todos los símbolos pueden derivar epsilon..." << std::endl;
    }
    
    bool all_epsilon = true;
    for (const auto& symbol : symbols) {
        Sentence sym_sent(symbol);
        if (firsts.find(sym_sent) == firsts.end() || !firsts.at(sym_sent).contains_epsilon()) {
            all_epsilon = false;
            if (verbose) {
                std::cout << "        " << symbol->Name() << " no puede derivar epsilon" << std::endl;
            }
            break;
        } else if (verbose) {
            std::cout << "        " << symbol->Name() << " puede derivar epsilon" << std::endl;
        }
    }
    
    if (all_epsilon) {
        local_first.set_epsilon();
        if (verbose) {
            std::cout << "      Todos los símbolos pueden derivar epsilon, agregando epsilon al resultado" << std::endl;
        }
    }
    
    // Calculate FIRST for the string
    if (verbose) {
        std::cout << "      Calculando FIRST para la cadena de símbolos..." << std::endl;
    }
    
    for (size_t i = 0; i < symbols.size(); ++i) {
        const auto& symbol = symbols[i];
        Sentence sym_sent(symbol);
        
        if (verbose) {
            std::cout << "        Procesando símbolo #" << (i+1) << ": " << symbol->Name();
            if (symbol->IsTerminal()) {
                std::cout << " (Terminal)";
            } else if (symbol->IsNonTerminal()) {
                std::cout << " (No Terminal)";
            } else if (symbol->IsEndOfFile()) {
                std::cout << " (EOF)";
            }
            std::cout << std::endl;
        }
        
        if (symbol->IsEndOfFile()) {
            local_first.add(G.GetEndOfFile()->Name());
            if (verbose) {
                std::cout << "          Es EOF, agregando '" << G.GetEndOfFile()->Name() << "' y terminando" << std::endl;
            }
            break;
        }
        
        if (firsts.find(sym_sent) != firsts.end()) {
            const auto& first_set = firsts.at(sym_sent);
            if (verbose) {
                std::cout << "          FIRST(" << symbol->Name() << ") = { ";
                for (const auto& val : first_set.get_values()) {
                    std::cout << val << " ";
                }
                if (first_set.contains_epsilon()) {
                    std::cout << "epsilon ";
                }
                std::cout << "}" << std::endl;
            }
            
            // Add all non-epsilon terminals
            for (const auto& val : first_set.get_values()) {
                // We only need to check if the symbol name is the epsilon symbol name
                if (val != G.GetEpsilon()->Name()) {
                    local_first.add(val);
                    if (verbose) {
                        std::cout << "          Agregando '" << val << "' al resultado" << std::endl;
                    }
                }
            }
            
            // If the symbol doesn't derive epsilon, stop processing
            if (!first_set.contains_epsilon()) {
                if (verbose) {
                    std::cout << "          Este símbolo no deriva epsilon, terminando el proceso" << std::endl;
                }
                break;
            }
            
            // If it's the last symbol and it can derive epsilon, add epsilon to result
            if (i == symbols.size() - 1) {
                local_first.set_epsilon();
                if (verbose) {
                    std::cout << "          Es el último símbolo y puede derivar epsilon, agregando epsilon al resultado" << std::endl;
                }
            } else if (verbose) {
                std::cout << "          Este símbolo puede derivar epsilon, continuando con el siguiente" << std::endl;
            }
        } else {
            // Symbol not in firsts, it must be a terminal
            local_first.add(symbol->Name());
            if (verbose) {
                std::cout << "          Símbolo no encontrado en firsts, asumiendo que es terminal y agregando '" << symbol->Name() << "'" << std::endl;
            }
            break;
        }
    }
    
    if (verbose) {
        std::cout << "      FIRST local calculado: { ";
        for (const auto& val : local_first.get_values()) {
            std::cout << val << " ";
        }
        if (local_first.contains_epsilon()) {
            std::cout << "epsilon ";
        }
        std::cout << "}" << std::endl;
    }
    
    return local_first;
}
// ContainerSet<string> LR1Parser::compute_local_firsts(const Sentence& alpha, const map<Sentence, ContainerSet<string>>& firsts, const Grammar& G) {
//     ContainerSet<string> local_first;
//     auto symbols = alpha.Symbols();

//     // Caso especial: cadena vacía
//     if (symbols.empty()) {
//         local_first.set_epsilon();
//         return local_first;
//     }

//     // Verificar si todos los símbolos pueden ser epsilon
//     bool all_epsilon = true;
//     for (const auto& symbol : symbols) {
//         Sentence sym_sent(symbol);
//         if (firsts.find(sym_sent) == firsts.end() || !firsts.at(sym_sent).contains_epsilon()) {
//             all_epsilon = false;
//             break;
//         }
//     }
//     if (all_epsilon) {
//         local_first.set_epsilon();
//         return local_first;
//     }

//     // Calcular FIRST para la cadena
//     for (size_t i = 0; i < symbols.size(); ++i) {
//         const auto& symbol = symbols[i];
//         Sentence sym_sent(symbol);

//         if (symbol->IsEndOfFile()) {
//             local_first.add(G.GetEndOfFile()->Name());
//             break;
//         }

//         if (firsts.find(sym_sent) != firsts.end()) {
//             const auto& first_set = firsts.at(sym_sent);
            
//             // Añadir todos los terminales no-épsilon
//             for (const auto& val : first_set.get_values()) {
//                 if (val != G.GetEpsilon()->Name()) {
//                     local_first.add(val);
//                 }
//             }

//             // Si el símbolo no deriva épsilon, terminar
//             if (!first_set.contains_epsilon()) {
//                 break;
//             }
            
//             // Si es el último símbolo y todos derivan épsilon, añadir épsilon
//             if (i == symbols.size() - 1) {
//                 local_first.set_epsilon();
//             }
//         } else {
//             // Si el símbolo no está en firsts, es un terminal
//             local_first.add(symbol->Name());
//             break;
//         }
//     }

//     return local_first;
// }

std::map<Sentence, ContainerSet<string>> LR1Parser::compute_follows(const map<Sentence, ContainerSet<string>>& symbol_firsts) {
    std::map<Sentence, ContainerSet<string>> follows;
    bool changed = true;

    for (const auto& nonterminal : G_.NonTerminals()) {
        follows[Sentence(nonterminal)] = ContainerSet<string>();
    }
    Sentence start_sentence = Sentence(G_.GetStartSymbol());
    shared_ptr<Symbol> EOFile = G_.GetEndOfFile();
    follows[start_sentence] = ContainerSet<string>();
    follows[start_sentence].add(EOFile->Name());

    while (changed == true) {
        changed = false;
        auto prods = G_.Productions();
        for (const auto& prod : G_.Productions()) {
            const auto& X = Sentence(prod.Left());
            const auto& alpha = prod.Right();

            auto& follows_x = follows[X];

            // If alpha is epsilon, add epsilon to local first
            for (const auto& symbol : alpha.Symbols()) {
                if (symbol->IsEpsilon()) {
                    continue;
                }
            }
            
            
            int n = alpha.Symbols().size() - 1;
            if (n == 0) {
                auto& current_symbol = alpha.Symbols()[0];
                if (current_symbol->IsNonTerminal()) {
                    // changed |= update_container_set(follows[current_symbol], follows_x);
                    changed |= follows[Sentence(current_symbol)].update(follows_x);
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
                            changed |= follows[Y_sentence].update(symbol_firsts.at(beta_sentence));
                            // changed |= update_container_set(follows[Y], symbol_firsts.at(beta));
                            if (symbol_firsts.at(beta_sentence).contains_epsilon()) {
                                changed |= follows[Y_sentence].update(follows_x);
                                // changed |= update_container_set(follows[Y], follows_x);
                            }
                        }
                    }
                    if (i == n-1 && beta->IsNonTerminal()) {
                        // changed |= update_container_set(follows.at(beta_sentence), follows_x);
                        changed |= follows[beta_sentence].update(follows_x);
                    }
                    
                    // auto& current_follow = follows[current_symbol]; // Remove or comment out if current_symbol is undefined
                }
            }
        }
    }
    return follows;
}

static vector<std::shared_ptr<Symbol>> get_symbols(vector<string> symbols, Grammar& G_) {
    vector<std::shared_ptr<Symbol>> result;
    for (const auto& symbol : symbols) {
        result.push_back(G_.GetSymbol(symbol));
    }
    return result;
}

vector<Item> expand(const Item& item, const map<Sentence, ContainerSet<string>>& firsts, Grammar& G_) {
    vector<Item> expanded;
    const auto& next_symbol = item.NextSymbol();
    if (next_symbol == nullptr || !next_symbol->IsNonTerminal()) {
        return expanded;
    }
    auto lookaheads = ContainerSet<string>();
    for (const auto& preview : item.Preview()) {
        // lookaheads.update(compute_local_firsts(Sentence(preview), firsts));
        auto local_first = LR1Parser::compute_local_firsts(Sentence(get_symbols(preview, G_)), firsts, G_, false); // No verbose here
        // update_container_set(lookaheads, local_first);
        lookaheads.update(local_first);
    }
    if (lookaheads.contains_epsilon()) {
        throw std::runtime_error("Epsilon in lookaheads");
    }
    for (auto& prod : G_.Productions()) {
        if (prod.Left()->Name() == next_symbol->Name()) {
            auto prod_ptr = std::make_shared<Production>(prod);
            expanded.push_back(Item(prod_ptr, 0, lookaheads));
            
            // Añadimos debug para verificar si hay elementos duplicados
            // std::cout << "Expandido: " << prod_ptr->ToString() << ", pos: 0, LA: " << lookaheads.str() << std::endl;
        }
    }
    return expanded;
}
set<Item> compress(const vector<Item>& items) {
    map<pair<string, int>, pair<shared_ptr<Production>, ContainerSet<string>>> centers;
    for (const auto& item : items) {
        auto key = make_pair(item.production()->ToString(), item.pos());
        if (centers.find(key) == centers.end()) {
            centers[key] = make_pair(item.production(), item.lookaheads());
        } else {
            // hard_update_container_set(centers[key].second, item.lookaheads());
            centers[key].second.update(item.lookaheads());
        }
        
    }
    set<Item> compressed;
    for (const auto& [key, lookaheads] : centers) {
        compressed.insert(Item(centers[key].first, key.second, centers[key].second));
    }
    return compressed;
}

/// @brief Cierra un conjunto de ítems LR(1)
/// @param items El conjunto de ítems
/// @param firsts Los conjuntos FIRST
/// @param G_ La gramática
/// @return El conjunto de cierre
std::vector<Item> closure_lr1(const std::vector<Item>& items, const std::map<Sentence, ContainerSet<string>>& firsts, Grammar& G_) {
    // Inicializar el conjunto de cierre con los elementos iniciales
    ContainerSet<Item> closure;
    closure.add(items);

    bool changed = true;
    while (changed) {
        changed = false;

        // Crear un nuevo conjunto para los nuevos elementos
        ContainerSet<Item> new_items;
        for (const auto& item : closure) {
            // string item_str = item.production()->ToString() + " pos: " + std::to_string(item.pos());
            auto lookaheads = item.lookaheads();
            // Expandir cada elemento y agregar los nuevos elementos al conjunto
            auto expanded_items = expand(item, firsts, G_);
            new_items.add(expanded_items);
        }

        changed = closure.update(new_items);

        // // Agregar los nuevos elementos al cierre si no están ya presentes
        // for (const auto& new_item : new_items) {
        //     bool found = false;
        //     for (const auto& existing_item : closure) {
        //         // Comparamos manualmente usando los valores, no los punteros
        //         if (compare_items(existing_item, new_item)) {
        //             found = true;
        //             break;
        //         }
        //     }
            
        //     if (!found) {
        //         closure.push_back(new_item);
        //         changed = true;
        //     }
        // }
    }
    auto closure_items = closure.get_set();
    vector<Item> closure_items_vector(closure_items.begin(), closure_items.end());
    auto compressed_set = compress(closure_items_vector);
    return std::vector<Item>(compressed_set.begin(), compressed_set.end());
} 
vector<Item> goto_lr1(const vector<Item>& items, shared_ptr<Symbol> symbol, const map<Sentence, ContainerSet<string>>& firsts, bool just_kernel, Grammar& G_) {
    vector<Item> goto_items;
    for (const auto& item : items) {
        if (item.NextSymbol() == symbol) {
            auto next_item = item.NextItem();
            if (next_item != nullptr) {
                goto_items.push_back(*next_item);
            }
        }
    }
    if (just_kernel) {
        return goto_items;
    }
    return closure_lr1(goto_items, firsts, G_);
}

// Método para limpiar todos los estados creados
void LR1Parser::CleanupAutomatonStates() {
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
LR1Parser::~LR1Parser() {
    CleanupAutomatonStates();
}

// ============= IMPLEMENTACIÓN DE SERIALIZACIÓN DEL PARSER =============

// Constructor privado para deserialización
LR1Parser::LR1Parser(Grammar& G, 
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

bool LR1Parser::serialize_parser(const std::string& filename) const {
    if (!ensure_hulk_directory()) {
        return false;
    }
    return serialize_parser(filename, "hulk");
}

bool LR1Parser::serialize_parser(const std::string& filename, const std::string& directory) const {
    if (!ensure_directory(directory)) {
        return false;
    }
    
    std::string filepath = get_custom_path(filename, directory);
    std::ofstream file(filepath, std::ios::binary);
    
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo para escritura: " << filepath << std::endl;
        return false;
    }
    
    try {
        // Escribir firma del archivo
        const char* signature = "LR1PARSER";
        file.write(signature, 10);
        
        // Escribir versión
        uint32_t version = 1;
        file.write(reinterpret_cast<const char*>(&version), sizeof(version));
        
        // Escribir flag verbose
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
        std::cout << "Parser serializado exitosamente en: " << filepath << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error durante la serialización del parser: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

LR1Parser* LR1Parser::deserialize_parser(const std::string& filename, Grammar& grammar) {
    return deserialize_parser(filename, "hulk", grammar);
}

LR1Parser* LR1Parser::deserialize_parser(const std::string& filename, const std::string& directory, Grammar& grammar) {
    std::string filepath = get_custom_path(filename, directory);
    std::ifstream file(filepath, std::ios::binary);
    
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo para lectura: " << filepath << std::endl;
        return nullptr;
    }
    
    try {
        // Verificar firma
        char signature[11] = {0};
        file.read(signature, 10);
        if (std::string(signature) != "LR1PARSER") {
            std::cerr << "Error: Archivo no es un parser serializado válido" << std::endl;
            file.close();
            return nullptr;
        }
        
        // Leer versión
        uint32_t version;
        file.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (version != 1) {
            std::cerr << "Error: Versión de parser no soportada: " << version << std::endl;
            file.close();
            return nullptr;
        }
        
        // Leer flag verbose
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
        std::cout << "Parser deserializado exitosamente desde: " << filepath << std::endl;
        
        // Crear nuevo parser con las tablas deserializadas
        return new LR1Parser(grammar, action, goto_table, verbose);
        
    } catch (const std::exception& e) {
        std::cerr << "Error durante la deserialización del parser: " << e.what() << std::endl;
        file.close();
        return nullptr;
    }
}

std::vector<std::string> LR1Parser::getExpectedTokens(int state_id) const {
    std::vector<std::string> expected_tokens;
    
    // Buscar todas las acciones válidas para este estado
    for (const auto& [key, value] : action_) {
        if (key.first == state_id) {
            expected_tokens.push_back(key.second.Name());
        }
    }
    
    return expected_tokens;
}

std::pair<std::string, std::vector<std::string>> LR1Parser::generateErrorMessage(int state_id, const std::string& token) const {
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

std::string LR1Parser::formatErrorWithContext(const std::vector<Terminal>& tokens, 
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

std::string LR1Parser::formatErrorWithContext(const std::vector<Token>& tokens, 
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
std::pair<std::vector<int>, std::vector<std::string>> LR1Parser::Parse(const std::vector<Token>& tokens) {
    std::vector<int> production_ids;
    std::vector<std::string> actions;
    
    // Initialize stack and state
    std::stack<int> state_stack;
    state_stack.push(0);
    std::stack<Token> symbol_stack;

    int index = 0;
    int current_position = 0;

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
                            std::cout << "Reducing by production: " << production.ToString() << std::endl;
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
                                std::cout << "Error: No se encontró transición GOTO" << std::endl;
                            }
                            
                            std::string error_msg = "Error interno del parser: no se encontró transición GOTO para el no terminal '" + 
                                                production.Left()->Name() + "' en el estado " + std::to_string(state_stack.top());
                            throw std::runtime_error(error_msg);
                        }
                    } 
                    else if (action_value.first == OK) {
                        // Accept action
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
                    
                    // Lanzar LR1ParsingError con toda la información
                    throw LR1ParsingError(enhanced_error, state_stack.top(), current_token.Name(), expected_tokens);
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
                    throw LR1ParsingError(error_msg, state_stack.top(), "EOF", expected_tokens);
                }
            }
        }
    } 
    catch (const LR1ParsingError& e) {
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
        
        throw LR1ParsingError(enhanced_message, e.getState(), e.getToken(), e.getExpectedTokens());
    }
    
    return std::make_pair(production_ids, actions);
}