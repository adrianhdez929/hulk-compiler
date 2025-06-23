#include "LALR1Parser.h"
#include "../Automata/utils/ContainerSet.h"
#include <stack>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "../Lexer/Token.h"
#include "SLR1Parser.h" // Para acceder a la clase ParsingError

LALR1Parser::LALR1Parser(Grammar& G, bool verbose)
    : G_(G), verbose_(verbose) {
    G_.Augment(); // Asegurarse de que la gramática esté aumentada
    BuildParsingTable();
}

void LALR1Parser::BuildParsingTable() {
    ComputeFirstSets();
    BuildAutomaton();
    GenerateParsingTable();
}

//Parse method
std::pair<std::vector<int>, std::vector<std::string>> LALR1Parser::Parse(const std::vector<std::string>& tokens) {
    // Convert string tokens to Terminal objects
    std::vector<Terminal> terminal_tokens;
    for (const auto& token : tokens) {
        terminal_tokens.push_back(Terminal(token, G_));
    }
    return Parse(terminal_tokens);
}
std::pair<std::vector<int>, std::vector<std::string>> LALR1Parser::Parse(const std::vector<Terminal>& tokens) {
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
                    throw LALR1ParsingError(error_msg, state_stack.top(), current_token.Name(), expected_tokens);
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
                    throw LALR1ParsingError(error_msg, state_stack.top(), "EOF", expected_tokens);
                }
            }
        }
    } catch (const LALR1ParsingError& e) {
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

        throw LALR1ParsingError(enhanced_message, e.getState(), e.getToken(), e.getExpectedTokens());
    } catch (const std::exception& e) {
        // Re-lanzar excepciones normales
        throw;
    }
    
    return std::make_pair(production_ids, actions);
}

std::pair<std::vector<int>, std::vector<std::string>> LALR1Parser::Parse(const std::vector<Token>& tokens) {
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

                    // Lanzar LALR1ParsingError con toda la información
                    throw LALR1ParsingError(enhanced_error, state_stack.top(), current_token.Name(), expected_tokens);
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
                    throw LALR1ParsingError(error_msg, state_stack.top(), "EOF", expected_tokens);
                }
            }
        }
    } 
    catch (const LALR1ParsingError& e) {
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

        throw LALR1ParsingError(enhanced_message, e.getState(), e.getToken(), e.getExpectedTokens());
    }
    
    return std::make_pair(production_ids, actions);
}

void LALR1Parser::ComputeFirstSets() {

    for (const auto& terminal : G_.Terminals()) {
        firsts_sets_[terminal->Name()] = {terminal->Name()};
    }
    for (const auto& non_terminal : G_.NonTerminals()) {
        firsts_sets_[non_terminal->Name()] = {};
    }

    bool changed = true;
    while (changed) {
        changed = false;
        for (const auto& production : G_.Productions()) {
            const auto& left = production.Left()->Name();
            const auto& right = production.Right().Symbols();

            std::set<std::string> new_first;
            bool all_epsilon = true;

            for (const auto& symbol : right) {
                if (symbol->IsTerminal()) {
                    if (symbol->Name() != G_.GetEpsilon()->Name()) {
                        new_first.insert(symbol->Name());
                        all_epsilon = false;
                        break;
                    }
                } else {
                    const auto& first_set = firsts_sets_[symbol->Name()];
                    for (const auto& first_symbol : first_set) {
                        if (first_symbol != G_.GetEpsilon()->Name()) {
                            new_first.insert(first_symbol);
                        }
                    }

                    if (first.find(G_.GetEpsilon()->Name()) == first_set.end()) {
                        all_epsilon = false;
                        break;
                    }
                }
            }
            if (all_epsilon) {
                new_first.insert(G_.GetEpsilon()->Name());
            }

            size_t old_size = firsts_sets_[left].size();
            for (const auto& symbol : new_first) {
                firsts_sets_[left].insert(symbol);
            }
            if (firsts_sets_[left].size() > old_size) {
                changed = true;
            }
        }
    }
}

void LALR1Parser::BuildAutomaton() {
    assert(G_.GetStartSymbol()->productions.size() == 1 && "Grammar must be augmented");

    // Calcular conjuntos FIRST
    auto firsts = compute_firsts();
    auto EOFile = Sentence(G_.GetEndOfFile());
    firsts[EOFile] = ContainerSet<string>();
    firsts[EOFile].add(G_.GetEndOfFile()->Name());

    // Ítem inicial
    auto start_production = G_.GetStartSymbol()->productions[0];
    ContainerSet<string> lookahead_set;
    lookahead_set.add(G_.GetEndOfFile()->Name());
    auto start_item = Item(std::make_shared<Production>(start_production), 0, lookahead_set);

    // Calcular clausura inicial
    auto closure = closure_lr1({start_item}, firsts, G_);
    
    // Estado inicial
    int state_id = 0;
    auto automaton = State(state_id++, true);
    for (const auto& item : closure) {
        automaton.add_item(item);
    }

    // Almacenamiento de estados por núcleo
    // std::map<std::set<production->id(), item.pos>, State*> kernel_to_state;
    std::map<std::set<std::pair<int, int>>, State*> visited;
    // std::map<State*, std::set<ItemCore>> state_to_kernel;
    
    // Inicializar con el estado inicial
    std::set<std::pair<int, int>> initial_kernel = {{start_production.get_id(), 0}};
    visited[initial_kernel] = &automaton;
    // state_to_kernel[&automaton] = initial_kernel;

    // Cola para procesar estados
    std::queue<State*> pending;
    pending.push(&automaton);

    while (!pending.empty()) {
        auto current_state = pending.front();
        pending.pop();
        std::cout << "Procesando estado: " << current_state->get_id() << std::endl;
        // Items del estado
        for (const auto& item : current_state->get_items()) {
            std::cout << "  - " << item.ToString() << std::endl;
        }
        // Para cada símbolo en la gramática
        for (const auto& symbol : G_.Symbols()) {
            std::cout << "  - Calculando GOTO para símbolo: " << symbol->Name() << std::endl;
            // Calcular GOTO (solo kernel)
            //goto_lr1 aplica el goto y, si just_kernel es false, también calcula la clausura
            auto goto_items = goto_lr1(current_state->get_items(), symbol, firsts, true, G_);
            std::cout << "  - GOTO resultante: " << goto_items.size() << " ítems" << std::endl;
            for (const auto& item : goto_items) {
                std::cout << "    - " << item.ToString() << std::endl;
            }
            // std::vector<Item> goto_items;
            // for (const auto& item : current_state->get_items()) {
            //     if (item.NextSymbol() == symbol) {
            //         auto next_item = item.NextItem();
            //         if (next_item) {
            //             goto_items.push_back(*next_item);
            //         }
            //     }
            // }

            if (goto_items.empty()) continue;

            // Calcular clausura para el nuevo kernel
            auto new_closure = closure_lr1(goto_items, firsts, G_);
            std::cout << "  - Clausura resultante: " << new_closure.size() << " ítems" << std::endl;
            for (const auto& item : new_closure) {
                std::cout << "    - " << item.ToString() << std::endl;
            }

            std::set<std::pair<int, int>> new_kernel;
            for (const auto& item : new_closure) {
                new_kernel.insert({item.production()->get_id(), item.pos()});
            }
            State* new_state = nullptr;
            if (visited.find(new_kernel) != visited.end()) {
                // Crear lista con items de new_closure y del estado existente
                new_state = visited[new_kernel];
                std::cout << "  - Estado existente encontrado: " << new_state->get_id() << std::endl;
                // Mostrar los ítems del estado existente
                for (const auto& item : new_state->get_items()) {
                    std::cout << "    - Ítem existente: " << item.ToString() << std::endl;
                }
                // Unir los items de new_closure con los del estado existente
                std::vector<Item> combined_items = new_state->get_items();
                combined_items.insert(combined_items.end(), new_closure.begin(), new_closure.end());

                // Comprimir los ítems combinados
                auto compressed_items = compress(combined_items);
                new_state->set_items(std::vector<Item>(compressed_items.begin(), compressed_items.end()));
                std::cout << "  - Ítems combinados y comprimidos: " << new_state->get_items().size() << " ítems" << std::endl;
                for (const auto& item : new_state->get_items()) {
                    std::cout << "    - Ítem combinado: " << item.ToString() << std::endl;
                }
            } else {
                // Crear nuevo estado
                new_state = new State(state_id++, true);
                for (const auto& item : new_closure) {
                    new_state->add_item(item);
                }
                
                std::cout << "  - Creando nuevo estado: " << new_state->get_id() << std::endl;
                // Mostrar los ítems del nuevo estado
                for (const auto& item : new_state->get_items()) {
                    std::cout << "    - Ítem nuevo: " << item.ToString() << std::endl;
                }
                // Registrar nuevo estado
                visited[new_kernel] = new_state;
                pending.push(new_state);
                automaton_states_.push_back(new_state);// Esto es solo pa limpiar memoria cuando se borre el parser
            }
            
            // // Extraer el núcleo del nuevo estado
            // std::set<ItemCore> new_kernel;
            // for (const auto& item : new_closure) {
            //     new_kernel.insert({item.production(), item.pos()});
            // }
            
            // // Buscar si ya existe un estado con este núcleo
            // State* target_state = nullptr;
            // if (kernel_to_state.find(new_kernel) != kernel_to_state.end()) {
            //     // Estado existente: fusionar lookaheads
            //     target_state = kernel_to_state[new_kernel];
                
            //     // Fusionar lookaheads de los ítems correspondientes
            //     for (const auto& new_item : new_closure) {
            //         for (auto& existing_item : target_state->get_mutable_items()) {
            //             if (existing_item.production() == new_item.production() &&
            //                 existing_item.pos() == new_item.pos()) 
            //             {
            //                 existing_item.merge_lookaheads(new_item.lookaheads());
            //             }
            //         }
            //     }
            // } else {
            //     // Crear nuevo estado
            //     target_state = new State(state_id++, true);
            //     for (const auto& item : new_closure) {
            //         target_state->add_item(item);
            //     }
                
            //     // Registrar nuevo estado
            //     kernel_to_state[new_kernel] = target_state;
            //     state_to_kernel[target_state] = new_kernel;
            //     pending.push(target_state);
            //     automaton_states_.push_back(target_state);
            // }
            
            // Añadir transición
            current_state->add_transition(symbol->Name(), new_state);
        }
    }
    
    return automaton;
}
