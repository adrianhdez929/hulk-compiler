#include "LALR1Parser.h"
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

LALR1Parser::LALR1Parser(Grammar& G, bool verbose)
    : G_(G), verbose_(verbose) {
    // Initialize action and goto tables
    action_ = std::map<std::pair<int, Symbol>, std::pair<std::string, int>>();
    goto_ = std::map<std::pair<int, Symbol>, int>();
    
    if (verbose_) {
        LogInfo("Inicializando parser LALR1 con gramática de " + std::to_string(G_.Productions().size()) + " producciones");
        LogDebug("Detalle de la gramática:");
        for (const auto& prod : G_.Productions()) {
            LogDebug("  " + prod.ToString() + " (ID: " + std::to_string(prod.get_id()) + ")");
        }
    } else {
        LogInfo("Inicializando parser LALR1");
    }
    
    BuildParsingTable();
    
    if (verbose_) {
        LogInfo("Tablas de análisis LALR1 creadas: " + std::to_string(action_.size()) + " entradas en ACTION, " 
                + std::to_string(goto_.size()) + " entradas en GOTO");
    }
}

//Parse method
std::pair<std::vector<int>, std::vector<std::string>> LALR1Parser::Parse(const std::vector<std::string>& tokens) {
    // Convert string tokens to Terminal objects
    std::vector<Terminal> terminal_tokens;
    
    LogInfo("Iniciando análisis sintáctico LALR1 con " + std::to_string(tokens.size()) + " tokens");
    
    if (verbose_) {
        LogDebug("Lista de tokens recibidos por el parser LALR1:");
        for (size_t i = 0; i < tokens.size(); i++) {
            LogDebug("[" + std::to_string(i) + "] Token string: " + tokens[i]);
        }
    }
    
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
                    
                    if (verbose_) {

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
void LALR1Parser::BuildParsingTable() {
    LogInfo("Construyendo tabla de análisis sintáctico LALR1");
    
    G_.Augment();
    if (!G_.IsAugmented()) {
        LogError("Error: La gramática no se pudo aumentar correctamente");
        throw std::runtime_error("Grammar is not augmented");
    }
    
    if (verbose_) {
        LogDebug("Gramática aumentada correctamente");
        LogDebug("Símbolo inicial: " + G_.GetStartSymbol()->Name());
    }
    
    LogDebug("Calculando conjuntos FIRST para construcción de autómata LALR1");
    auto firsts = compute_firsts();
    // Los conjuntos FIRST ya están registrados dentro de compute_firsts()

    LogDebug("Construyendo autómata LALR1");
    State* automaton = BuildLALR1Automaton().to_deterministic();
    
    if (verbose_) {
        LogDebug("Autómata LALR1 construido con " + std::to_string(automaton->get_all_states().size()) + " estados");
    }
    
    LogDebug("Generando tablas ACTION y GOTO a partir del autómata");
    for (const auto& state : automaton->get_all_states()) {
        int state_id = state->id();
        
        if (verbose_) {
            LogDebug("Procesando estado " + std::to_string(state_id) + " con " + 
                    std::to_string(state->get_items().size()) + " items");
            
            for (const auto& item : state->get_items()) {
                std::string dot_position = "";
                // En LALR1Parser usamos NextSymbol() en lugar de position()
                auto next_symbol = item.NextSymbol();
                if (next_symbol) {
                    dot_position = " (• antes de " + next_symbol->Name() + ")";
                } else {
                    dot_position = " (• al final)";
                }
                
                LogDebug("  Item: " + item.production()->ToString() + dot_position);
                
                if (!item.lookaheads().get_values().empty()) {
                    std::string lookaheads = "Lookaheads: {";
                    bool first = true;
                    for (const auto& look : item.lookaheads().get_values()) {
                        if (!first) lookaheads += ", ";
                        lookaheads += look;
                        first = false;
                    }
                    lookaheads += "}";
                    LogDebug("    " + lookaheads);
                }
            }
        }
        
        for (const auto& item : state->get_items()) {
            if (item.IsReduceItem()) {
                // Reduce action
                auto production = item.production();
                if (production->Left() == G_.GetStartSymbol()) {
                    // Accept action
                    Register(action_, {state_id, *(G_.GetEndOfFile())}, {OK, 0});
                    
                    if (verbose_) {
                        LogDebug("Estado " + std::to_string(state_id) + ": ACCEPT con EOF");
                    }
                } else {
                    // Regular reduce action
                    auto lookaheads = item.lookaheads().get_values();
                    
                    if (verbose_ && !lookaheads.empty()) {
                        LogDebug("Estado " + std::to_string(state_id) + ": REDUCE por " + 
                                production->ToString() + " con lookaheads: " + 
                                std::to_string(lookaheads.size()));
                    }
                    
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
                        int target_state = (*next_state.begin())->id();
                        Register(action_, {state_id, *next_symbol}, {SHIFT, target_state});
                        
                        if (verbose_) {
                            LogDebug("Estado " + std::to_string(state_id) + ": SHIFT con " + 
                                    next_symbol->Name() + " al estado " + std::to_string(target_state));
                        }
                    } else if (verbose_) {
                        LogDebug("Estado " + std::to_string(state_id) + ": No hay estado siguiente para " + 
                                next_symbol->Name() + " (terminal)");
                    }
                } else if (next_symbol && next_symbol->IsNonTerminal()) {
                    // Goto action
                    auto next_state = state->move(next_symbol->Name());
                    if (!next_state.empty()) {
                        int target_state = (*next_state.begin())->id();
                        Register(goto_, {state_id, *next_symbol}, target_state);
                        
                        if (verbose_) {
                            LogDebug("Estado " + std::to_string(state_id) + ": GOTO con " + 
                                    next_symbol->Name() + " al estado " + std::to_string(target_state));
                        }
                    } else if (verbose_) {
                        LogDebug("Estado " + std::to_string(state_id) + ": No hay estado siguiente para " + 
                                next_symbol->Name() + " (no terminal)");
                    }
                }
            }
        }
    }
}
void LALR1Parser::Register(std::map<std::pair<int, Symbol>, std::pair<std::string, int>>& table, 
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
void LALR1Parser::Register(std::map<std::pair<int, Symbol>, int>& table, 
                                 const std::pair<int, Symbol>& key, 
                                 int value) {
    if (verbose_) {
        LogDebug("GOTO[" + std::to_string(key.first) + ", " + key.second.Name() + "] = " + std::to_string(value));
    }
    table[key] = value;
}

map<Sentence, ContainerSet<string>> LALR1Parser::compute_firsts() {
    map<Sentence, ContainerSet<string>> firsts;
    bool changed = true;
    
    // Inicializar primeros para terminales
    for (const auto& terminal : G_.Terminals()) {
        if (terminal->IsEndOfFile()) {
            continue; // Skip EndOfFile terminal
        }
        ContainerSet<string> cs;
        cs.add(terminal->Name());
        firsts[Sentence(terminal)] = cs;
    }
    // firsts[Sentence(G_.GetEndOfFile())] = ContainerSet<string>().add(G_.GetEndOfFile()->Name());
    
    // Inicializar primeros para no terminales
    for (const auto& nonterminal : G_.NonTerminals()) {
        firsts[Sentence(nonterminal)] = ContainerSet<string>();
    }

    while (changed == true) {
        changed = false;

        for (const auto& prod : G_.Productions()) {
            const auto& X = Sentence(prod.Left());
            const auto& alpha = prod.Right();

            // Firsts de X
            auto& first_X = firsts[X];

            // Firsts de alpha
            auto& first_alpha = firsts[alpha];

            ContainerSet<string> local_first = compute_local_firsts(alpha, firsts, G_, verbose_);

            bool changed_alpha = first_alpha.hard_update(local_first);
            // bool changed_alpha = hard_update_container_set(first_alpha, local_first);
            bool changed_X = first_X.hard_update(local_first);
            // bool changed_X = hard_update_container_set(first_X, local_first);
            changed = changed || changed_alpha || changed_X;
            
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

ContainerSet<string> LALR1Parser::compute_local_firsts(const Sentence& alpha, const map<Sentence, ContainerSet<string>>& firsts, const Grammar& G, bool verbose) {
    if (firsts_cache_.find(alpha) != firsts_cache_.end()) {
        return firsts_cache_.at(alpha);
    }
    ContainerSet<string> local_first = ContainerSet<string>();
    auto symbols = alpha.Symbols();
    bool all_epsilon = true;

    for (const auto& symbol : symbols) {
        auto sym_sentence = Sentence(symbol);
        if (firsts.find(sym_sentence) == firsts.end()) {
            if (symbol->IsTerminal() && !symbol->IsEpsilon()) {
                local_first.add(symbol->Name());
                all_epsilon = false;
                continue; // Si no hay FIRST para el símbolo, asumimos que puede derivar epsilon
            }
        }
        const auto& first_x = firsts.at(sym_sentence);
        for (const auto& terminal : first_x.get_values()) {
            if (terminal != G.GetEpsilon()->Name()) {
                local_first.add(terminal);
            }
        }

        if (!first_x.contains_epsilon()) {
            all_epsilon = false;
            break; // Si un símbolo no deriva epsilon, terminamos el cálculo
        }
    }
    if (all_epsilon) {
        local_first.set_epsilon();
    }

    // Almacenar en la caché para futuras consultas
    firsts_cache_[alpha] = local_first;
    
    return local_first;
}

std::map<Sentence, ContainerSet<string>> LALR1Parser::compute_follows(const map<Sentence, ContainerSet<string>>& symbol_firsts) {
    LogDebug("Calculando conjuntos FOLLOW para LALR1");
    
    std::map<Sentence, ContainerSet<string>> follows;
    bool changed = true;

    // Inicializar los conjuntos FOLLOW para cada no terminal
    for (const auto& nonterminal : G_.NonTerminals()) {
        follows[Sentence(nonterminal)] = ContainerSet<string>();
        
        if (verbose_) {
            LogDebug("FOLLOW(" + nonterminal->Name() + ") inicializado como conjunto vacío");
        }
    }
    
    // El símbolo $ (fin de entrada) pertenece al conjunto FOLLOW del símbolo inicial
    Sentence start_sentence = Sentence(G_.GetStartSymbol());
    shared_ptr<Symbol> EOFile = G_.GetEndOfFile();
    follows[start_sentence] = ContainerSet<string>();
    follows[start_sentence].add(EOFile->Name());
    
    if (verbose_) {
        LogDebug("FOLLOW(" + G_.GetStartSymbol()->Name() + ") = {" + EOFile->Name() + "} (inicial)");
    }

    int iteration = 0;
    while (changed == true) {
        changed = false;
        iteration++;
        
        if (verbose_) {
            LogDebug("Iteración " + std::to_string(iteration) + " del algoritmo de cálculo de FOLLOW");
        }
        
        auto prods = G_.Productions();
        for (const auto& prod : G_.Productions()) {
            const auto& X = Sentence(prod.Left());
            const auto& alpha = prod.Right().Symbols();
            
            if (verbose_) {
                std::string production_str = prod.Left()->Name() + " -> ";
                for (const auto& symbol : alpha) {
                    production_str += symbol->Name() + " ";
                }
                LogDebug("Analizando producción: " + production_str);
            }

            for (int i = 0; i < alpha.size(); i++) {
                if (!alpha[i]->IsNonTerminal()) {
                    continue; // Skip if the symbol is not a non-terminal
                }
                Sentence Y = Sentence(alpha[i]);

                ContainerSet<string> first_rest;
                bool has_epsilon = true;
                for (int j = i + 1; j < alpha.size(); j++) {
                    const auto& beta = alpha[j];
                    auto first_x = symbol_firsts.at(Sentence(beta));

                    for (const auto& sym : first_x.get_values()) {
                        if (sym != G_.GetEpsilon()->Name()) {
                            first_rest.add(sym);
                        }
                    }
                    if (!first_x.contains_epsilon()) {
                        has_epsilon = false;
                        break; // Stop if we find a symbol that does not derive epsilon
                    }
                }

                bool updated = false;
                
                if (!first_rest.get_values().empty() && follows[Y].update(first_rest)) {
                    changed = true;
                    updated = true;
                    
                    if (verbose_) {
                        std::string first_rest_str = "{";
                        bool first = true;
                        for (const auto& sym : first_rest.get_values()) {
                            if (!first) first_rest_str += ", ";
                            first_rest_str += sym;
                            first = false;
                        }
                        first_rest_str += "}";
                        
                        LogDebug("Actualizado FOLLOW(" + Y.ToString() + ") con FIRST del resto: " + first_rest_str);
                    }
                }

                if (has_epsilon || i == alpha.size() - 1) {
                    // If the last symbol or has epsilon, add follows[X] to follows[Y]
                    if (follows[Y].update(follows[X])) {
                        changed = true;
                        updated = true;
                        
                        if (verbose_) {
                            LogDebug("Actualizado FOLLOW(" + Y.ToString() + ") con FOLLOW(" + X.ToString() + ")");
                        }
                    }
                }
                
                if (updated && verbose_) {
                    std::string follow_str = "{";
                    bool first = true;
                    for (const auto& sym : follows[Y].get_values()) {
                        if (!first) follow_str += ", ";
                        follow_str += sym;
                        first = false;
                    }
                    follow_str += "}";
                    
                    LogDebug("FOLLOW(" + Y.ToString() + ") actualizado: " + follow_str);
                }
            }
        }
    }
    
    // Registrar los conjuntos FOLLOW finales en los logs
    if (verbose_) {
        LogDebug("=== CONJUNTOS FOLLOW CALCULADOS ===");
        for (const auto& [key, value] : follows) {
            std::string values = "{";
            bool first = true;
            for (const auto& val : value.get_values()) {
                if (!first) values += ", ";
                values += val;
                first = false;
            }
            values += "}";
            LogDebug("FOLLOW(" + key.ToString() + ") = " + values);
        }
    } else {
        // Versión resumida para log siempre (no verbose)
        LogInfo("Conjuntos FOLLOW calculados");
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

std::vector<Item> LALR1Parser::expand(const Item& item, const map<Sentence, ContainerSet<string>>& firsts, Grammar& G) {
    std::vector<Item> expanded;
    const auto& next_symbol = item.NextSymbol();
    if (next_symbol == nullptr || !next_symbol->IsNonTerminal()) {
        return expanded;
    }
    auto lookaheads = ContainerSet<string>();

    // Obtener la secuencia β (símbolos después del punto)
    auto beta_symbols = item.GetBetaSymbols();
    
    // Calcular FIRST(β) - sin epsilon
    Sentence beta_sentence(beta_symbols);
    auto first_beta = compute_local_firsts(beta_sentence, firsts, G, false);
    
    for (const auto& terminal : first_beta.get_values()) {
        // Añadir solo si no es epsilon
        if (terminal != G.GetEpsilon()->Name()) {
            lookaheads.add(terminal);
        }
    }
    // Si FIRST(β) contiene epsilon, añadimos los lookaheads del ítem actual
    for (const auto& terminal : first_beta.get_values()) {
        if (terminal != G.GetEpsilon()->Name()) {
            lookaheads.update(item.lookaheads());
        }
    }
    
    for (const auto& prod : G.Productions()) {
        if (prod.Left()->Name() == next_symbol->Name()) {
            auto prod_ptr = std::make_shared<Production>(prod);
            expanded.push_back(Item(prod_ptr, 0, lookaheads));
        }
    }
        
    return expanded;
}
std::set<Item> LALR1Parser::compress(const vector<Item>& items) {
    
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

std::vector<Item> LALR1Parser::closure_lr1(const std::vector<Item>& items, const std::map<Sentence, ContainerSet<string>>& firsts) {
    // Inicializar el conjunto de cierre con los elementos iniciales
    ContainerSet<Item> closure;
    closure.add(items);

    bool changed = true;
    while (changed) {
        changed = false;

        // Crear un nuevo conjunto para los nuevos elementos
        ContainerSet<Item> new_items;
        for (const auto& item : closure) {
            auto next_sym = item.NextSymbol();
            if (!next_sym || !next_sym->IsNonTerminal()) continue;

            auto beta = item.GetBetaSymbols();
            std::vector<shared_ptr<Symbol>> beta_symbols;
            for (const auto& sym : beta) {
                beta_symbols.push_back(sym);
            }

            auto first_beta = compute_local_firsts(Sentence(beta_symbols), firsts, G_, false);

            ContainerSet<string> lookaheads;
            for (const auto& term : first_beta.get_values()) {
                // Añadir solo si no es epsilon
                if (term != G_.GetEpsilon()->Name()) {
                    lookaheads.add(term);
                }
            }
            if (first_beta.contains_epsilon()) {
                // Si FIRST(β) contiene epsilon, añadimos los lookaheads del ítem actual
                lookaheads.update(item.lookaheads());
            }

            bool exists = false;
            for (const auto& prod : G_.Productions()) {
                if (prod.Left()->Name() == next_sym->Name()) {
                    // Crear un nuevo ítem con la producción y el lookahead calculado
                    auto new_item = Item(std::make_shared<Production>(prod), 0, lookaheads);
                    for (const auto& existing : closure.get_set()) {
                        if (existing == new_item) {
                            // Si el ítem ya está en el cierre, no lo añadimos
                            exists = true;
                            break;
                        }
                    }
                    if (!exists) {
                        new_items.add(new_item);
                        changed = true; // Si se añade un nuevo ítem, marcamos que hubo un cambio
                    }
                }
            }
        }
        if (changed) {
            closure.update(new_items);
        }
    }
    return std::vector<Item>(closure.get_set().begin(), closure.get_set().end());
}
vector<Item> LALR1Parser::goto_lr1(const vector<Item>& items, shared_ptr<Symbol> symbol, const map<Sentence, ContainerSet<string>>& firsts, bool just_kernel) {
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
    return closure_lr1(goto_items, firsts);
}
State LALR1Parser::BuildLALR1Automaton() {

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
    auto closure = closure_lr1({start_item}, firsts);
    
    // Estado inicial
    int state_id = 0;
    auto automaton = State(state_id++, true);
    for (const auto& item : closure) {
        automaton.add_item(item);
    }

    // Almacenamiento de estados por núcleo
    std::map<std::set<std::pair<int, int>>, State*> kernel_to_state;
    // std::map<State*, std::set<ItemCore>> state_to_kernel;
    
    // Inicializar con el estado inicial
    std::set<std::pair<int, int>> initial_kernel = {{start_production.get_id(), 0}};
    kernel_to_state[initial_kernel] = &automaton;
    // state_to_kernel[&automaton] = initial_kernel;

    // Cola para procesar estados
    std::deque<State*> pending;
    pending.push_back(&automaton);

    while (!pending.empty()) {
        auto current_state = pending.front();
        pending.pop_front();

        set<shared_ptr<Symbol>> trans_symbols;
        for (const auto& item : current_state->get_items()){
            if (item.NextSymbol() != nullptr) {
                trans_symbols.insert(item.NextSymbol());
            }
        }

        for (const auto& symbol : trans_symbols) {
            auto goto_items = goto_lr1(current_state->get_items(), symbol, firsts, false);
            if (goto_items.empty()) continue;
        

            set<pair<int, int>> new_kernel;
            for (const auto& item : goto_items) {
                if (item.pos() > 0 || item.production()->Left() == G_.GetStartSymbol()) {
                    new_kernel.insert({item.production()->get_id(), item.pos()});
                }
            }
            State* new_state = nullptr;

            if (auto it = kernel_to_state.find(new_kernel); it != kernel_to_state.end()) {
                // Estado existente: fusionar lookaheads
                new_state = it->second;
                // Fusionar lookaheads
                for (const auto& new_item : goto_items) {
                    for (auto& existing_item : new_state->get_mutable_items()) {
                        if (existing_item.production()->get_id() == new_item.production()->get_id() &&
                            existing_item.pos() == new_item.pos()) {
                            existing_item.merge_lookaheads(new_item.lookaheads());
                        }
                    }
                }
                // Crear nuevo estado
                new_state = new State(state_id++, true);
                for (const auto& item : goto_items) {
                    new_state->add_item(item);
                }
                
                // Registrar nuevo estado
                kernel_to_state[new_kernel] = new_state;
                pending.push_back(new_state);
                automaton_states_.push_back(new_state); // Esto es solo pa limpiar memoria cuando se borre el parser
            }

            current_state->add_transition(symbol->Name(), new_state);
        }
    }
    return automaton;
}

// Método para limpiar todos los estados creados
void LALR1Parser::CleanupAutomatonStates() {
    if (!automaton_states_.empty()) {
        LogDebug("Limpiando estados del autómata LALR1 (" + std::to_string(automaton_states_.size()) + " estados)");
    }
    
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
LALR1Parser::~LALR1Parser() {
    CleanupAutomatonStates();
}

// ============= IMPLEMENTACIÓN DE SERIALIZACIÓN DEL PARSER =============

// Constructor privado para deserialización
LALR1Parser::LALR1Parser(Grammar& G, 
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

bool LALR1Parser::serialize_parser(const std::string& filename) const {
    if (!ensure_hulk_directory()) {
        return false;
    }
    return serialize_parser(filename, "hulk");
}

bool LALR1Parser::serialize_parser(const std::string& filename, const std::string& directory) const {
    if (!ensure_directory(directory)) {
        return false;
    }
    
    std::string filepath = get_custom_path(filename, directory);
    std::ofstream file(filepath, std::ios::binary);
    
    if (!file.is_open()) {
        LogError("Error: No se pudo abrir el archivo para escritura: " + filepath);
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

        return true;
        
    } catch (const std::exception& e) {
        LogError("Error durante la serialización del parser: " + std::string(e.what()));
        file.close();
        return false;
    }
}

LALR1Parser* LALR1Parser::deserialize_parser(const std::string& filename, Grammar& grammar) {
    return deserialize_parser(filename, "hulk", grammar);
}

LALR1Parser* LALR1Parser::deserialize_parser(const std::string& filename, const std::string& directory, Grammar& grammar) {
    std::string filepath = get_custom_path(filename, directory);
    std::ifstream file(filepath, std::ios::binary);
    
    if (!file.is_open()) {
        LogError("Error: No se pudo abrir el archivo para lectura: " + filepath);
        return nullptr;
    }
    
    try {
        // Verificar firma
        char signature[11] = {0};
        file.read(signature, 10);
        if (std::string(signature) != "LR1PARSER") {
            LogError("Error: Archivo no es un parser serializado válido");
            file.close();
            return nullptr;
        }
        
        // Leer versión
        uint32_t version;
        file.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (version != 1) {
            LogError("Error: Versión de parser no soportada: " + std::to_string(version));
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
                LogError("Error: Símbolo no encontrado en gramática: " + symbol_name);
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
                LogError("Error: Símbolo no encontrado en gramática: " + symbol_name);
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
        return new LALR1Parser(grammar, action, goto_table, verbose);
        
    } catch (const std::exception& e) {
        LogError("Error durante la deserialización del parser: " + std::string(e.what()));
        file.close();
        return nullptr;
    }
}

std::vector<std::string> LALR1Parser::getExpectedTokens(int state_id) const {
    std::vector<std::string> expected_tokens;
    
    // Buscar todas las acciones válidas para este estado
    for (const auto& [key, value] : action_) {
        if (key.first == state_id) {
            expected_tokens.push_back(key.second.Name());
        }
    }
    
    return expected_tokens;
}

std::pair<std::string, std::vector<std::string>> LALR1Parser::generateErrorMessage(int state_id, const std::string& token) const {
    std::vector<std::string> expected_tokens = getExpectedTokens(state_id);
    
    LogError("Error sintáctico en estado " + std::to_string(state_id) + ": token inesperado '" + token + "'");
    
    if (verbose_ && !expected_tokens.empty()) {
        std::string expected = "Tokens esperados: ";
        for (size_t i = 0; i < expected_tokens.size(); ++i) {
            if (i > 0) {
                expected += (i == expected_tokens.size() - 1) ? " o " : ", ";
            }
            expected += "'" + expected_tokens[i] + "'";
        }
        LogDebug(expected);
    }
    
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

std::string LALR1Parser::formatErrorWithContext(const std::vector<Terminal>& tokens, 
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

std::string LALR1Parser::formatErrorWithContext(const std::vector<Token>& tokens, 
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
std::pair<std::vector<int>, std::vector<std::string>> LALR1Parser::Parse(const std::vector<Token>& tokens) {
    std::vector<int> production_ids;
    std::vector<std::string> actions;
    
    // Initialize stack and state
    std::stack<int> state_stack;
    state_stack.push(0);
    std::stack<Token> symbol_stack;

    int index = 0;
    int current_position = 0;
    
    LogInfo("LALR1Parser: Analizando " + std::to_string(tokens.size()) + " tokens");
    
    if (verbose_) {
        LogDebug("Lista de tokens a analizar en LALR1:");
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
                        
                        if (verbose_) {
                            LogDebug("SHIFT: Token '" + current_token.Name() + "' al estado " + 
                                    std::to_string(action_value.second) + 
                                    " (lexema: '" + current_token.Lexeme() + 
                                    "', línea " + std::to_string(current_token.Line()) + 
                                    ", columna " + std::to_string(current_token.Column()) + ")");
                        }
                        
                        index++;
                    } 
                    else if (action_value.first == REDUCE) {
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
                        } 
                        else {
                            // Error interno del parser - no se encontró una transición goto
                            std::string error_msg = "Error interno del parser: no se encontró transición GOTO para el no terminal '" + 
                                                production.Left()->Name() + "' en el estado " + std::to_string(state_stack.top());
                            
                            LogError(error_msg);
                            throw std::runtime_error(error_msg);
                        }
                    } 
                    else if (action_value.first == OK) {
                        // Accept action
                        actions.push_back(OK);
                        
                        LogInfo("Parser LALR1: Entrada aceptada");
                        
                        if (verbose_) {
                            LogDebug("ACCEPT: La entrada ha sido reconocida correctamente por la gramática");
                        }
                        
                        break;
                    } 
                    else {
                        std::string error_msg = "Acción desconocida: " + action_value.first;
                        LogError(error_msg);
                        throw std::runtime_error(error_msg);
                    }
                } 
                else {
                    // No se encontró una acción para este estado y token
                    auto [error_msg, expected_tokens] = generateErrorMessage(state_stack.top(), current_token.Name());
                    
                    // Enriquecer el mensaje con información de posición
                    std::string position_info = " en línea " + std::to_string(current_token.Line()) + 
                                               ", columna " + std::to_string(current_token.Column());
                    
                    std::string enhanced_error = error_msg + position_info;
                    
                    if (verbose_) {
                        LogDebug("Contexto del error:");
                        int context_start = std::max(0, current_position - 3);
                        int context_end = std::min(static_cast<int>(tokens.size()) - 1, current_position + 3);
                        
                        for (int i = context_start; i <= context_end; ++i) {
                            std::string marker = (i == current_position) ? " >> " : "    ";
                            LogDebug(marker + tokens[i].ToString() + " (lexema: '" + tokens[i].Lexeme() + "')");
                        }
                    }
                    
                    // Lanzar LR1ParsingError con toda la información
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