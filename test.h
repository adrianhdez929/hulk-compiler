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
int grammar_test() {
    // Grammar g;
    // auto E = g.SetNonTerminal("E", true);
    // auto T = g.SetNonTerminal("T");
    // auto plus = g.SetTerminal("+");
    // auto num = g.SetTerminal("num");
    // auto pipe = g.SetTerminal("|");
    // auto symbol = g.SetTerminal("symbol");

    // // Crear producciones
    // E %= {
    //     E + plus + T,
    //     {
    //         // Acción para E (heredado, sintetizado)
    //         [](auto h, auto s) { 
    //             auto left = std::any_cast<double>(s[0]);
    //             auto right = std::any_cast<double>(s[2]);
    //             return left + right;  
    //         }
    //     }
    // };
    // E %= {T.GetSentence(),
    //     {
    //         // Acción para E (heredado, sintetizado)
    //         [](auto h, auto s) { 
    //             auto left = std::any_cast<double>(s[0]);
    //             return left; 
    //         }
    //     }
    // };

    // string grammar_string = g.ToString();
    // std::cout << grammar_string << std::endl;

    // AQUIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIIII
    // Grammar g;
    // auto E = g.SetNonTerminal("E", true);
    // auto T = g.SetNonTerminal("T");
    // auto F = g.SetNonTerminal("F");
    // auto A = g.SetNonTerminal("A");
    // auto S = g.SetNonTerminal("S");

    // auto pipe = g.SetTerminal("|");
    // auto star = g.SetTerminal("*");
    // auto opar = g.SetTerminal("(");
    // auto cpar = g.SetTerminal(")");
    // auto symbol = g.SetTerminal("symbol");
    // auto epsilon = g.SetTerminal("ε");
    // auto quest = g.SetTerminal("?");
    // auto plus = g.SetTerminal("+");
    // auto minus = g.SetTerminal("-");
    // auto obra = g.SetTerminal("[");
    // auto cbra = g.SetTerminal("]");

    // E %= {
    //     E + pipe + T, 
    //     { 
    //         [](auto h, auto s){ 
    //             return std::make_shared<UnionNode>(std::move(s[1]),std::move(s[3]));
    //         }
    //     }
    // };
    // E %= AttributeProduction::ProdDef{
        // E + pipe + T, 
        // { [](auto h, auto s) { 
        //     return unique_ptr<Node>(make_unique<UnionNode>(std::move(s[1]), std::move(s[3]))); 
        // }}
    // };
    // E %= { T.GetSentence(), { [](auto h, auto s) { return s[1]; }}};
    // E %= { 
    //     E + pipe + T, 
    //     { 
    //         [](auto h, auto s) { 
    //             return unique_ptr<Node>(make_unique<UnionNode>(std::move(s[1]), std::move(s[3]))); 
    //         }
    //     }
    // };

    // T %= { F.GetSentence(), { [](auto h, auto s) { return s[1]; }}};
    // T %= { T + F, { [](auto h, auto s) { return unique_ptr<Node>(make_unique<ConcatNode>(std::move(s[1]), std::move(s[2]))); }}};

    // F %= { A.GetSentence(), { [](auto h, auto s) { return s[1]; }}};
    // F %= { A + star, { [](auto h, auto s) { return unique_ptr<Node>(std::make_unique<ClosureNode>(std::move(s[1]))); }}};
    // F %= { A + quest, { [](auto h, auto s) { return unique_ptr<Node>(std::make_unique<ZeroOrOneNode>(std::move(s[1]))); }}};
    // F %= { A + plus, { [](auto h, auto s) { return unique_ptr<Node>(std::make_unique<PositiveClosure>(std::move(s[1]))); }}};

    // A %= { symbol.GetSentence(), { [](auto h, auto s) { return unique_ptr<Node>(std::make_unique<SymbolNode>(s[1]->Name())); }}};
    // A %= { epsilon.GetSentence(), { [](auto h, auto s) { return unique_ptr<Node>(std::make_unique<EpsilonNode>()); }}};
    // A %= { opar + E + cpar, { [](auto h, auto s) { return s[2]; }}};
    // A %= { obra + S + cbra, { [](auto h, auto s) { return unique_ptr<Node>(std::make_unique<StringClassNode>(s[2])); }}};

    // S %= { symbol.GetSentence(), { [](auto h, auto s) { 
    //     auto symbols = vector<unique_ptr<SymbolNode>>();
    //     symbols.push_back(std::make_unique<SymbolNode>(s[1]->Name()));
    //     return symbols;
    // }}};
    // S %= { S + symbol, { [](auto h, auto s) { 
    //     auto symbols = vector<unique_ptr<SymbolNode>>();
    //     symbols.push_back(std::make_unique<SymbolNode>(s[2]->Name()));
    //     auto new_vector = vector_concat(std::move(s[1]), std::move(symbols));
    //     return new_vector;
    // }}};
    // S %= { symbol + minus + symbol, { [](auto h, auto s) { 
    //     std::unique_ptr<SymbolNode> start = std::make_unique<SymbolNode>(s[1]->Name());
    //     std::unique_ptr<SymbolNode> end = std::make_unique<SymbolNode>(s[3]->Name());
    //     auto symbols = vector<unique_ptr<SymbolNode>>();
    //     for (char c = start->Name()[0]; c <= end->Name()[0]; ++c) {
    //         symbols.push_back(std::make_unique<SymbolNode>(std::string(1, c)));
    //     }
    //     return symbols;
    // }}};
    // S %= { S + symbol + minus + symbol, { [](auto h, auto s) { 
    //     std::unique_ptr<SymbolNode> start = std::make_unique<SymbolNode>(s[1]->Name());
    //     std::unique_ptr<SymbolNode> end = std::make_unique<SymbolNode>(s[3]->Name());
    //     auto symbols = vector<unique_ptr<SymbolNode>>();
    //     for (char c = start->Name()[0]; c <= end->Name()[0]; ++c) {
    //         symbols.push_back(std::make_unique<SymbolNode>(std::string(1, c)));
    //     }
    //     auto new_vector = vector_concat(std::move(s[0]), std::move(symbols));
    //     return new_vector;
    // }}};








    //concatenar vectores
    // std::unique_ptr<Node> symbolNode = std::make_unique<SymbolNode>("a");
    // std::unique_ptr<Node> symbolNode2 = std::make_unique<SymbolNode>("b");
    // vector<unique_ptr<Node>> vec1;
    // vec1.push_back(std::move(symbolNode));
    // vector<unique_ptr<Node>> vec2;
    // vec2.push_back(std::move(symbolNode2));
    // vector<unique_ptr<Node>> c = concatenar_vectores(std::move(vec1), std::move(vec2));
    // Imprimir los elementos del vector resultante

    
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

int execute_all_tests() {
    grammar_test();
    automata_tests();
    state_automata_test();
    return 0;
}

int execute_test() {
    // execute_all_tests();
    // lexer_node_test();
    grammar_test();
    return 0; 
}