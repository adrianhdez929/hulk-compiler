#pragma once
#include <string>
#include "../Grammar/grammar.h"
#include <fstream>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include "node.h"
#include <functional>
#include <vector>
#include <variant>
#include "SpecialTypes.h"



/// @brief Toma una cadena y la divide en subcadenas (tokens) basándose en un delimitador específico.
/// @param s La cadena a dividir.
/// @param delimiter El carácter delimitador que se utilizará para dividir la cadena.
/// @return Un vector de subcadenas (tokens) resultantes de la división.
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

/// @brief Elimina los caracteres no deseados de los extremos de una cadena.
/// @param str La cadena a limpiar.
/// @param chars_to_remove Los caracteres a eliminar (por defecto: espacio, tabulación, nueva línea, retorno de carro).
/// @return La cadena limpia.
std::string trim(const std::string& str, const std::string& chars_to_remove = " \t\n\r") {
    auto start = str.find_first_not_of(chars_to_remove);
    if (start == std::string::npos) return "";
    
    auto end = str.find_last_not_of(chars_to_remove);
    return str.substr(start, end - start + 1);
};

class GrammarParser {
private:
    using InnerFunction = std::function<ElementType(const std::vector<ElementType>&)>;
    using OuterFunction = std::function<InnerFunction(const std::vector<int>&)>;
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
        InnerFunction semanticAction;
        // Si hay una acción, la procesamos
        if (!action.empty()) {
            //caso: hay "=" en la accion
            if (action.find('=') != std::string::npos) {
                // Extraer el nombre de la acción y los índices
                auto actionParts = split(action, '=');
                std::string var1 = trim(actionParts[0]);
                if (var1 == "$$") {
                    std::string var2 = trim(actionParts[1]);
                    // if Estructura de var2: funcName(arg1, arg2, ...)
                    // Obtener el funcName y los indices de los args
                    auto funcNameEnd = var2.find('(');
                    std::vector<int> indexes;
                    if (funcNameEnd != std::string::npos) {
                        std::string funcName = trim(var2.substr(0, funcNameEnd));
                        auto argsStr = var2.substr(funcNameEnd + 1);
                        argsStr = trim(argsStr, ") }");
                        auto argParts = split(argsStr, ',');
                        for (const auto& arg : argParts) {
                            auto new_arg = trim(arg);
                            //arg de la forma: $1, $2, etc.
                            if (new_arg[0] == '$') {
                                int index = std::stoi(new_arg.substr(1)) - 1;
                                indexes.push_back(index);
                            }
                        }
                        semanticAction = attrTable.at(funcName)(indexes);
                    } else if (var2[0] == '$') {
                        // Si es una variable, obtenemos el índice
                        int index = std::stoi(var2.substr(1)) - 1;
                        std::vector<int> reduceIndex = {index};
                        semanticAction = attrTable.at("Reduce")(reduceIndex);
                    } else {
                        throw std::invalid_argument("Invalid action format: " + action);
                    }
                }

                // Aquí se podría usar la tabla de atributos para obtener la función correspondiente
                // ElementType actionNode = attrTable[var](indexes);
            }

            // Aquí se podría usar la tabla de atributos para obtener la función correspondiente
            // ElementType actionNode = attrTable[var](indexes);
        } else {
            // If no action specified, create a default identity function
            semanticAction = [](const std::vector<ElementType>& args) -> ElementType {
                if (args.empty()) {
                    return std::make_shared<EpsilonNode>();
                }
                return args[0];  // Default: just return the first argument
            };
        }











        // AttrProd::SemanticAction semanticAction = [action](auto h, auto s) {
        //     return parseAction(action, s);
        // };

        // Add the production to the grammar or the non-terminal directly
        AttrProd prod = AttrProd(left, Sentence(rightSymbols), semanticAction);
        // Production prod = Production(left, Sentence(rightSymbols));
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


private:

    static const std::map<std::string, OuterFunction> attrTable;

};

inline const std::map<std::string, GrammarParser::OuterFunction> GrammarParser::attrTable = {
    {"SymbolNode", [](const std::vector<int>& indexes) {
        if (indexes.size() != 1) {
            throw std::invalid_argument("SymbolNode requires exactly one index");
        }
        int index = indexes[0];
        return [index](const std::vector<ElementType>& args) -> ElementType {
            if (index >= args.size() || !std::holds_alternative<std::string>(args[index])) {
                throw std::invalid_argument("SymbolNode requires a single string argument");
            }
            auto node = new SymbolNode(std::get<std::string>(args[index]));
            return std::make_shared<SymbolNode>(*node);
        };
    }},
    {"UnionNode", [](const std::vector<int>& indexes) {
        if (indexes.size() != 2) {
            throw std::invalid_argument("UnionNode requires exactly two indexes");
        }
        return [indexes](const std::vector<ElementType>& args) -> ElementType {
            if (args.size() <= std::max(indexes[0], indexes[1]) ||
                !std::holds_alternative<std::shared_ptr<Node>>(args[indexes[0]]) ||
                !std::holds_alternative<std::shared_ptr<Node>>(args[indexes[1]])) {
                throw std::invalid_argument("UnionNode requires two Node arguments");
            }
            return std::make_shared<UnionNode>(
                std::get<std::shared_ptr<Node>>(args[indexes[0]]),
                std::get<std::shared_ptr<Node>>(args[indexes[1]])
            );
        };
    }},
    {"ConcatNode", [](const std::vector<int>& indexes) {
        if (indexes.size() != 2) {
            throw std::invalid_argument("ConcatNode requires exactly two indexes");
        }
        return [indexes](const std::vector<ElementType>& args) -> ElementType {
            if (args.size() <= std::max(indexes[0], indexes[1]) ||
                !std::holds_alternative<std::shared_ptr<Node>>(args[indexes[0]]) ||
                !std::holds_alternative<std::shared_ptr<Node>>(args[indexes[1]])) {
                throw std::invalid_argument("ConcatNode requires two Node arguments");
            }
            return std::make_shared<ConcatNode>(
                std::get<std::shared_ptr<Node>>(args[indexes[0]]),
                std::get<std::shared_ptr<Node>>(args[indexes[1]])
            );
        };
    }},
    // {"StringClassNode", [](const std::vector<int>& indexes) {
    //     if (indexes.empty()) {
    //         throw std::invalid_argument("StringClassNode requires at least one index");
    //     }
    //     return [indexes](const std::vector<ElementType>& args) -> ElementType {
    //         std::vector<std::shared_ptr<SymbolNode>> symbols;
    //         for (int index : indexes) {
    //             if (index < 0 || index >= args.size() || !std::holds_alternative<std::shared_ptr<SymbolNode>>(args[index])) {
    //                 throw std::invalid_argument("StringClassNode requires valid SymbolNode arguments");
    //             }
    //             symbols.push_back(std::get<std::shared_ptr<SymbolNode>>(args[index]));
    //         }
    //         return std::make_shared<StringClassNode>(symbols);
    //     };
    // }},
    {"RangeNode", [](const std::vector<int>& indexes) {
        if (indexes.size() != 2) {
            throw std::invalid_argument("RangeNode requires exactly two indexes");
        }
        return [indexes](const std::vector<ElementType>& args) -> ElementType {
            if (args.size() <= std::max(indexes[0], indexes[1]) ||
                !std::holds_alternative<std::shared_ptr<Node>>(args[indexes[0]]) ||
                !std::holds_alternative<std::shared_ptr<Node>>(args[indexes[1]])) {
                throw std::invalid_argument("RangeNode requires two SymbolNode arguments");
            }
            return std::make_shared<RangeNode>(
                std::get<std::shared_ptr<Node>>(args[indexes[0]]),
                std::get<std::shared_ptr<Node>>(args[indexes[1]])
            );
        };
    }},
    {"ClosureNode", [](const std::vector<int>& indexes) {
        if (indexes.size() != 1) {
            throw std::invalid_argument("ClosureNode requires exactly one index");
        }
        int index = indexes[0];
        return [index](const std::vector<ElementType>& args) -> ElementType {
            if (args.size() <= index || !std::holds_alternative<std::shared_ptr<Node>>(args[index])) {
                throw std::invalid_argument("ClosureNode requires a single Node argument");
            }
            return std::make_shared<ClosureNode>(
                std::get<std::shared_ptr<Node>>(args[index])
            );
        };
    }},
    {"PositiveClosure", [](const std::vector<int>& indexes) {
        if (indexes.size() != 1) {
            throw std::invalid_argument("PositiveClosure requires exactly one index");
        }
        int index = indexes[0];
        return [index](const std::vector<ElementType>& args) -> ElementType {
            if (args.size() <= index || !std::holds_alternative<std::shared_ptr<Node>>(args[index])) {
                throw std::invalid_argument("PositiveClosure requires a single Node argument");
            }
            return std::make_shared<PositiveClosure>(
                std::get<std::shared_ptr<Node>>(args[index])
            );
        };
    }},
    {"ZeroOrOneNode", [](const std::vector<int>& indexes) {
        if (indexes.size() != 1) {
            throw std::invalid_argument("ZeroOrOneNode requires exactly one index");
        }
        int index = indexes[0];
        return [index](const std::vector<ElementType>& args) -> ElementType {
            if (args.size() <= index || !std::holds_alternative<std::shared_ptr<Node>>(args[index])) {
                throw std::invalid_argument("ZeroOrOneNode requires a single Node argument");
            }
            return std::make_shared<ZeroOrOneNode>(
                std::get<std::shared_ptr<Node>>(args[index])
            );
        };
    }},
    {"EpsilonNode", [](const std::vector<int>& indexes) {
        if (!indexes.empty()) {
            throw std::invalid_argument("EpsilonNode requires no indexes");
        }
        return [](const std::vector<ElementType>& /*args*/) -> ElementType {
            return std::make_shared<EpsilonNode>();
        };
    }},
    //Caso $$ = $i
    {"Reduce", [](const std::vector<int>& indexes) {
        if (indexes.size() != 1) {
            throw std::invalid_argument("Reduce requires exactly one index");
        }
        int index = indexes[0];
        return [index](const std::vector<ElementType>& args) -> ElementType {
            // if (!std::holds_alternative<std::shared_ptr<Node>>(args[index])) {
            //     throw std::invalid_argument("args must contain a single SymbolNode argument");
            // }
            return std::get<std::shared_ptr<Node>>(args[index]);
        };
    }}
};
