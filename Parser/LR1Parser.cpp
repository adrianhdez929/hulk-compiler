#include "LR1Parser.h"
#include <cassert>
#include <iostream>
#include "../Automata/state.h"

LR1Parser::LR1Parser(Grammar& G, bool verbose)
    : G_(G), verbose_(verbose) {
    // Initialize action and goto tables
    action_ = std::map<std::pair<int, std::shared_ptr<Terminal>>, std::pair<std::string, int>>();
    goto_ = std::map<std::pair<int, std::shared_ptr<NonTerminal>>, int>();
    BuildParsingTable();
}
static bool compare_lookaheads(const ContainerSet<shared_ptr<Symbol>>& lhs, const ContainerSet<shared_ptr<Symbol>>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    
    // Para cada símbolo en lhs, debe existir un símbolo equivalente en rhs
    for (const auto& lhs_symbol : lhs.get_values()) {
        bool found = false;
        for (const auto& rhs_symbol : rhs.get_values()) {
            if (lhs_symbol->Name() == rhs_symbol->Name()) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;  // No se encontró un símbolo equivalente en rhs
        }
    }
    return true;
}
static bool compare_items(const Item& lhs, const Item& rhs) {
    string lhs_prod = lhs.production()->ToString();
    string rhs_prod = rhs.production()->ToString();
    if (lhs_prod != rhs_prod) {
        return false; // Productions are not equal
    }
    if (lhs.pos() != rhs.pos()) {
        return false;
    }
    if (!compare_lookaheads(lhs.lookaheads(), rhs.lookaheads())) {
        return false;
    }
    return true;
}
static bool update_container_set(ContainerSet<shared_ptr<Symbol>>& lhs, const ContainerSet<shared_ptr<Symbol>>& rhs) {
    bool updated = false;
    
    // Si lhs está vacío, simplemente agregamos todos los símbolos de rhs
    if (lhs.get_values().empty()) {
        for (const auto& symbol : rhs.get_values()) {
            updated |= lhs.add(symbol);
        }
        return updated;
    }
    
    // Si lhs no está vacío, comprobamos cada símbolo de rhs
    for (const auto& symbol : rhs.get_values()) {
        bool exists = false;
        
        // Verificar si el símbolo ya existe en lhs
        for (const auto& existing_symbol : lhs.get_values()) {
            if (symbol->Name() == existing_symbol->Name()) {
                exists = true;
                break;
            }
        }
        
        // Si no existe, lo agregamos
        if (!exists) {
            updated |= lhs.add(symbol);
        }
    }
    
    return updated;
}

// Realiza una actualización completa incluyendo epsilon
static bool hard_update_container_set(ContainerSet<std::shared_ptr<Symbol>>& lhs, const ContainerSet<std::shared_ptr<Symbol>>& rhs) {
    bool updated = update_container_set(lhs, rhs);
    
    // Actualizar epsilon solo si rhs lo tiene y lhs no
    if (rhs.contains_epsilon() && !lhs.contains_epsilon()) {
        lhs.set_epsilon(true);
        updated = true;
    }
    
    return updated;
}

//Parse
std::pair<std::vector<std::shared_ptr<Production>>, std::vector<std::string>> LR1Parser::Parse(const std::vector<std::shared_ptr<Terminal>>& tokens) {
    std::vector<std::shared_ptr<Production>> productions;
    std::vector<std::string> actions;
    
    // Initialize stack and state
    std::vector<int> state_stack = {0};
    std::vector<std::shared_ptr<Terminal>> symbol_stack;
    
    int index = 0;
    while (index < tokens.size() || !symbol_stack.empty()) {
        if (index < tokens.size()) {
            auto current_token = tokens[index];
            auto action_key = std::make_pair(state_stack.back(), current_token);
            if (action_.find(action_key) != action_.end()) {
                auto action_value = action_[action_key];
                if (action_value.first == SHIFT) {
                    // Shift action
                    state_stack.push_back(action_value.second);
                    symbol_stack.push_back(current_token);
                    actions.push_back(SHIFT);
                    index++;
                } else if (action_value.first == REDUCE) {
                    // Reduce action
                    auto production = G_.Productions()[action_value.second];
                    productions.push_back(std::make_shared<Production>(production));
                    actions.push_back(REDUCE);
                    
                    for (int i = 0; i < production.Right().Symbols().size(); i++) {
                        state_stack.pop_back();
                        symbol_stack.pop_back();
                    }
                    
                    auto goto_key = std::make_pair(state_stack.back(), production.Left());
                    if (goto_.find(goto_key) != goto_.end()) {
                        state_stack.push_back(goto_[goto_key]);
                        //actions.push_back(OK);//NOTE: ATENCIOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOONNNNNNNNNNNNNN
                    } else {
                        throw std::runtime_error("Goto not found for " + production.Left()->Name());
                    }
                } else if (action_value.first == OK) {
                    // Accept action
                    actions.push_back(OK);
                    break;
                } else {
                    throw std::runtime_error("Unknown action: " + action_value.first);
                }
            } else {
                throw std::runtime_error("Action not found for state " + std::to_string(state_stack.back()) + " and token " + current_token->Name());
            }
        } else {
            // If no more tokens, check for reduce or accept
            if (!symbol_stack.empty() && state_stack.back() == 0) {
                actions.push_back(OK);
            }
        }
    }
    return std::make_pair(productions, actions);
}

//Register
void LR1Parser::Register(std::map<std::pair<int, std::shared_ptr<Terminal>>, std::pair<std::string, int>>& table, 
                                 const std::pair<int, std::shared_ptr<Terminal>>& key, 
                                 const std::pair<std::string, int>& value) {
    if (verbose_) {
        std::cout << "Registering action: " << key.first << ", " << key.second->Name() << " -> " << value.first << ", " << value.second << std::endl;
    }
    table[key] = value;
}
void LR1Parser::Register(std::map<std::pair<int, std::shared_ptr<NonTerminal>>, int>& table, 
                                 const std::pair<int, std::shared_ptr<NonTerminal>>& key, 
                                 int value) {
    if (verbose_) {
        std::cout << "Registering goto: " << key.first << ", " << key.second->Name() << " -> " << value << std::endl;
    }
    table[key] = value;
}

//LR1PARSER BuildParsingTable
void LR1Parser::BuildParsingTable() {
    // Limpiar estados anteriores si existen
    CleanupAutomatonStates();
    
    G_.Augment();
    if (!G_.IsAugmented()) {
        throw std::runtime_error("Grammar is not augmented");
    }
    auto automaton = BuildLR1Automaton();

    for (const auto& state : automaton.get_all_states()) {
        int state_id = state->id();
        for (const auto& item : state->get_items()) {
            
            if (item.IsReduceItem()) {
                // Reduce action
                auto production = item.production();
                if (production->Left() == G_.GetStartSymbol()) {
                    // Accept action
                    Register(action_, {state_id, std::dynamic_pointer_cast<Terminal>(G_.GetEndOfFile())}, {OK, 0});
                } else {
                    // Regular reduce action
                    auto lookaheads = item.lookaheads().get_values();
                    for (const auto& lookahead : lookaheads) {
                        Register(action_, {state_id, std::dynamic_pointer_cast<Terminal>(lookahead)}, {REDUCE, production->get_id()});
                    }
                }
            } else {
                // Shift action
                auto next_symbol = item.NextSymbol();
                if (next_symbol && next_symbol->IsTerminal()) {
                    auto next_state = automaton.transitions().at(next_symbol->Name())[0];// Assuming deterministic transitions
                    // Shift action
                    Register(action_, {state_id, std::static_pointer_cast<Terminal>(next_symbol)}, {SHIFT, next_state->id()});
                } else if (next_symbol && next_symbol->IsNonTerminal()) {
                    // Goto action
                    auto next_state = automaton.transitions().at(next_symbol->Name())[0]; // Assuming deterministic transitions
                    Register(goto_, {state_id, std::static_pointer_cast<NonTerminal>(next_symbol)}, next_state->id());
                }
            }
        }
    }
}

ContainerSet<shared_ptr<Symbol>> LR1Parser::compute_local_firsts(const Sentence& alpha, const map<shared_ptr<Symbol>, ContainerSet<shared_ptr<Symbol>>>& firsts) {
    //Compute local first
    ContainerSet<shared_ptr<Symbol>> local_first = ContainerSet<shared_ptr<Symbol>>();
    auto symbols = alpha.Symbols();
    // If alpha is epsilon, add epsilon to local first
    for (const auto& symbol : symbols) {
        if (symbol->IsEpsilon()) {
            local_first.add(symbol);
            local_first.set_epsilon(true);
        }
    }
    if (!local_first.contains_epsilon()){
        // local_first.update(firsts.at(symbols[0]));
        update_container_set(local_first, firsts.at(symbols[0]));
        int i = 0;
        std::shared_ptr<Symbol> s = symbols[i];
        while (firsts.at(s).contains_epsilon()) {
            if (i < symbols.size() - 1) {
                i++;
                s = symbols[i];
                if (!firsts.at(s).contains_epsilon()) {
                    update_container_set(local_first, firsts.at(s));
                    break;
                }
            } else {
                local_first.add(G_.GetEpsilon());
                local_first.set_epsilon(true);
                break;
            }
        }
    }
    return local_first;
}

pair<map<shared_ptr<Symbol>, ContainerSet<shared_ptr<Symbol>>>, map<Sentence, ContainerSet<shared_ptr<Symbol>>>> LR1Parser::compute_firsts() {
    std::map<std::shared_ptr<Symbol>, ContainerSet<shared_ptr<Symbol>>> firsts;
    bool changed = true;
    
    // Inicializar primeros para terminales
    for (const auto& terminal : G_.Terminals()) {
        ContainerSet<shared_ptr<Symbol>> cs;
        cs.add(terminal);
        firsts[terminal] = cs;
    }
    
    // Inicializar primeros para no terminales
    for (const auto& nonterminal : G_.NonTerminals()) {
        firsts[nonterminal] = ContainerSet<shared_ptr<Symbol>>();
    }

    std::map<Sentence, ContainerSet<shared_ptr<Symbol>>> sentence_firsts;
    for (const auto& prod : G_.Productions()) {
        auto right = prod.Right();
        sentence_firsts[right] = ContainerSet<shared_ptr<Symbol>>();
    }
    while (changed == true) {
        changed = false;

        for (const auto& prod : G_.Productions()) {
            const auto& X = prod.Left();
            const auto& alpha = prod.Right();

            // Firsts de X
            auto& first_X = firsts[X];

            // Firsts de alpha
            auto& first_alpha = sentence_firsts[alpha];

            ContainerSet<shared_ptr<Symbol>> local_first = compute_local_firsts(alpha, firsts);

            // bool changed_alpha = first_alpha.hard_update(local_first);
            bool changed_alpha = hard_update_container_set(first_alpha, local_first);
            // bool changed_X = first_X.hard_update(local_first);
            bool changed_X = hard_update_container_set(first_X, local_first);
            changed = changed || changed_alpha || changed_X;
            
        }
    }
    return std::make_pair(firsts, sentence_firsts);
};

//Follows
std::map<std::shared_ptr<Symbol>, ContainerSet<shared_ptr<Symbol>>> LR1Parser::compute_follows(const map<shared_ptr<Symbol>, ContainerSet<shared_ptr<Symbol>>>& symbol_firsts) {
    std::map<std::shared_ptr<Symbol>, ContainerSet<shared_ptr<Symbol>>> follows;
    bool changed = true;

    for (const auto& nonterminal : G_.NonTerminals()) {
        follows[nonterminal] = ContainerSet<shared_ptr<Symbol>>();
    }
    shared_ptr<Symbol> start_symbol = G_.GetStartSymbol();
    shared_ptr<Symbol> EOFile = G_.GetEndOfFile();
    follows[start_symbol] = ContainerSet<shared_ptr<Symbol>>();
    follows[start_symbol].add(EOFile);

    while (changed == true) {
        changed = false;
        for (const auto& prod : G_.Productions()) {
            const auto& X = prod.Left();
            const auto& alpha = prod.Right();

            auto& follows_x = follows[X];

            // If alpha is epsilon, add epsilon to local first
            for (const auto& symbol : alpha.Symbols()) {
                if (symbol->IsEpsilon()) {
                    continue;
                }
            }

            int n = alpha.Symbols().size() - 1;
            for (int i = 0; i < n; i++) {
                auto& Y = alpha.Symbols()[i];
                auto& beta = alpha.Symbols()[i + 1];
                if (Y->IsNonTerminal()) {
                    if (symbol_firsts.find(beta) != symbol_firsts.end()) {
                        // changed |= follows[Y].update(symbol_firsts.at(beta));
                        changed |= update_container_set(follows[Y], symbol_firsts.at(beta));
                        if (symbol_firsts.at(beta).contains_epsilon()) {
                            // changed |= follows[Y].update(follows_x);
                            changed |= update_container_set(follows[Y], follows_x);
                        }
                    }
                    if (i == n-1 && beta->IsNonTerminal()) {
                        changed |= update_container_set(follows.at(beta), follows_x);
                    }
                }
                // auto& current_follow = follows[current_symbol]; // Remove or comment out if current_symbol is undefined
            }
        }
    }
    return follows;
};

vector<Item> LR1Parser::expand(const Item& item, const map<shared_ptr<Symbol>, ContainerSet<shared_ptr<Symbol>>>& firsts) {
    vector<Item> expanded;
    const auto& next_symbol = item.NextSymbol();
    if (next_symbol == nullptr || !next_symbol->IsNonTerminal()) {
        return expanded;
    }
    auto lookaheads = ContainerSet<shared_ptr<Symbol>>();
    for (const auto& preview : item.Preview()) {
        // lookaheads.update(compute_local_firsts(Sentence(preview), firsts));
        auto local_first = compute_local_firsts(Sentence(preview), firsts);
        update_container_set(lookaheads, local_first);
    }
    if (lookaheads.contains_epsilon()) {
        throw std::runtime_error("Epsilon in lookaheads");
    }
    for (auto& prod : G_.Productions()) {
        if (prod.Left() == next_symbol) {
            // Usamos una referencia a la producción existente en lugar de crear una nueva
            auto prod_ptr = std::make_shared<Production>(prod);
            expanded.push_back(Item(prod_ptr, 0, lookaheads));
            
            // Añadimos debug para verificar si hay elementos duplicados
            // std::cout << "Expandido: " << prod_ptr->ToString() << ", pos: 0, LA: " << lookaheads.str() << std::endl;
        }
    }
    return expanded;
}
set<Item> LR1Parser::compress(const vector<Item>& items) {
    map<pair<string, int>, pair<shared_ptr<Production>, ContainerSet<shared_ptr<Symbol>>>> centers;
    for (const auto& item : items) {
        auto key = make_pair(item.production()->ToString(), item.pos());
        if (centers.find(key) == centers.end()) {
            centers[key] = make_pair(item.production(), item.lookaheads());
        } else {
            update_container_set(centers[key].second, item.lookaheads());
        }
        
    }
    set<Item> compressed;
    for (const auto& [key, lookaheads] : centers) {
        compressed.insert(Item(centers[key].first, key.second, centers[key].second));
    }
    return compressed;
}

// vector<Item> LR1Parser::closure_lr1(const vector<Item>& items, const map<shared_ptr<Symbol>, ContainerSet<shared_ptr<Symbol>>>& firsts) {
//     vector<Item> closure;
//     set<Item> visited;
//     queue<Item> to_visit;

//     for (const auto& item : items) {
//         to_visit.push(item);
//     }

//     while (!to_visit.empty()) {
//         auto item = to_visit.front();
//         to_visit.pop();

//         if (visited.find(item) != visited.end()) {
//             continue;
//         }
//         visited.insert(item);

//         if (item.IsReduceItem()) {
//             closure.push_back(item);
//             continue;
//         }

//         auto expanded_items = expand(item, firsts);
//         for (const auto& expanded_item : expanded_items) {
//             if (visited.find(expanded_item) == visited.end()) {
//                 to_visit.push(expanded_item);
//             }
//         }
//     }
//     return closure;
// }


std::vector<Item> LR1Parser::closure_lr1(const std::vector<Item>& items, const std::map<std::shared_ptr<Symbol>, ContainerSet<std::shared_ptr<Symbol>>>& firsts) {
    // Inicializar el conjunto de cierre con los elementos iniciales
    std::vector<Item> closure = items;

    bool changed = true;
    while (changed) {
        changed = false;

        // Crear un nuevo conjunto para los nuevos elementos
        std::vector<Item> new_items;
        for (const auto& item : closure) {
            string item_str = item.production()->ToString() + " pos: " + std::to_string(item.pos());
            auto lookaheads = item.lookaheads();
            // Expandir cada elemento y agregar los nuevos elementos al conjunto
            auto expanded_items = expand(item, firsts);
            new_items.insert(new_items.end(), expanded_items.begin(), expanded_items.end());
        }

        // Agregar los nuevos elementos al cierre si no están ya presentes
        for (const auto& new_item : new_items) {
            bool found = false;
            for (const auto& existing_item : closure) {
                // Comparamos manualmente usando los valores, no los punteros
                if (compare_items(existing_item, new_item)) {
                    found = true;
                    break;
                }
            }
            
            if (!found) {
                closure.push_back(new_item);
                changed = true;
            }
        }
    }

    auto compressed_set = compress(closure);
    return std::vector<Item>(compressed_set.begin(), compressed_set.end());
} 


vector<Item> LR1Parser::goto_lr1(const vector<Item>& items, shared_ptr<Symbol> symbol, const map<shared_ptr<Symbol>, ContainerSet<shared_ptr<Symbol>>>& firsts, bool just_kernel) {
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

//build lr1 automaton
State LR1Parser::BuildLR1Automaton() {
    assert(G_.GetStartSymbol()->productions.size() == 1 && "Grammar must be augmented");

    // Compute first sets
    auto [symbol_firsts, sentence_firsts] = compute_firsts();
    symbol_firsts[G_.GetEndOfFile()] = ContainerSet<std::shared_ptr<Symbol>>();
    symbol_firsts[G_.GetEndOfFile()].add(G_.GetEndOfFile());

    // Initialize start production and item
    auto start_production = G_.GetStartSymbol()->productions[0];
    ContainerSet<std::shared_ptr<Symbol>> lookahead_set;
    lookahead_set.add(G_.GetEndOfFile());
    auto start_item = Item(std::make_shared<Production>(start_production), 0, lookahead_set);
    std::vector<Item> start = {start_item};

    // Compute closure for the start state
    auto closure = closure_lr1(start, symbol_firsts);
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
            auto next_items = goto_lr1(std::vector<Item>(current.begin(), current.end()), symbol, symbol_firsts, true);
            if (next_items.empty()) {
                continue;
            }

            std::set<Item> next_set(next_items.begin(), next_items.end());
            State* next_state;

            if (visited.find(next_set) == visited.end()) {
                auto next_closure = closure_lr1(next_items, symbol_firsts);
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
            auto next_items = goto_lr1(std::vector<Item>(current.begin(), current.end()), symbol, symbol_firsts, true);
            if (next_items.empty()) {
                continue;
            }

            std::set<Item> next_set(next_items.begin(), next_items.end());
            State* next_state;

            if (visited.find(next_set) == visited.end()) {
                auto next_closure = closure_lr1(next_items, symbol_firsts);
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

// Destructor de LR1Parser
LR1Parser::~LR1Parser() {
    CleanupAutomatonStates();
}

