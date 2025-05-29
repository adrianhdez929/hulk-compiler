// #pragma once
// #include "../Grammar/grammar.h"
// #include "../Lexer/node.h"
// #include "../Automata/utils/utils.h"
// #include <iostream>

// using namespace std;
// Función para definir la gramática del lexer
// Grammar define_lexer_grammar() {
//     // Definición de la gramática
//     Grammar g;
//     auto E = g.SetNonTerminal("E", true);
//     auto T = g.SetNonTerminal("T");
//     auto F = g.SetNonTerminal("F");
//     auto A = g.SetNonTerminal("A");
//     auto S = g.SetNonTerminal("S");

//     auto pipe = g.SetTerminal("|");
//     auto star = g.SetTerminal("*");
//     auto opar = g.SetTerminal("(");
//     auto cpar = g.SetTerminal(")");
//     auto symbol = g.SetTerminal("symbol");
//     auto epsilon = g.SetTerminal("ε");
//     auto quest = g.SetTerminal("?");
//     auto plus = g.SetTerminal("+");
//     auto minus = g.SetTerminal("-");
//     auto obra = g.SetTerminal("[");
//     auto cbra = g.SetTerminal("]");

//     E %= { T->GetSentence(), { [](auto h, auto s) { return s[1]; }}};
//     E %= { E + pipe + T, { [](auto h, auto s) { return std::make_shared<UnionNode>(std::move(s[1]),std::move(s[3])); }}};

//     T %= { F.GetSentence(), { [](auto h, auto s) { return s[1]; }}};
//     T %= { T + F, { [](auto h, auto s) { return unique_ptr<Node>(make_unique<ConcatNode>(std::move(s[1]), std::move(s[2]))); }}};

//     F %= { A.GetSentence(), { [](auto h, auto s) { return s[1]; }}};
//     F %= { A + star, { [](auto h, auto s) { return unique_ptr<Node>(std::make_unique<ClosureNode>(std::move(s[1]))); }}};
//     F %= { A + quest, { [](auto h, auto s) { return unique_ptr<Node>(std::make_unique<ZeroOrOneNode>(std::move(s[1]))); }}};
//     F %= { A + plus, { [](auto h, auto s) { return unique_ptr<Node>(std::make_unique<PositiveClosure>(std::move(s[1]))); }}};

//     A %= { symbol.GetSentence(), { [](auto h, auto s) { return unique_ptr<Node>(std::make_unique<SymbolNode>(s[1]->Name())); }}};
//     A %= { epsilon.GetSentence(), { [](auto h, auto s) { return unique_ptr<Node>(std::make_unique<EpsilonNode>()); }}};
//     A %= { opar + E + cpar, { [](auto h, auto s) { return s[2]; }}};
//     A %= { obra + S + cbra, { [](auto h, auto s) { return s[2]; }}};//{ return unique_ptr<Node>(std::make_unique<StringClassNode>(s[2])); }}};

//     S %= { symbol.GetSentence(), { [](auto h, auto s) { return unique_ptr<Node>(std::make_unique<SymbolNode>(s[1]->Name())); }}};
//     S %= { symbol.GetSentence(), { [](auto h, auto s) { 
//         auto symbols = vector<unique_ptr<SymbolNode>>();
//         symbols.push_back(std::make_unique<SymbolNode>(s[1]->Name()));
//         return symbols;
//     }}};
//     S %= { S + symbol, { [](auto h, auto s) { return unique_ptr<Node>(std::make_unique<ConcatNode>(s[1], s[2])); }}};
//         auto symbols = vector<unique_ptr<SymbolNode>>();
//         symbols.push_back(std::make_unique<SymbolNode>(s[2]->Name()));
//         auto new_vector = vector_concat(std::move(s[1]), std::move(symbols));
//         return new_vector;
//     }}};
//     S %= { symbol + minus + symbol, { [](auto h, auto s) { 
//         std::unique_ptr<SymbolNode> start = std::make_unique<SymbolNode>(s[1]->Name());
//         std::unique_ptr<SymbolNode> end = std::make_unique<SymbolNode>(s[3]->Name());
//         auto symbols = vector<unique_ptr<SymbolNode>>();
//         for (char c = start->Name()[0]; c <= end->Name()[0]; ++c) {
//             symbols.push_back(std::make_unique<SymbolNode>(std::string(1, c)));
//         }
//         return symbols;
//     }}};
//     S %= { S + symbol + minus + symbol, { [](auto h, auto s) { 
//         std::unique_ptr<SymbolNode> start = std::make_unique<SymbolNode>(s[1]->Name());
//         std::unique_ptr<SymbolNode> end = std::make_unique<SymbolNode>(s[3]->Name());
//         auto symbols = vector<unique_ptr<SymbolNode>>();
//         for (char c = start->Name()[0]; c <= end->Name()[0]; ++c) {
//             symbols.push_back(std::make_unique<SymbolNode>(std::string(1, c)));
//         }
//         auto new_vector = vector_concat(std::move(s[0]), std::move(symbols));
//         return new_vector;
//     }}};
    // return g;
// }