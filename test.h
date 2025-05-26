#pragma once
#include "Automata/nfa.h"
#include "Automata/dfa.h"
#include "Automata/utils/ContainerSet.h"
#include "Automata/utils/aut_manipulation.h"
#include <iostream>
#include <vector>
#include "Automata/operations/operations.h"
#include "Gramatica2/grammar.h"
#include "Automata/state.h"

int grammar_test() {
    Grammar g;
    auto E = g.SetNonTerminal("E", true);
    auto T = g.SetNonTerminal("T");
    auto plus = g.SetTerminal("+");
    auto num = g.SetTerminal("num");
    auto pipe = g.SetTerminal("|");
    auto symbol = g.SetTerminal("symbol");

    // Crear producciones
    E %= {
        E + plus + T,
        {
            // Acción para E (heredado, sintetizado)
            [](auto h, auto s) { 
                auto left = std::any_cast<double>(s[0]);
                auto right = std::any_cast<double>(s[2]);
                return left + right; 
            },
            // Acción para '+'
            [](auto h, auto s) { return 0.0; }, 
            // Acción para T
            [](auto h, auto s) { return s[3]; },
            // Acción para LHS (E)
            [](auto h, auto s) { return s[0]; }
        }
    };
    E %= {T.GetSentence(),
        {
            // Acción para E (heredado, sintetizado)
            [](auto h, auto s) { 
                auto left = std::any_cast<double>(s[0]);
                return left; 
            }
        }
    };

    string grammar_string = g.ToString();
    std::cout << grammar_string << std::endl;

    //ATTEMPT 2
    
    // Grammar g;
    // auto E = g.SetNonTerminal("E", true);
    // auto T = g.SetNonTerminal("T");
    // auto plus = g.SetTerminal("+");
    // auto num = g.SetTerminal("num");

    // Definición de un autómata no determinista (NFA)
    // NFA::Transitions transitions = {
    //     {{0, "a"}, {1, 2}},
    //     {{1, "b"}, {3}},
    //     {{2, "c"}, {4}},
    //     {{3, ""}, {5}}, // Epsilon transition
    //     {{4, "d"}, {5}}
    // };
    // NFA nfa(6, {5}, transitions, 0);

    // DFA::Transitions transitions2 = {
    //     {{0, "a"}, {0}},
    //     {{0, "b"}, {1}},
    //     {{1, "a"}, {2}},
    //     {{1, "b"}, {1}},
    //     {{2, "a"}, {0}},
    //     {{2, "b"}, {1}}
    // };
    // DFA dfa(3, {2}, transitions2, 0);//Reconoce cadenas sobre {a,b}* q terminan en "ba"

    // std::cout << dfa.recognize("ba") << std::endl; // 1
    // std::cout << dfa.recognize("aababbaba") << std::endl; // 1

    // std::cout << dfa.recognize("") << std::endl; // 0
    // std::cout << dfa.recognize("aabaa") << std::endl; // 0
    // std::cout << dfa.recognize("aababb") << std::endl; // 0

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
    state_automata_test();
    return 0; 
}