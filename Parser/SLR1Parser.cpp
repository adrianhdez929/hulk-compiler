#include "SLR1Parser.h"
#include "../Automata/utils/ContainerSet.h"
#include <stack>
#include <assert.h>
#include <iostream>

SLR1Parser::SLR1Parser(Grammar& G, bool verbose)
    : G_(G), verbose_(verbose) {
    // Initialize action and goto tables
    action_ = std::map<std::pair<int, Symbol>, std::pair<std::string, int>>();
    goto_ = std::map<std::pair<int, Symbol>, int>();
    BuildParsingTable();
}

//Parse method
std::pair<std::vector<Production>, std::vector<std::string>> SLR1Parser::Parse(const std::vector<std::string>& tokens) {
    // Convert string tokens to Terminal objects
    std::vector<Terminal> terminal_tokens;
    for (const auto& token : tokens) {
        terminal_tokens.push_back(Terminal(token, G_));
    }
    return Parse(terminal_tokens);
}
std::pair<std::vector<Production>, std::vector<std::string>> SLR1Parser::Parse(const std::vector<Terminal>& tokens) {
    std::vector<Production> productions;
    std::vector<std::string> actions;
    
    // Initialize stack and state
    std::stack<int> state_stack;
    state_stack.push(0);
    std::stack<Terminal> symbol_stack;

    int index = 0;
    while (index < tokens.size() || !symbol_stack.empty()) {
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
                    productions.push_back(production);
                    actions.push_back(REDUCE);
                    // index++;
                    for (int i = 0; i < production.Right().Symbols().size(); i++) {
                        state_stack.pop();
                        if (!symbol_stack.empty()) {
                            symbol_stack.pop();
                        }
                    }

                    auto goto_key = std::make_pair(state_stack.top(), *(production.Left()));
                    if (goto_.find(goto_key) != goto_.end()) {
                        state_stack.push(goto_[goto_key]);
                        // symbol_stack.push(*(production.Left()));
                        //actions.push_back(OK);//NOTE: ATENCIOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOONNNNNNNNNNNNNN
                    } else {
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
                        throw std::runtime_error("Goto not found for " + production.Left()->Name() + " in state " + std::to_string(state_stack.top()));
                    }
                } else if (action_value.first == OK) {
                    // Accept action
                    actions.push_back(OK);
                    break;
                } else {
                    throw std::runtime_error("Unknown action: " + action_value.first);
                }
            } else {
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
                throw std::runtime_error("Action not found for state " + std::to_string(state_stack.top()) + " and token " + current_token.Name());
            }
        } else {
            // If no more tokens, check for reduce or accept
            if (!symbol_stack.empty() && state_stack.top() == 0) {
                actions.push_back(OK);
            }
        }
    }
    return std::make_pair(productions, actions);
}

void SLR1Parser::BuildParsingTable() {
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

    auto follows = compute_follows(firsts);
    
    // Debug: Print FOLLOW sets
    if (verbose_) {
        std::cout << "FOLLOW sets:" << std::endl;
        for (const auto& [nt, follow_set] : follows) {
            std::cout << "FOLLOW(" << nt.ToString() << ") = { ";
            for (const auto& t : follow_set.get_values()) {
                std::cout << t << " ";
            }
            std::cout << "}" << std::endl;
        }
    }
    
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
        std::cout << "Registering action: " << key.first << ", " << key.second.Name() << " -> " << value.first << ", " << value.second << std::endl;
    }
    table[key] = value;
}
void SLR1Parser::Register(std::map<std::pair<int, Symbol>, int>& table, 
                                 const std::pair<int, Symbol>& key, 
                                 int value) {
    if (verbose_) {
        std::cout << "Registering goto: " << key.first << ", " << key.second.Name() << " -> " << value << std::endl;
    }
    table[key] = value;
}


State SLR1Parser::BuildLR0Automaton() {
    assert(G_.GetStartSymbol()->productions.size() == 1 && "Grammar must be augmented");

    auto start_production = G_.GetStartSymbol()->productions[0];
    auto start_item = Item(std::make_shared<Production>(start_production), 0);
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
                    Item new_item(std::make_shared<Production>(production), 0);
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

    // for (const auto& prod : G_.Productions()) {
    //     auto right = prod.Right();
    //     firsts[right] = ContainerSet<string>();
    // }
    while (changed == true) {
        changed = false;

        for (const auto& prod : G_.Productions()) {
            const auto& X = Sentence(prod.Left());
            const auto& alpha = prod.Right();

            // Firsts de X
            auto& first_X = firsts[X];

            // Firsts de alpha
            auto& first_alpha = firsts[alpha];

            ContainerSet<string> local_first = compute_local_firsts(alpha, firsts);

            bool changed_alpha = first_alpha.hard_update(local_first);
            // bool changed_alpha = hard_update_container_set(first_alpha, local_first);
            bool changed_X = first_X.hard_update(local_first);
            // bool changed_X = hard_update_container_set(first_X, local_first);
            changed = changed || changed_alpha || changed_X;
            
        }
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