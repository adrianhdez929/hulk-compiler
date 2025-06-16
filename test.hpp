#pragma once
#include "Automata/nfa.hpp"
#include "Automata/dfa.hpp"
#include "Automata/utils/ContainerSet.hpp"
#include "Automata/utils/aut_manipulation.hpp"
#include <iostream>
#include <vector>
#include <variant>
#include "Automata/operations/operations.hpp"
#include "Grammar/grammar.hpp"
#include "Automata/state.hpp"
#include "Lexer/node.hpp"
#include "Automata/utils/utils.hpp"
#include "Parser/Item.hpp"
#include "Lexer/grammar_parser.hpp"
// #include "Parser/LR1Parser.h"
#include "Parser/SLR1Parser.hpp"
#include <cassert>
#include <stack>
#include "Lexer/Regex.hpp"
#include "Lexer/Lexer.hpp"

int test_grammar() {
    // Crear una gramática
    Grammar g;
    // Definir símbolos
    auto S = g.SetNonTerminal("S", true);
    auto A = g.SetNonTerminal("A");
    auto B = g.SetNonTerminal("B");
    auto a = g.SetTerminal("a");
    auto b = g.SetTerminal("b");
    
    // Definir producciones
    g.AddProduction(AttrProd(S, Sentence({A, B}), [](const std::vector<ElementType>& args) -> ElementType {
        return std::make_shared<UnionNode>(std::get<std::shared_ptr<Node>>(args[0]), std::get<std::shared_ptr<Node>>(args[1]));
    }));
    g.AddProduction(AttrProd(A, Sentence(a), [](const std::vector<ElementType>& args) -> ElementType {
        return std::make_shared<SymbolNode>(std::get<std::string>(args[0]));
    }));
    g.AddProduction(AttrProd(B, Sentence(b), [](const std::vector<ElementType>& args) -> ElementType {
        return std::make_shared<SymbolNode>(std::get<std::string>(args[0]));
    }));

    // Imprimir la gramática
    std::cout << "Grammar:\n" << g.ToString() << std::endl;
    
    return 0;
}

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

void test_parser() {
    Grammar g = GrammarParser::Parse("Lexer/grammar.txt");
    // LR1Parser parser(g);
    SLR1Parser parser(g,true);
    g.Augment();
    for (const auto& production : g.Productions()) {
        std::cout << production.ToString() << " id = " << production.get_id() << std::endl;
    }
    //                                           "(0|[1-9][0-9]*)(.[0-9]+)?"
    std::vector<std::string> tokens = {"(", "symbol", "|", "[", "symbol", "-", "symbol", "]", "[", "symbol", "-", "symbol", "]", "*", ")", "(", "symbol", "[", "symbol", "-", "symbol", "]", "+", ")", "?", "EOF"};
    auto [production_ids, actions] = parser.Parse(tokens);
    std::cout << "Productions:" << std::endl;
    for (const auto& production_id : production_ids) {
        std::cout << g.GetProduction(production_id).ToString() << std::endl;
    }
    std::cout << "Actions:" << std::endl;
    for (const auto& action : actions) {
        std::cout << action << std::endl;
    }


    //Compute firsts and follows
    // auto firsts = compute_firsts(g);

    // //Print firsts
    // std::cout << "Firsts:" << std::endl;
    // for (const auto& [symbol, first_set] : firsts) {
    //     std::cout << symbol.ToString() << ": ";
    //     std::cout << "{";
    //     for (const auto& f : first_set.get_values()) {
    //         std::cout << "'" << f << "', ";
    //     }
    //     std::cout << "} - " << first_set.contains_epsilon() << std::endl;
    // }

    // auto follows = compute_follows(firsts, g);
    // // Print follows
    // std::cout << "Follows:" << std::endl;
    // for (const auto& [symbol, follow_set] : follows) {
    //     std::cout << symbol.ToString() << ": ";
    //     for (const auto& f : follow_set.get_values()) {
    //         std::cout << f << " ";
    //     }
    //     std::cout << " -> " << follow_set.contains_epsilon() << std::endl;
    // }
    // std::vector<std::shared_ptr<Terminal>> terminals = g.Terminals();
    // std::vector<std::shared_ptr<NonTerminal>> non_terminals = g.NonTerminals();



    // shared_ptr<Symbol> plus;
    // for (const auto& nt : g.Terminals()) {
    //     if (nt->Name() == "+") {
    //         plus = nt;
    //         break;
    //     }
    // }
    // shared_ptr<Symbol> eof = g.GetEndOfFile();
    // vector<string> symbols = {eof->Name(), plus->Name()};
    // Item item = Item(std::make_shared<Production>(g.Productions()[0]), 0, ContainerSet<string>(symbols));
    // std::cout << "Item: " << item.ToString() << std::endl;
    // std::cout << "Next Symbol: " << item.NextSymbol()->Name() << std::endl;
    // std::cout << "Next Item: " << item.NextItem()->ToString() << std::endl;
    // std::cout << "Center Item: " << item.Center()->ToString() << std::endl;
    // std::cout << "Preview: " << std::endl;
    // for (const auto& preview : item.Preview()) {
    //     std::cout << "  ";
    //     for (const auto& sym : preview) {
    //         std::cout << sym << " ";
    //     }
    //     std::cout << std::endl;
    // }
    // std::cout << "Is Reduce Item: " << item.IsReduceItem() << std::endl;

    // auto firsts = compute_firsts(g);
    // auto expanded_item = expand(item, firsts, g);
    // std::cout << "Expanded Items: " << std::endl;
    // for (const auto& expanded : expanded_item) {
    //     std::cout << "  " << expanded.ToString() << std::endl;
    // }


    // vector<string> symbols1 = {eof->Name()};
    // vector<string> symbols2 = {plus->Name()};
    // vector<string> symbols3 = {plus->Name(), eof->Name()};
    // vector<Item> items = {
    //     Item(std::make_shared<Production>(g.Productions()[0]), 0, ContainerSet<string>(symbols1)),
    //     Item(std::make_shared<Production>(g.Productions()[0]), 0, ContainerSet<string>(symbols2)),
    //     Item(std::make_shared<Production>(g.Productions()[0]), 1, ContainerSet<string>(symbols2)),
    //     Item(std::make_shared<Production>(g.Productions()[0]), 2, ContainerSet<string>(symbols2)),
    //     Item(std::make_shared<Production>(g.Productions()[0]), 2, ContainerSet<string>(symbols3)),
    // };
    // set<Item> compressed = compress(items);
    // std::cout << "Compressed Items: " << std::endl;
    // for (const auto& compressed_item : compressed) {
    //     std::cout << "  " << compressed_item.ToString() << std::endl;
    // }

    // shared_ptr<Item> temp = item.NextItem()->NextItem();
    // Item item2 = *temp;
    // std::cout << "Item2: " << item2.ToString() << std::endl;
    // // Test closure_lr1
    // vector<Item> clo_items = {item, item2};
    // std::vector<Item> closure_items = closure_lr1(clo_items, firsts, g);
    // std::cout << "Closure Items: " << std::endl;
    // for (const auto& closure_item : closure_items) {
    //     std::cout << "  " << closure_item.ToString() << std::endl;
    // }
    // shared_ptr<Symbol> A;
    // for (const auto& nt : g.NonTerminals()) {
    //     if (nt->Name() == "A") {
    //         A = nt;
    //         break;
    //     }
    // }
    // auto goto_items = goto_lr1({item}, A, firsts, false, g);
    // std::cout << "Goto Items for A: " << std::endl;
    // for (const auto& goto_item : goto_items) {
    //     std::cout << "  " << goto_item.ToString() << std::endl;
    // }

    // State automaton = BuildLR1Automaton(g);
    // //Cadenas
    // std::vector<std::string> test_string1 = {"E"};
    // std::vector<std::string> test_string2 = {"int", "+", "int", "+", "A"};
    // std::cout << "Testing automaton with strings: ";
    // // std::cout << "State item:" << std::endl;
    // // vector<Item> items2 = automaton.get_items();
    // // for (const auto& item : items2) {
    // //     std::cout << "  " << item.ToString() << std::endl;
    // // }
    // // True si reconoce la cadena
    // // std::cout << "Test String 1: " << automaton.recognizes(test_string1) << std::endl;
    // std::cout << "Test String 2: " << automaton.recognizes(test_string2) << std::endl;

    // //Probar LR0 Automaton
    // State automaton = BuildLR0Automaton(g);
    // //Cadenas
    // std::vector<std::string> test_string1 = {"E"};
    // std::vector<std::string> test_string2 = {"E","*","F"};
    // std::cout << "Testing automaton with strings: ";
    // // std::cout << "State item:" << std::endl;
    // vector<Item> items2 = automaton.get_items();
    // for (const auto& item : items2) {
    //     std::cout << "  " << item.ToString() << std::endl;
    // }
    // // True si reconoce la cadena
    // std::cout << "Test String 1: " << automaton.recognizes(test_string1) << std::endl;
    // std::cout << "Test String 2: " << automaton.recognizes(test_string2) << std::endl;

}

// std::shared_ptr<Node> reverse_evaluate(std::queue<std::shared_ptr<AttrProd>>& productions, 
//                         const std::vector<std::string>& actions, 
//                         const std::vector<std::pair<std::string, std::string>>& token_names, Grammar& grammar) {
//     std::stack<string> token_stack;
//     std::map<std::string, std::stack<std::shared_ptr<Node>>> node_stack;
//     // Initialize the node stack for each non-terminal in the grammar
//     // for (const auto& nt : grammar.NonTerminals()) {
//     //     node_stack[nt->Name()].push(nullptr); // Push an empty node for each non-terminal
//     // }

//     int index = 0;
//     for (const auto& action : actions) {
//         if (action == SLR1Parser::SHIFT) {
//             // Shift operation
//             token_stack.push(token_names[index].second);
//             index++;
//         } else if (action == SLR1Parser::REDUCE) {
//             // Reduce operation
//             auto production = productions.front();
//             productions.pop();
//             auto attr = production->Attribute();
//             std::vector<ElementType> args;
//             auto prod_right = production->Right().Symbols();

//             // Debug information
//             std::cout << "Processing production: " << production->ToString() << std::endl;
//             std::cout << "Production right side has " << prod_right.size() << " symbols" << std::endl;
            
//             // Process symbols in reverse order since in LR parsing, we pop from stacks
//             // We need to build the arguments vector in reverse order, then reverse it
//             std::vector<ElementType> temp_args;
//             for (int i = prod_right.size() - 1; i >= 0; --i) {
//                 std::cout << "Processing symbol " << i << std::endl;
                
//                 // Check for null pointer
//                 if (!prod_right[i]) {
//                     std::cerr << "ERROR: Symbol at position " << i << " is null!" << std::endl;
//                     throw std::runtime_error("Null symbol in production");
//                 }
                
//                 std::cout << "Symbol name: " << prod_right[i]->Name() << std::endl;
                
//                 if (prod_right[i]->IsTerminal()) {
//                     // If it's a terminal, we pop from the token stack
//                     if (!token_stack.empty()) {
//                         auto token_value = token_stack.top();
//                         token_stack.pop();
//                         temp_args.push_back(token_value);
//                     } else {
//                         throw std::runtime_error("Token stack is empty during reduce operation.");
//                     }
//                 } else if (prod_right[i]->IsNonTerminal()) {
//                     // If it's a non-terminal, we pop from the node stack
//                     std::string nt_name = prod_right[i]->Name();
//                     if (!node_stack[nt_name].empty()) {
//                         auto node = node_stack[nt_name].top();
//                         node_stack[nt_name].pop();
//                         temp_args.push_back(node);
//                     } else {
//                         throw std::runtime_error("Node stack is empty for non-terminal during reduce operation.");
//                     }
//                 }
//             }
//             // Reverse the arguments to get them in the correct order for semantic actions
//             for (int i = temp_args.size() - 1; i >= 0; --i) {
//                 args.push_back(temp_args[i]);
//             }
//             // Call the attribute function with the collected arguments
//             auto result = attr(args);
//             // std::cout << "Attribute function returned: ";
//             // if (std::holds_alternative<std::string>(result)) {
//             //     std::cout << std::get<std::string>(result) << std::endl;
//             // } else if (std::holds_alternative<std::shared_ptr<Node>>(result)) {
//             //     std::cout << "Node" << std::endl;
//             //     auto object = std::get<std::shared_ptr<Node>>(result);
//             //     if (object) {
//             //         auto nfa = object->evaluate();
//             //         std::cout << "Node object is valid." << std::endl;
//             //     } else {
//             //         std::cerr << "ERROR: Node object is null!" << std::endl;
//             //         throw std::runtime_error("Node object is null after attribute function call.");
//             //     }
//             // } else {
//             //     std::cout << "Unknown type" << std::endl;
//             // }
//             // Check if the result is a Node
//             if (std::holds_alternative<std::shared_ptr<Node>>(result)) {
//                 // If the result is a Node, we push it onto the node stack for the left-hand side non-terminal
//                 auto left_nt = production->Left();
//                 if (left_nt) {
//                     node_stack[left_nt->Name()].push(std::get<std::shared_ptr<Node>>(result));
//                 } else {
//                     throw std::runtime_error("Left-hand side of production is not a NonTerminal.");
//                 }
//             } else {
//                 throw std::runtime_error("Attribute function did not return a Node.");
//             }
//         } else if (action == SLR1Parser::OK) {
//             // Accept operation
//             std::cout << "Accept operation reached" << std::endl;
//             if (!productions.empty()) {
//                 auto production = productions.front();
//                 productions.pop();
//                 auto attr = production->Attribute();
//                 std::vector<ElementType> args;
//                 auto prod_right = production->Right().Symbols();

//                 // Debug information
//                 std::cout << "Processing production: " << production->ToString() << std::endl;
//                 std::cout << "Production right side has " << prod_right.size() << " symbols" << std::endl;
                
//                 // Process symbols in reverse order since in LR parsing, we pop from stacks
//                 // We need to build the arguments vector in reverse order, then reverse it
//                 std::vector<ElementType> temp_args;
//                 for (int i = prod_right.size() - 1; i >= 0; --i) {
//                     std::cout << "Processing symbol " << i << std::endl;
                    
//                     // Check for null pointer
//                     if (!prod_right[i]) {
//                         std::cerr << "ERROR: Symbol at position " << i << " is null!" << std::endl;
//                         throw std::runtime_error("Null symbol in production");
//                     }
                    
//                     std::cout << "Symbol name: " << prod_right[i]->Name() << std::endl;
                    
//                     if (prod_right[i]->IsTerminal()) {
//                         // If it's a terminal, we pop from the token stack
//                         if (!token_stack.empty()) {
//                             auto token_value = token_stack.top();
//                             token_stack.pop();
//                             temp_args.push_back(token_value);
//                         } else {
//                             throw std::runtime_error("Token stack is empty during reduce operation.");
//                         }
//                     } else if (prod_right[i]->IsNonTerminal()) {
//                         // If it's a non-terminal, we pop from the node stack
//                         std::string nt_name = prod_right[i]->Name();
//                         if (!node_stack[nt_name].empty()) {
//                             auto node = node_stack[nt_name].top();
//                             node_stack[nt_name].pop();
//                             temp_args.push_back(node);
//                         } else {
//                             throw std::runtime_error("Node stack is empty for non-terminal during reduce operation.");
//                         }
//                     }
//                 }
//                 // Reverse the arguments to get them in the correct order for semantic actions
//                 for (int i = temp_args.size() - 1; i >= 0; --i) {
//                     args.push_back(temp_args[i]);
//                 }
//                 // Call the attribute function with the collected arguments
//                 auto result = attr(args);
//                 // std::cout << "Attribute function returned: ";
//                 // if (std::holds_alternative<std::string>(result)) {
//                 //     std::cout << std::get<std::string>(result) << std::endl;
//                 // } else if (std::holds_alternative<std::shared_ptr<Node>>(result)) {
//                 //     std::cout << "Node" << std::endl;
//                 //     auto object = std::get<std::shared_ptr<Node>>(result);
//                 //     if (object) {
//                 //         auto nfa = object->evaluate();
//                 //         std::cout << "Node object is valid." << std::endl;
//                 //     } else {
//                 //         std::cerr << "ERROR: Node object is null!" << std::endl;
//                 //         throw std::runtime_error("Node object is null after attribute function call.");
//                 //     }
//                 // } else {
//                 //     std::cout << "Unknown type" << std::endl;
//                 // }
//                 // Check if the result is a Node
//                 if (std::holds_alternative<std::shared_ptr<Node>>(result)) {
//                     // If the result is a Node, we push it onto the node stack for the left-hand side non-terminal
//                     auto left_nt = production->Left();
//                     if (left_nt) {
//                         node_stack[left_nt->Name()].push(std::get<std::shared_ptr<Node>>(result));
//                     } else {
//                         throw std::runtime_error("Left-hand side of production is not a NonTerminal.");
//                     }
//                 } else {
//                     throw std::runtime_error("Attribute function did not return a Node.");
//                 }
//             }
//             break;
//         }
//     }
    
//     std::cout << "Parsing completed. Checking node stacks:" << std::endl;
//     for (const auto& [name, stack] : node_stack) {
//         std::cout << "Stack for " << name << " has " << stack.size() << " elements" << std::endl;
//     }
    
//     // At the end, we should have a single node for the start symbol
//     // auto start_symbol = grammar.GetStartSymbol();
//     // if (node_stack.find(*start_symbol) == node_stack.end() || node_stack[*start_symbol].empty()) {
//     //     throw std::runtime_error("Node stack for start symbol is empty after parsing.");
//     // }
//     std::string start_name = grammar.GetStartSymbol()->Name();
//     if (node_stack.find(start_name) == node_stack.end() || node_stack[start_name].empty()) {
//         throw std::runtime_error("Node stack for start symbol is empty after parsing.");
//     }
//     std::cout << "Final node stack for start symbol '" << start_name << "' has " 
//               << node_stack[start_name].size() << " elements." << std::endl;
//     return node_stack[start_name].top();
// }

void lexer_ast_test() {
    // using Token = std::pair<std::string, std::string>; // Pair of token type and value
    // vector<string> input = split("( 0 | [ 1 - 9 ] [ 0 - 9 ] * ) ( . [ 0 - 9 ] + ) ?", ' ');
    // // vector<string> input = split("( a | b )", ' ');
    // // vector<string> token_names = {"(", "symbol", "|", "symbol", ")", "EOF"};
    // vector<string> token_names = {"(", "symbol", "|", "[", "symbol", "-", "symbol", "]", "[", "symbol", "-", "symbol", "]", "*", ")", "(", "symbol", "[", "symbol", "-", "symbol", "]", "+", ")", "?", "EOF"};

    // std::vector<Token> tokens;
    // assert(input.size() == token_names.size() - 1);
    // for (size_t i = 0; i < input.size(); ++i) {
    //     tokens.emplace_back(token_names[i], input[i]);
    // }
    // tokens.emplace_back(make_pair("EOF", "EOF"));


    //= {
    //     {"(", "("},         // (
    //     {"symbol", "0"},    // 0
    //     {"|", "|"},         // |
    //     {"[", "["},         // [
    //     {"symbol", "1"},    // 1
    //     {"-", "-"},         // -
    //     {"symbol", "9"},    // 9
    //     {"]", "]"},         // ]
    //     {"[", "["},         // [
    //     {"symbol", "0"},    // 0
    //     {"-", "-"},         // -
    //     {"symbol", "9"},    // 9
    //     {"]", "]"},         // ]
    //     {"*", "*"},         // *
    //     {")", ")"},         // )
    //     {"(", "("},         // (
    //     {"symbol", "."},    // .
    //     {"[", "["},         // [
    //     {"symbol", "0"},    // 0
    //     {"-", "-"},         // -
    //     {"symbol", "9"},    // 9
    //     {"]", "]"},         // ]
    //     {"+", "+"},         // +
    //     {")", ")"},         // )
    //     {"?", "?"},         // ?
    //     {"EOF", "EOF"}      // EOF
    // };
    std::vector<std::pair<std::string, std::string>> table = {
        {"string", "\"([\\x20-!#-\\x7e])*\""},
        {"number", "(0|[1-9][0-9]*)(.[0-9]+)?"}, // Regular expression for numbers
        {"bool", "true|false"},  // Regular expression for boolean values
        {"type_id", "[A-Z][_a-zA-Z0-9]*"},
        {"var_id", "[_a-z][_a-zA-Z0-9]*"},
        {"space", " +"}, // Regular expression for spaces
        // Regular expression for identifiers
        {"(", "\\("},            // Left parenthesis
        {")", "\\)"}            // Right parenthesis
        // {"-", "-"},              // Minus sign
        // {"EOF", "EOF"}           // End of file token
    };
    Grammar g = GrammarParser::Parse("Lexer/grammar.txt");
    SLR1Parser parser(g);
    Lexer lexer(table, g, parser);
    auto tokens = lexer.tokenize("Func fibonacci(\"soy el animal?\", 0.543)"); //( 1 - 2 )");
    std::cout << "Tokens:" << std::endl;
    for (const auto& token : tokens) {
        std::cout << "Type: " << token.first << ", Value: " << token.second << std::endl;
    }
    // Regex regex("(0|[1-9][0-9]*)(.[0-9]+)?", g, parser);

    // DFA dfa = regex.Automaton();
    // cout << "DFA States: " << dfa.states() << endl;
    // cout << "DFA Start State: " << dfa.startState() << endl;
    // cout << "DFA Final States: ";
    // for (const auto& finalState : dfa.finalStates()) {
    //     cout << finalState << " ";
    // }
    // cout << endl;
    // const auto& transitions = dfa.getTransitionsMap();
    // for (const auto& transition : transitions) {
    //     cout << "Transition from state " << transition.first.first 
    //           << " with symbol '" << transition.first.second 
    //           << "' to states: ";
    //     for (const auto& dest : transition.second) {
    //         cout << dest << " ";
    //     }
    //     std::cout << std::endl;
    // }

    // auto [raw_productions, actions] = parser.Parse(token_names);
    // // Print productions
    // std::cout << "Productions:" << std::endl;
    // for (const auto& production : raw_productions) {
    //     std::cout << production.ToString() << std::endl;
    // }
    // // Print actions
    // std::cout << "Actions:" << std::endl;
    // for (const auto& action : actions) {
    //     std::cout << action << std::endl;
    // }
    // cout << raw_productions.size() << " productions" << endl;
    // std::queue<std::shared_ptr<AttrProd>> productions;
    // vector<std::shared_ptr<AttrProd>> productions_vector;
    // for (const auto& production : raw_productions) {
    //     // Use the production ID to get the corresponding AttrProd
    //     auto attr_prod = std::make_shared<AttrProd>(g.GetProductionByID(production.get_id()));
    //     productions.push(attr_prod);
    //     productions_vector.push_back(attr_prod);
    // }
    // cout << "Productions stack size: " << productions.size() << endl;
    // while (!productions.empty()) {
    //     auto production = productions.top();
    //     productions.pop();
    //     std::cout << "Production: " << production->ToString() << std::endl;
    // }
    // for (const auto& production : productions_vector) {
    //     std::cout << "Production: " << production->ToString() << std::endl;
    // }
    // Reverse evaluate the productions and actions to get the AST



    // std::shared_ptr<Node> ast = reverse_evaluate(productions, actions, tokens, g);

    // auto nfa = ast->evaluate(); // Evaluar el nodo para obtener el NFA
    // std::cout << "NFA States: " << nfa->states() << std::endl;
    // std::cout << "NFA Start State: " << nfa->startState() << std::endl; 
    // std::cout << "NFA Final States: ";
    // for (const auto& finalState : nfa->finalStates()) {
    //     std::cout << finalState << " ";
    // }
    // std::cout << std::endl;
    // const auto& transitions = nfa->getTransitionsMap();
    // for (const auto& transition : transitions) {
    //     std::cout << "Transition from state " << transition.first.first 
    //               << " with symbol '" << transition.first.second 
    //               << "' to states: ";
    //     for (const auto& dest : transition.second) {
    //         std::cout << dest << " ";
    //     }
    //     std::cout << std::endl;
    // }



    //Printear gramática
    // cout << g.ToString() << endl;

    // auto prod = g.Productions()[0];
    // auto fun = prod.Attribute(); // Esto debería devolver un atributo de producción, si existe
    
    // // Create a SymbolNode and pass it as part of a vector
    // auto symbolNode = std::make_shared<SymbolNode>("a");
    // std::vector<ElementType> args = {symbolNode};
    // auto result = fun(args);
    
    // // Since the first production is E -> T { $$ = $1 }, it should return the first argument
    // // which in our case would be a SymbolNode as Node
    // auto node1 = result;

    // auto nfa1 = std::get<std::shared_ptr<Node>>(node1)->evaluate(); // Evaluar el nodo para obtener el NFA
    // cout << "NFA States: " << nfa1->states() << std::endl;
    // cout << "NFA Start State: " << nfa1->startState() << std::endl;
    // cout << "NFA Final States: ";
    // for (const auto& finalState : nfa1->finalStates()) {
    //     cout << finalState << " ";
    // }
    // cout << std::endl;
}
// Pair of token type and value


int execute_all_tests() {
    Item_test();
    automata_tests();        // {"-", "-"},              // Minus sign
        // {"EOF", "EOF"}           // End of file token

    state_automata_test();
    return 0;
}

int execute_test() {
    // lexer_ast_test();
    // test_parser();
    // test_grammar();
    // execute_all_tests();
    // lexer_node_test();
    test_grammar();
    // Item_test();
    return 0; 
}