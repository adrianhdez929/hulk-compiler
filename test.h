#pragma once
#include "Automata/nfa.h"
#include "Automata/dfa.h"
#include "Automata/utils/ContainerSet.h"
#include "Automata/utils/aut_manipulation.h"
#include <iostream>
#include <vector>
#include "Automata/operations/operations.h"
#include "Grammar/grammar.h"
#include "Automata/state.h"
#include "Lexer/node.h"
#include "Automata/utils/utils.h"
#include "Parser/Item.h"
#include "Lexer/grammar_parser.h"
#include "Parser/LR1Parser.h"
#include <cassert>


int Item_test() {
    // Grammar g = GrammarParser::Parse("Lexer/grammar.txt");
    // // Crear un objeto Production
    // auto productions = g.Productions();
    // // Extract Production from variant
    // // std::shared_ptr<Production> prod;
    // //Production prod = productions[0];
    // // if (std::holds_alternative<Production>(productions[0])) {
    // //     prod = std::make_shared<Production>(std::get<Production>(productions[0]));
    // // } else if (std::holds_alternative<AttrProd>(productions[0])) {
    // //     // If you want to handle AttrProd, add code here
    // //     std::cerr << "First production is AttrProd, not Production." << std::endl;
    // //     return -1;
    // // }
    // std::shared_ptr<Production> production = std::make_shared<Production>(std::get<Production>(productions[0]));

    // // Crear un objeto Item
    // Item item(production, 0, ContainerSet<std::shared_ptr<Symbol>>());

    // // Probar el método Center
    // std::shared_ptr<Item> centerItem = item.Center();
    // std::cout << "Center Item: " << centerItem->ToString() << std::endl;

    // // Probar el método operator==
    // Item otroItem(production, 0, ContainerSet<std::shared_ptr<Symbol>>());
    // std::cout << "Son iguales? " << (item == otroItem) << std::endl;

    // // Probar el método hash
    // std::cout << "Hash: " << item.hash() << std::endl;

    // // Probar el método ToString
    // std::cout << "ToString: " << item.ToString() << std::endl;

    // // Probar NextItem
    // std::cout << "NextItem: " << item.NextItem()->ToString() << std::endl;
    // std::cout << "NextItem: " << item.NextItem()->NextItem()->ToString() << std::endl;
    return 0;
}
int lexer_node_test() {
    // Crear nodo simbolo
    // std::unique_ptr<Node> symbolNode = std::make_unique<SymbolNode>("a");
    // std::unique_ptr<Node> symbolNode2 = std::make_unique<SymbolNode>("b");
    // // std::unique_ptr<Node> epsilonNode = std::make_unique<EpsilonNode>();

    // std::unique_ptr<Node> symbolNode = std::make_unique<SymbolNode>("a");
    // std::unique_ptr<Node> symbolNode2 = std::make_unique<SymbolNode>("b");
    // std::unique_ptr<Node> unionNode = std::make_unique<UnionNode>(std::move(symbolNode), std::move(symbolNode2));

    // std::unique_ptr<Node> symbolNode3 = std::make_unique<SymbolNode>("a");
    // std::unique_ptr<Node> symbolNode4 = std::make_unique<SymbolNode>("b");
    // std::unique_ptr<Node> concatNode = std::make_unique<ConcatNode>(std::move(symbolNode3), std::move(symbolNode4));

    // std::unique_ptr<Node> symbolNode5 = std::make_unique<SymbolNode>("a");
    // std::unique_ptr<Node> closureNode = std::make_unique<ClosureNode>(std::move(symbolNode5));

    // //Automatas
    // std::shared_ptr<NFA> nfa = closureNode->evaluate();
    // std::shared_ptr<NFA> nfa2 = unionNode->evaluate();
    // std::shared_ptr<NFA> nfa3 = concatNode->evaluate();

    // Node* regex = new PositiveClosure(
    //     new UnionNode(
    //         std::make_unique<SymbolNode>("a"),
    //         std::make_unique<SymbolNode>("b")
    //     )
    // );
    // std::shared_ptr<NFA> nfa = regex->evaluate();
    // //convert to dfa

    // DFA dfa = nfa_to_dfa(*nfa);
    // dfa = automata_minimization(dfa);

    // std::cout << "NFA States: " << dfa.states() << std::endl;
    // std::cout << "NFA Start State: " << dfa.startState() << std::endl;
    // std::cout << "NFA Final States: ";
    // for (const auto& finalState : dfa.finalStates()) {
    //     std::cout << finalState << " ";
    // }
    // std::cout << std::endl;
    // const auto& transitions = dfa.getTransitionsMap();
    // for (const auto& transition : transitions) {
    //     std::cout << "Transition from state " << transition.first.first 
    //               << " with symbol '" << transition.first.second 
    //               << "' to states: ";
    //     for (const auto& dest : transition.second) {
    //         std::cout << dest << " ";
    //     }
    //     std::cout << std::endl;
    // }
    // delete regex;  // Limpiar memoria del nodo

    // std::vector<std::shared_ptr<NFA>> nfas = {nfa, nfa2, nfa3};

    // for (const auto& nfa : nfas) {
    //     std::cout << "NFA States: " << nfa->states() << std::endl;
    //     std::cout << "NFA Start State: " << nfa->startState() << std::endl;
    //     std::cout << "NFA Final States: ";
    //     for (const auto& finalState : nfa->finalStates()) {
    //         std::cout << finalState << " ";
    //     }
    //     std::cout << std::endl;

    //     const auto& transitions = nfa->getTransitionsMap();
    //     for (const auto& transition : transitions) {
    //         std::cout << "Transition from state " << transition.first.first 
    //                   << " with symbol '" << transition.first.second 
    //                   << "' to states: ";
    //         for (const auto& dest : transition.second) {
    //             std::cout << dest << " ";
    //         }
    //         std::cout << std::endl;
    //     }
    // }

    //Porbar el RangeNode
    // auto first = std::make_unique<SymbolNode>("a");
    // auto last = std::make_unique<SymbolNode>("z");
    // std::unique_ptr<Node> rangeNode = std::make_unique<RangeNode>(std::move(first), std::move(last));
    // // std::unique_ptr<Node> closure = std::make_unique<ClosureNode>(std::move(rangeNode));
    // std::shared_ptr<NFA> range_nfa = rangeNode->evaluate();
    // // std::shared_ptr<NFA> closure_nfa = closure->evaluate();
    // DFA dfa = nfa_to_dfa(*range_nfa);
    // dfa = automata_minimization(dfa);
    // std::cout << "Range NFA States: " << dfa.states() << std::endl;
    // std::cout << "Range NFA Start State: " << dfa.startState() << std::endl;
    // std::cout << "Range NFA Final States: ";
    // for (const auto& finalState : dfa.finalStates()) {
    //     std::cout << finalState << " ";
    // }
    // std::cout << std::endl;
    // const auto& rangeTransitions = dfa.getTransitionsMap();
    // for (const auto& transition : rangeTransitions) {
    //     std::cout << "Transition from state " << transition.first.first 
    //               << " with symbol '" << transition.first.second 
    //               << "' to states: ";
    //     for (const auto& dest : transition.second) {
    //         std::cout << dest << " ";
    //     }
    //     std::cout << std::endl;
    // }

    //probar el zero or one y el positive closure node
    // auto child = std::make_unique<SymbolNode>("a");
    // auto child2 = std::make_unique<SymbolNode>("b");
    // std::unique_ptr<Node> concatNode = std::make_unique<ConcatNode>(std::move(child), std::move(child2));
    // std::unique_ptr<Node> zeroOrOneNode = std::make_unique<ZeroOrOneNode>(std::move(concatNode));
    // std::unique_ptr<Node> positive_closure = std::make_unique<PositiveClosure>(std::move(zeroOrOneNode));

    // std::shared_ptr<NFA> pos_clo_nfa = positive_closure->evaluate();
    // // std::shared_ptr<NFA> zeroOrOneNfa = zeroOrOneNode->evaluate();

    // DFA dfa = nfa_to_dfa(*pos_clo_nfa);
    // dfa = automata_minimization(dfa);
    // std::cout << "Range NFA States: " << dfa.states() << std::endl;
    // std::cout << "Range NFA Start State: " << dfa.startState() << std::endl;
    // std::cout << "Range NFA Final States: ";
    // for (const auto& finalState : dfa.finalStates()) {
    //     std::cout << finalState << " ";
    // }
    // std::cout << std::endl;
    // const auto& rangeTransitions = dfa.getTransitionsMap();
    // for (const auto& transition : rangeTransitions) {
    //     std::cout << "Transition from state " << transition.first.first 
    //               << " with symbol '" << transition.first.second 
    //               << "' to states: ";
    //     for (const auto& dest : transition.second) {
    //         std::cout << dest << " ";
    //     }
    //     std::cout << std::endl;
    // }
    

    return 0;
}

int automata_tests() {
    // NFA::State startState = 0;
    // NFA::Transitions transitions = {
    //     {{0, "a"}, {1}}
    // };
    // NFA::Transitions transitions2 = {
    //     {{0, "b"}, {1}},
    // };
    // //Automata1
    // NFA nfa1(2, {1}, transitions, startState);
    // //Automata2
    // NFA nfa2(2, {1}, transitions2, startState);
    // //Automata3
    // // NFA nfa3 = union_nfa(nfa1, nfa2);
    // // NFA nfa3 = concat_nfa(nfa1, nfa2);
    // NFA nfa3 = closure_nfa(nfa1);

    // //Imprimir transiciones
    // cout << "Transiciones del automata 3:" << endl;
    // for (const auto& transition : nfa3.getTransitionsMap()) {
    //     cout << "Estado: " << transition.first.first << ", Simbolo: " << transition.first.second << ", Destinos: ";
    //     for (const auto& dest : transition.second) {
    //         cout << dest << " ";
    //     }
    //     cout << endl;
    // }
    // cout << "Estados finales: ";
    // for (const auto& finalState : nfa3.finalStates()) {
    //     cout << finalState << " ";
    // }
    // cout << endl;
    // cout << "Estado inicial: " << nfa3.startState() << endl;
    // cout << "Cantidad de estados: " << nfa3.states() << endl;
    // cout << endl;

    // automaton = DFA(states=5, finals=[4], transitions={
    //     (0,'a'): 1,
    //     (0,'b'): 2,
    //     (1,'a'): 1,
    //     (1,'b'): 3,
    //     (2,'a'): 1,
    //     (2,'b'): 2,
    //     (3,'a'): 1,
    //     (3,'b'): 4,
    //     (4,'a'): 1,
    //     (4,'b'): 2,
    //     })
    DFA::Transitions transitions = {
        {{0, "a"}, {1}},
        {{0, "b"}, {2}},
        {{1, "a"}, {1}},
        {{1, "b"}, {3}},
        {{2, "a"}, {1}},
        {{2, "b"}, {2}},
        {{3, "a"}, {1}},
        {{3, "b"}, {4}},
        {{4, "a"}, {1}},
        {{4, "b"}, {2}}
    };
    DFA dfa(5, {4}, transitions, 0);
    DFA mini = automata_minimization(dfa);
    cout << "Estados totales: " << mini.states() << endl;
    string cadena1 = "abb";
    string cadena2 = "ababbaabb";
    cout << "Cadena: " << cadena1 << ", Reconocida: " << mini.recognize(cadena1) << endl;
    cout << "Cadena: " << cadena2 << ", Reconocida: " << mini.recognize(cadena2) << endl;

    vector<string> cadenas = {"", "ab", "aaaaa", "bbbbb", "abbabababa"};
    for (string cadena : cadenas) {
        cout << "Cadena: " << cadena << ", Reconocida: " << mini.recognize(cadena) << endl;
    }

    return 0;
}

int state_automata_test() {
    DFA::Transitions transitions ={
        {{0, "a"}, {0}},
        {{0, "b"}, {1}},
        {{1, "a"}, {2}},
        {{1, "b"}, {1}},
        {{2, "a"}, {0}},
        {{2, "b"}, {1}}
    };
    DFA dfa(3, {2}, transitions, 0); // Reconoce cadenas sobre {a,b}* que terminan en "ba"
    State* start = State::from_nfa(dfa);
    // std::cout << "Estado inicial: " << start->id() << std::endl;
    // std::cout << "Transiciones del estado inicial:" << std::endl;
    // for (const auto& [symbol, states] : start->transitions()) {
    //     std::cout << "  Símbolo: " << symbol << " -> Estados: ";
    //     for (const auto* state : states) {
    //         std::cout << state->id() << " ";
    //     }
    //     std::cout << std::endl;
    // }
    std::cout << start->ToString() << std::endl;

    return 0;
}
// static bool compare_lookaheads(const ContainerSet<shared_ptr<Symbol>>& lhs, const ContainerSet<shared_ptr<Symbol>>& rhs) {
//     if (lhs.size() != rhs.size()) {
//         return false;
//     }

//     // Para cada símbolo en lhs, debe existir un símbolo equivalente en rhs
//     for (const auto& lhs_symbol : lhs.get_values()) {
//         bool found = false;
//         for (const auto& rhs_symbol : rhs.get_values()) {
//             if (lhs_symbol->Name() == rhs_symbol->Name()) {
//                 found = true;
//                 break;
//             }
//         }
//         if (!found) {
//             return false;  // No se encontró un símbolo equivalente en rhs
//         }
//     }

//     // Si llegamos aquí, significa que todos los símbolos de lhs tienen equivalentes en rhs
//     // Como los tamaños son iguales, esto implica que ambos conjuntos son iguales
//     return true;
// }
// static bool compare_items(const Item& lhs, const Item& rhs) {
//     string lhs_prod = lhs.production()->ToString();
//     string rhs_prod = rhs.production()->ToString();
//     if (lhs_prod != rhs_prod) {
//         return false; // Productions are not equal
//     }
//     if (lhs.pos() != rhs.pos()) {
//         return false;
//     }
//     if (!compare_lookaheads(lhs.lookaheads(), rhs.lookaheads())) {
//         return false;
//     }
//     return true;
// }
// bool update_container_set(ContainerSet<shared_ptr<Symbol>>& lhs, const ContainerSet<shared_ptr<Symbol>>& rhs) {
//     bool updated = false;
    
//     // Si lhs está vacío, simplemente agregamos todos los símbolos de rhs
//     if (lhs.get_values().empty()) {
//         for (const auto& symbol : rhs.get_values()) {
//             updated |= lhs.add(symbol);
//         }
//         return updated;
//     }
    
//     // Si lhs no está vacío, comprobamos cada símbolo de rhs
//     for (const auto& symbol : rhs.get_values()) {
//         bool exists = false;
        
//         // Verificar si el símbolo ya existe en lhs
//         for (const auto& existing_symbol : lhs.get_values()) {
//             if (symbol->Name() == existing_symbol->Name()) {
//                 exists = true;
//                 break;
//             }
//         }
        
//         // Si no existe, lo agregamos
//         if (!exists) {
//             updated |= lhs.add(symbol);
//         }
//     }
    
//     return updated;
// }

// // Realiza una actualización completa incluyendo epsilon
// inline bool hard_update_container_set(ContainerSet<std::shared_ptr<Symbol>>& lhs, const ContainerSet<std::shared_ptr<Symbol>>& rhs) {
//     bool updated = update_container_set(lhs, rhs);
    
//     // Actualizar epsilon solo si rhs lo tiene y lhs no
//     if (rhs.contains_epsilon() && !lhs.contains_epsilon()) {
//         lhs.set_epsilon(true);
//         updated = true;
//     }
    
//     return updated;
// }
ContainerSet<string> compute_local_firsts(const Sentence& alpha, const map<Sentence, ContainerSet<string>>& firsts, Grammar& G_) {
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

map<Sentence, ContainerSet<string>> compute_firsts(Grammar& G_) {
    map<Sentence, ContainerSet<string>> firsts;
    bool changed = true;
    
    // Inicializar primeros para terminales
    for (const auto& terminal : G_.Terminals()) {
        ContainerSet<string> cs;
        cs.add(terminal->Name());
        firsts[Sentence(terminal)] = cs;
    }
    firsts[Sentence(G_.GetEndOfFile())] = ContainerSet<string>().add(G_.GetEndOfFile()->Name());
    
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

            ContainerSet<string> local_first = compute_local_firsts(alpha, firsts, G_);

            bool changed_alpha = first_alpha.hard_update(local_first);
            // bool changed_alpha = hard_update_container_set(first_alpha, local_first);
            bool changed_X = first_X.hard_update(local_first);
            // bool changed_X = hard_update_container_set(first_X, local_first);
            changed = changed || changed_alpha || changed_X;
            
        }
    }
    return firsts;
};
vector<std::shared_ptr<Symbol>> get_symbols(vector<string> symbols, Grammar G_) {
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
        auto local_first = compute_local_firsts(Sentence(get_symbols(preview, G_)), firsts, G_);
        // update_container_set(lookaheads, local_first);
        lookaheads.update(local_first);
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
State BuildLR1Automaton(Grammar& G_) {
    assert(G_.GetStartSymbol()->productions.size() == 1 && "Grammar must be augmented");

    // Compute first sets
    auto firsts = compute_firsts(G_);
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

    // // Guardar todos los estados creados para liberarlos después
    // for (auto& [items, state] : visited) {
    //     if (state != &automaton) {  // No añadimos el estado automaton ya que se devuelve por valor
    //         automaton_states_.push_back(state);
    //     }
    // }
    
    return automaton;
}
void test_firsts_and_follows() {
    Grammar g = GrammarParser::Parse("Lexer/test_grammar.txt");
    // LR1Parser parser(g);
    g.Augment();

    // Compute firsts and follows
    // auto [firsts, sentence_firsts] = parser.compute_firsts();
    // auto follows = parser.compute_follows(firsts);

    // Print firsts
    // std::cout << "Firsts:" << std::endl;
    // for (const auto& [symbol, first_set] : firsts) {
    //     std::cout << symbol->Name() << ": ";
    //     for (const auto& f : first_set.get_values()) {
    //         std::cout << f->Name() << " ";
    //     }
    //     std::cout << std::endl;
    // }
    // // Print sentence firsts
    // std::cout << "Sentence Firsts:" << std::endl;
    // for (const auto& [sentence, first_set] : sentence_firsts) {
    //     std::cout << sentence.ToString() << ": ";
    //     for (const auto& f : first_set.get_values()) {
    //         std::cout << f->Name() << " ";
    //     }
    //     std::cout << std::endl;
    // }
    // // Print follows
    // std::cout << "Follows:" << std::endl;
    // for (const auto& [symbol, follow_set] : follows) {
    //     std::cout << symbol->Name() << ": ";
    //     for (const auto& f : follow_set.get_values()) {
    //         std::cout << f->Name() << " ";
    //     }
    //     std::cout << std::endl;
    // }
    // std::vector<std::shared_ptr<Terminal>> terminals = g.Terminals();
    // std::vector<std::shared_ptr<NonTerminal>> non_terminals = g.NonTerminals();



    shared_ptr<Symbol> plus;
    for (const auto& nt : g.Terminals()) {
        if (nt->Name() == "+") {
            plus = nt;
            break;
        }
    }
    shared_ptr<Symbol> eof = g.GetEndOfFile();
    vector<string> symbols = {eof->Name(), plus->Name()};
    Item item = Item(std::make_shared<Production>(g.Productions()[0]), 0, ContainerSet<string>(symbols));
    std::cout << "Item: " << item.ToString() << std::endl;
    std::cout << "Next Symbol: " << item.NextSymbol()->Name() << std::endl;
    std::cout << "Next Item: " << item.NextItem()->ToString() << std::endl;
    std::cout << "Center Item: " << item.Center()->ToString() << std::endl;
    std::cout << "Preview: " << std::endl;
    for (const auto& preview : item.Preview()) {
        std::cout << "  ";
        for (const auto& sym : preview) {
            std::cout << sym << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "Is Reduce Item: " << item.IsReduceItem() << std::endl;

    auto firsts = compute_firsts(g);
    auto expanded_item = expand(item, firsts, g);
    std::cout << "Expanded Items: " << std::endl;
    for (const auto& expanded : expanded_item) {
        std::cout << "  " << expanded.ToString() << std::endl;
    }


    vector<string> symbols1 = {eof->Name()};
    vector<string> symbols2 = {plus->Name()};
    vector<string> symbols3 = {plus->Name(), eof->Name()};
    vector<Item> items = {
        Item(std::make_shared<Production>(g.Productions()[0]), 0, ContainerSet<string>(symbols1)),
        Item(std::make_shared<Production>(g.Productions()[0]), 0, ContainerSet<string>(symbols2)),
        Item(std::make_shared<Production>(g.Productions()[0]), 1, ContainerSet<string>(symbols2)),
        Item(std::make_shared<Production>(g.Productions()[0]), 2, ContainerSet<string>(symbols2)),
        Item(std::make_shared<Production>(g.Productions()[0]), 2, ContainerSet<string>(symbols3)),
    };
    set<Item> compressed = compress(items);
    std::cout << "Compressed Items: " << std::endl;
    for (const auto& compressed_item : compressed) {
        std::cout << "  " << compressed_item.ToString() << std::endl;
    }

    shared_ptr<Item> temp = item.NextItem()->NextItem();
    Item item2 = *temp;
    std::cout << "Item2: " << item2.ToString() << std::endl;
    // Test closure_lr1
    vector<Item> clo_items = {item, item2};
    std::vector<Item> closure_items = closure_lr1(clo_items, firsts, g);
    std::cout << "Closure Items: " << std::endl;
    for (const auto& closure_item : closure_items) {
        std::cout << "  " << closure_item.ToString() << std::endl;
    }
    shared_ptr<Symbol> A;
    for (const auto& nt : g.NonTerminals()) {
        if (nt->Name() == "A") {
            A = nt;
            break;
        }
    }
    auto goto_items = goto_lr1({item}, A, firsts, false, g);
    std::cout << "Goto Items for A: " << std::endl;
    for (const auto& goto_item : goto_items) {
        std::cout << "  " << goto_item.ToString() << std::endl;
    }

    State automaton = BuildLR1Automaton(g);
    //Cadenas
    std::vector<std::string> test_string1 = {"E"};
    std::vector<std::string> test_string2 = {"int", "+", "int", "+", "A"};
    std::cout << "Testing automaton with strings: ";
    // std::cout << "State item:" << std::endl;
    // vector<Item> items2 = automaton.get_items();
    // for (const auto& item : items2) {
    //     std::cout << "  " << item.ToString() << std::endl;
    // }
    // True si reconoce la cadena
    // std::cout << "Test String 1: " << automaton.recognizes(test_string1) << std::endl;
    std::cout << "Test String 2: " << automaton.recognizes(test_string2) << std::endl;

}

int execute_all_tests() {
    Item_test();
    automata_tests();
    state_automata_test();
    return 0;
}

int execute_test() {
    test_firsts_and_follows();
    // test_grammar();
    // execute_all_tests();
    // lexer_node_test();
    // grammar_test();
    // Item_test();
    return 0; 
}