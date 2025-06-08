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
// #include "Parser/LR1Parser.h"
#include "Parser/SLR1Parser.h"
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
    auto [productions, actions] = parser.Parse(tokens);
    std::cout << "Productions:" << std::endl;
    for (const auto& production : productions) {
        std::cout << production.ToString() << std::endl;
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

int execute_all_tests() {
    Item_test();
    automata_tests();
    state_automata_test();
    return 0;
}

int execute_test() {
    test_parser();
    // test_grammar();
    // execute_all_tests();
    // lexer_node_test();
    // grammar_test();
    // Item_test();
    return 0; 
}