#include "LR1Parser.h"
#include <cassert>
#include <iostream>
#include "../Automata/state.h"

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
        local_first.update(firsts.at(symbols[0]));
        int i = 0;
        std::shared_ptr<Symbol> s = symbols[i];
        while (firsts.at(s).contains_epsilon()) {
            if (i < symbols.size() - 1) {
                i++;
                s = symbols[i];
                if (!firsts.at(s).contains_epsilon()) {
                    local_first.update(firsts.at(s));
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

            bool changed_alpha = first_alpha.hard_update(local_first);
            bool changed_X = first_X.hard_update(local_first);
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
    shared_ptr<Symbol> EOFile = G_.GetEndOfFile();
    follows[EOFile] = ContainerSet<shared_ptr<Symbol>>();
    follows[EOFile].add(EOFile);

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
                        changed |= follows[Y].update(symbol_firsts.at(beta));
                        if (symbol_firsts.at(beta).contains_epsilon()) {
                            changed |= follows[Y].update(follows_x);
                        }
                    }
                    if (i == n-1 && beta->IsNonTerminal()) {
                        changed |= follows.at(beta).update(follows_x);
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
        for (const auto& lookahead : lookaheads) {
            lookaheads.update(compute_local_firsts(Sentence(preview), firsts));
        }
        
    }
    if (lookaheads.contains_epsilon()) {
        throw std::runtime_error("Epsilon in lookaheads");
    }
    // for (const auto& p : next_symbol->Productions()) {
    //     expanded.push_back(Item(p, 0, lookaheads));
    // }
    for (auto prod : G_.Productions()) {
        if (prod.Left() == next_symbol) {
            // Adjust the constructor arguments to match the Item definition
            expanded.push_back(Item(std::make_shared<Production>(prod), 0, lookaheads));
        }
    }
    return expanded;
}
set<Item> LR1Parser::compress(const vector<Item>& items) {
    map<Item, ContainerSet<shared_ptr<Symbol>>> centers;
    for (const auto& item : items) {
        auto center = item.Center();
        if (centers.find(*center) == centers.end()) {
            centers[*center] = item.lookaheads();
        } else {
            centers[*center].update(item.lookaheads());
        }
    }
    set<Item> compressed;
    for (const auto& [item, lookaheads] : centers) {
        compressed.insert(Item(item.production(), item.pos(), lookaheads));
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
            // Expandir cada elemento y agregar los nuevos elementos al conjunto
            auto expanded_items = expand(item, firsts);
            new_items.insert(new_items.end(), expanded_items.begin(), expanded_items.end());
        }

        // Agregar los nuevos elementos al cierre si no están ya presentes
        for (const auto& new_item : new_items) {
            if (std::find(closure.begin(), closure.end(), new_item) == closure.end()) {
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
                visited[next_set] = next_state;
                pending.push(next_set);
            } else {
                next_state = visited[next_set];
            }

            current_state->add_transition(symbol->Name(), next_state);
        }
    }

    return automaton;
}