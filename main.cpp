#include "Automata/nfa.h"
#include "Automata/dfa.h"
#include "Automata/utils/ContainerSet.h"
#include "Automata/utils/aut_manipulation.h"
#include <iostream>
#include <vector>
#include "Gramatica2/grammar.h"

using namespace std;
// using namespace manipulation;

int main() {

    // // Definición de un autómata no determinista (NFA)
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


    // NFA::State startState = 0;
    // set<NFA::State> finalStates = {3, 5};
    // NFA::Transitions transitions = {
    //     {{0,  ""}, {1,2}},
    //     {{1,  ""}, {3}},
    //     {{1, "b"}, {4}},
    //     {{2, "a"}, {4}},
    //     {{3, "c"}, {3}},
    //     {{4,  ""}, {5}},
    //     {{5, "d"}, {5}}
    // };
    
    // NFA nfa(6, finalStates, transitions, startState);
    
    // // Convert NFA to DFA
    // DFA dfa = nfa_to_dfa(nfa);
    
    // // Test the DFA with some input strings
    // // vector<string> testStrings = {"abc", "ab", "a", "bc", "c"};
    // //Cadenas q debe reconocer:
    // vector<string> testStrings = {"", "a", "b", "cccccc", "adddd", "bdddd"};

    // //Comparar las cadenas q reconoce el NFA con las q reconoce el DFA
    // for (const auto& str : testStrings) {
    //     cout << "Testing string: " << str << endl;
    //     if (nfa_recognize(nfa, str)) {
    //         cout << "Accepted" << endl;
    //     } else {
    //         cout << "Rejected" << endl;
    //     }
    // }
    // cout << endl;
    // for (const auto& str : testStrings) {
    //     cout << "Testing string: " << str << endl;
    //     if (dfa.recognize(str)) {
    //         cout << "Accepted" << endl;
    //     } else {
    //         cout << "Rejected" << endl;
    //     }
    // }

    // cout << endl;

    // //Cadenas q no debe reconocer
    // // ('dddddd')
    // // ('cdddd')
    // // ('aa')
    // // ('ab')
    // // ('ddddc')
    // vector<string> testStrings2 = {"dddddd", "cdddd", "aa", "ab", "ddddc"};

    // // Comparar las cadenas q no reconoce el NFA con las q no reconoce el DFA
    // for (const auto& str : testStrings2) {
    //     cout << "Testing string: " << str << endl;
    //     if (nfa_recognize(nfa, str)) {
    //         cout << "Accepted" << endl;
    //     } else {
    //         cout << "Rejected" << endl;
    //     }
    // }
    // cout << endl;
    // for (const auto& str : testStrings2) {
    //     cout << "Testing string: " << str << endl;
    //     if (dfa.recognize(str)) {
    //         cout << "Accepted" << endl;
    //     } else {
    //         cout << "Rejected" << endl;
    //     }
    // }







    Grammar g;
    auto E = g.SetNonTerminal("E", true);
    auto T = g.SetNonTerminal("T");
    auto plus = g.SetTerminal("+");
    auto num = g.SetTerminal("num");
    auto pipe = g.SetTerminal("|");
    auto symbol = g.SetTerminal("symbol");

    // // Crear producciones
    // E %= {
    //     E + plus + T,
    //     {
    //         // Acción para E (heredado, sintetizado)
    //         [](auto h, auto s) { 
    //             auto left = std::any_cast<double>(s[0]);
    //             auto right = std::any_cast<double>(s[2]);
    //             return left + right; 
    //         },
    //         // Acción para '+'
    //         [](auto h, auto s) { return 0.0; }, 
    //         // Acción para T
    //         [](auto h, auto s) { return s[3]; },
    //         // Acción para LHS (E)
    //         [](auto h, auto s) { return s[0]; }
    //     }
    // };
    E %= {T,}

    string grammar_string = g.ToString();
    std::cout << grammar_string << std::endl;

    return 0;
}