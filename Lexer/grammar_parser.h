#pragma once
#include <string>
#include "../Grammar/grammar.h"
#include <fstream>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <cctype>

// Función para dividir una cadena en partes usando un delimitador
auto split(const std::string& s, char delimiter) -> std::vector<std::string> {
    std::vector<std::string> tokens;
    std::string token;
    for (char c : s) {
        if (c == delimiter) {
            if (!token.empty()) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += c;
        }
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
};
// std::string trim(const std::string& str) {
//     auto start = str.begin();
//     while (start != str.end() && std::isspace(*start)) {
//         start++;
//     }
//     auto end = str.end();
//     do {
//         end--;
//     } while (std::distance(start, end) > 0 && std::isspace(*end));

//     return std::string(start, end + 1);
// };

std::string trim(const std::string& str, const std::string& chars_to_remove = " \t\n\r") {
    auto start = str.find_first_not_of(chars_to_remove);
    if (start == std::string::npos) return "";
    
    auto end = str.find_last_not_of(chars_to_remove);
    return str.substr(start, end - start + 1);
};

class GrammarParser {
public:
    static Grammar Parse(const std::string& filename) {
        Grammar g = Grammar();
        std::ifstream file(filename);
        std::string line;
        
        std::shared_ptr<NonTerminal> startSymbol = nullptr;
        std::unordered_map<std::string, std::shared_ptr<Symbol>> symbols; 

        while (std::getline(file, line)) {
            // Limpiar y saltar líneas vacías/comentarios
            line = trim(line);
            if (line.empty() || line[0] == '#') continue;

            if (line.find("Start:") == 0) {
                std::string startName = trim(line.substr(6)); // Asumimos que "Start:" es seguido por un espacio y el nombre del símbolo
                startSymbol = g.SetNonTerminal(startName, true);
                symbols[startName] = startSymbol;
            } else if (line.find("NonTerminals:") == 0) {
                while (std::getline(file, line) && !line.empty()) {
                    auto terms = split(trim(line), ',');
                    for (const auto& term : terms) {
                        std::string name = trim(term);
                        if (!name.empty()) {
                            auto nt = g.SetNonTerminal(name);
                            symbols[name] = nt;
                        }
                    }
                }
                
            } else if (line.find("Terminals:") == 0) {
                while (std::getline(file, line) && !line.empty()) {
                    auto terms = split(trim(line), ',');
                    for (const auto& term : terms) {
                        std::string name = trim(term, "\" ");
                        auto t = g.SetTerminal(name);
                        symbols[name] = t;
                    }
                }
            } else if (line.find("Productions:") == 0) {
                while (std::getline(file, line) && !line.empty()) {
                    parseProduction(line, g, symbols);
                } 
            }
        }
        return g;
    };

private:
    static void parseProduction(const std::string& line, Grammar& g, std::unordered_map<std::string, std::shared_ptr<Symbol>>& symbols) {
        // Dividir E -> T { ... }
        auto parts = split(line, '{');
        std::string production = trim(parts[0]);
        std::string action = parts.size() > 1 ? trim(parts[1]) : "";

        // DIvidir left -> right
        auto arrowPos = production.find("->");
        std::string leftStr = trim(production.substr(0, arrowPos));
        std::string rightStr = trim(production.substr(arrowPos + 2));

        auto left = std::dynamic_pointer_cast<NonTerminal>(symbols[leftStr]);
        if (!left) {
            left = g.SetNonTerminal(leftStr);
            symbols[leftStr] = left;
        }

        // Construir Sentence
        std::vector<shared_ptr<Symbol>> rightSymbols;
        auto tokens = split(rightStr, ' ');
        for (const auto& token : tokens) {
            std::string trim_token = trim(token, "\" ");
            if (token.empty()) continue;
            if (token == "epsilon") {
                auto epsilon = g.GetEpsilon();
                rightSymbols.push_back(epsilon);
                symbols["epsilon"] = epsilon;
                continue;
            }
            if (symbols.find(trim_token) != symbols.end()) {
                rightSymbols.push_back(symbols[trim_token]);
            } else {
                //Si no esta en symbols, creo un nuevo simbolo y lo agrego
                if (token[0] == '"') {
                    auto newSym = g.SetTerminal(trim_token);
                    symbols[trim_token] = newSym;
                    rightSymbols.push_back(newSym);
                } else {
                    auto newSym = g.SetNonTerminal(trim_token);
                    symbols[trim_token] = newSym;
                    rightSymbols.push_back(newSym);
                }
            }
        }

        // AttrProd::SemanticAction semanticAction = [action](auto h, auto s) {
        //     return parseAction(action, s);
        // };

        // Add the production to the grammar or the non-terminal directly
        // AttrProd prod = AttrProd(left, Sentence(rightSymbols), {semanticAction});
        Production prod = Production(left, Sentence(rightSymbols));
        g.AddProduction(prod);
    };

    // static std::shared_ptr<Node> parseAction(const std::string& action, 
    //                                        const std::vector<std::shared_ptr<Node>>& s) {
    //     // Implementar parser de acciones (simplificado)
    //     if (action.find("std::make_shared") != std::string::npos) {
    //         // Extraer tipo y argumentos
    //         // Ej: $$ = std::make_shared<UnionNode>($1, $3);
    //         auto typeStart = action.find('<') + 1;
    //         auto typeEnd = action.find('>', typeStart);
    //         std::string type = action.substr(typeStart, typeEnd - typeStart);
            
    //         auto argStart = action.find('(') + 1;
    //         auto argEnd = action.find(')', argStart);
    //         std::string args = action.substr(argStart, argEnd - argStart);
            
    //         auto argList = split(args, ',');
    //         std::vector<std::shared_ptr<Node>> nodeArgs;
    //         for (const auto& arg : argList) {
    //             if (arg[0] == '$') {
    //                 int index = std::stoi(arg.substr(1)) - 1;
    //                 nodeArgs.push_back(s[index]);
    //             }
    //         }
            
    //         // Crear nodo (simplificado)
    //         if (type == "UnionNode") {
    //             return std::make_shared<UnionNode>(nodeArgs[0], nodeArgs[1]);
    //         }
    //         // ... otros tipos
    //     }
    //     return nullptr;
    // }
};

