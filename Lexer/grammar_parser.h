#pragma once
#include <string>
#include "../Grammar/grammar.h"
#include <fstream>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include "node.h"
#include "../Ast/ast.hpp"
#include <functional>
#include <vector>
#include <variant>
#include "SpecialTypes.h"
#include "FunRegistry.h"



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
        InnerFun semanticAction;
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
                        semanticAction = getFactoryFunction(funcName)(indexes);
                    } else if (var2[0] == '$') {
                        // Si es una variable, obtenemos el índice
                        int index = std::stoi(var2.substr(1)) - 1;
                        std::vector<int> reduceIndex = {index};
                        semanticAction = getFactoryFunction("Reduce")(reduceIndex);
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
            semanticAction = [](const AnyVec& args) -> std::any {
                if (args.empty()) {
                    return std::make_shared<EpsilonNode>();
                }
                return args[0];  // Default: just return the first argument
            };
        }











        // Create a wrapper that converts the InnerFun function to AttrProd::SemanticAction
        AttrProd::SemanticAction wrappedAction = [semanticAction](const std::vector<ElementType>& args) -> ElementType {
            // Convert std::vector<ElementType> to AnyVec (std::vector<std::any>)
            AnyVec anyArgs;
            for (const auto& arg : args) {
                // Handle different variant types
                if (std::holds_alternative<std::string>(arg)) {
                    anyArgs.push_back(std::get<std::string>(arg));
                } else if (std::holds_alternative<std::shared_ptr<Node>>(arg)) {
                    anyArgs.push_back(std::get<std::shared_ptr<Node>>(arg));
                }
            }
            
            // Call the original function
            std::any result = semanticAction(anyArgs);
            
            // Convert the result back to ElementType
            try {
                // Try to cast to std::string first
                return std::any_cast<std::string>(result);
            } catch (const std::bad_any_cast&) {
                try {
                    // Then try to cast to std::shared_ptr<Node>
                    return std::any_cast<std::shared_ptr<Node>>(result);
                } catch (const std::bad_any_cast&) {
                    // Default case, just return an epsilon node
                    return std::make_shared<EpsilonNode>();
                }
            }
        };

        // Add the production to the grammar or the non-terminal directly
        AttrProd prod = AttrProd(left, Sentence(rightSymbols), wrappedAction);
        g.AddProduction(prod);
    };


private:

    static const std::map<std::string, OuterFunction> attrTable;

};

// Ast Nodes


// inline const std::map<std::string, GrammarParser::OuterFunction> GrammarParser::attrTable = {
//     {"SymbolNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 1) {
//             throw std::invalid_argument("SymbolNode requires exactly one index");
//         }
//         int index = indexes[0];
//         return [index](const std::vector<ElementType>& args) -> ElementType {
//             if (index >= args.size() || !std::holds_alternative<std::string>(args[index])) {
//                 throw std::invalid_argument("SymbolNode requires a single string argument");
//             }
//             auto node = new SymbolNode(std::get<std::string>(args[index]));
//             return std::make_shared<SymbolNode>(*node);
//         };
//     }},
//     {"UnionNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 2) {
//             throw std::invalid_argument("UnionNode requires exactly two indexes");
//         }
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             if (args.size() <= std::max(indexes[0], indexes[1]) ||
//                 !std::holds_alternative<std::shared_ptr<Node>>(args[indexes[0]]) ||
//                 !std::holds_alternative<std::shared_ptr<Node>>(args[indexes[1]])) {
//                 throw std::invalid_argument("UnionNode requires two Node arguments");
//             }
//             return std::make_shared<UnionNode>(
//                 std::get<std::shared_ptr<Node>>(args[indexes[0]]),
//                 std::get<std::shared_ptr<Node>>(args[indexes[1]])
//             );
//         };
//     }},
//     {"ConcatNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 2) {
//             throw std::invalid_argument("ConcatNode requires exactly two indexes");
//         }
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             if (args.size() <= std::max(indexes[0], indexes[1]) ||
//                 !std::holds_alternative<std::shared_ptr<Node>>(args[indexes[0]]) ||
//                 !std::holds_alternative<std::shared_ptr<Node>>(args[indexes[1]])) {
//                 throw std::invalid_argument("ConcatNode requires two Node arguments");
//             }
//             return std::make_shared<ConcatNode>(
//                 std::get<std::shared_ptr<Node>>(args[indexes[0]]),
//                 std::get<std::shared_ptr<Node>>(args[indexes[1]])
//             );
//         };
//     }},
//     {"RangeNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 2) {
//             throw std::invalid_argument("RangeNode requires exactly two indexes");
//         }
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             if (args.size() <= std::max(indexes[0], indexes[1]) ||
//                 !std::holds_alternative<std::shared_ptr<Node>>(args[indexes[0]]) ||
//                 !std::holds_alternative<std::shared_ptr<Node>>(args[indexes[1]])) {
//                 throw std::invalid_argument("RangeNode requires two SymbolNode arguments");
//             }
//             return std::make_shared<RangeNode>(
//                 std::get<std::shared_ptr<Node>>(args[indexes[0]]),
//                 std::get<std::shared_ptr<Node>>(args[indexes[1]])
//             );
//         };
//     }},
//     {"ClosureNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 1) {
//             throw std::invalid_argument("ClosureNode requires exactly one index");
//         }
//         int index = indexes[0];
//         return [index](const std::vector<ElementType>& args) -> ElementType {
//             if (args.size() <= index || !std::holds_alternative<std::shared_ptr<Node>>(args[index])) {
//                 throw std::invalid_argument("ClosureNode requires a single Node argument");
//             }
//             return std::make_shared<ClosureNode>(
//                 std::get<std::shared_ptr<Node>>(args[index])
//             );
//         };
//     }},
//     {"PositiveClosure", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 1) {
//             throw std::invalid_argument("PositiveClosure requires exactly one index");
//         }
//         int index = indexes[0];
//         return [index](const std::vector<ElementType>& args) -> ElementType {
//             if (args.size() <= index || !std::holds_alternative<std::shared_ptr<Node>>(args[index])) {
//                 throw std::invalid_argument("PositiveClosure requires a single Node argument");
//             }
//             return std::make_shared<PositiveClosure>(
//                 std::get<std::shared_ptr<Node>>(args[index])
//             );
//         };
//     }},
//     {"ZeroOrOneNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 1) {
//             throw std::invalid_argument("ZeroOrOneNode requires exactly one index");
//         }
//         int index = indexes[0];
//         return [index](const std::vector<ElementType>& args) -> ElementType {
//             if (args.size() <= index || !std::holds_alternative<std::shared_ptr<Node>>(args[index])) {
//                 throw std::invalid_argument("ZeroOrOneNode requires a single Node argument");
//             }
//             return std::make_shared<ZeroOrOneNode>(
//                 std::get<std::shared_ptr<Node>>(args[index])
//             );
//         };
//     }},
//     {"EpsilonNode", [](const std::vector<int>& indexes) {
//         if (!indexes.empty()) {
//             throw std::invalid_argument("EpsilonNode requires no indexes");
//         }
//         return [](const std::vector<ElementType>& /*args*/) -> ElementType {
//             return std::make_shared<EpsilonNode>();
//         };
//     }},
//     // Nodos del AST
//     {"ProgramNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 1) {
//             throw std::invalid_argument("ProgramNode requires exactly one index");
//         }
//         int index = indexes[0];
//         return [index](const std::vector<ElementType>& args) -> ElementType {
//             if (index >= args.size() || !std::holds_alternative<std::shared_ptr<ASTNode>>(args[index])) {
//                 throw std::invalid_argument("ProgramNode requires a single ASTNode argument");
//             }
//             // Using .get() to convert shared_ptr to raw pointer for the constructor
//             return std::make_shared<ProgramNode>(
//                 std::get<std::shared_ptr<ASTNode>>(args[index]).get()
//             );
//         };
//     }},
//     {"FloatNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 1) {
//             throw std::invalid_argument("FloatNode requires exactly one index");
//         }
//         int index = indexes[0];
//         return [index](const std::vector<ElementType>& args) -> ElementType {
//             if (index >= args.size()) {
//                 throw std::invalid_argument("FloatNode index out of range");
//             }
            
//             // Si el valor es un float, usarlo directamente
//             if (std::holds_alternative<float>(args[index])) {
//                 return std::make_shared<FloatNode>(std::get<float>(args[index]));
//             } 
//             // Si es string, intentar convertirlo a float
//             else if (std::holds_alternative<std::string>(args[index])) {
//                 try {
//                     float value = std::stof(std::get<std::string>(args[index]));
//                     return std::make_shared<FloatNode>(value);
//                 } catch (const std::exception& e) {
//                     throw std::invalid_argument("Failed to convert string to float: " + std::get<std::string>(args[index]));
//                 }
//             }
//             throw std::invalid_argument("FloatNode requires a float or a string that can be converted to float");
//         };
//     }},
//     {"BoolNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 1) {
//             throw std::invalid_argument("BoolNode requires exactly one index");
//         }
//         int index = indexes[0];
//         return [index](const std::vector<ElementType>& args) -> ElementType {
//             if (index >= args.size()) {
//                 throw std::invalid_argument("BoolNode index out of range");
//             }
            
//             // Si el valor es un bool, usarlo directamente
//             if (std::holds_alternative<bool>(args[index])) {
//                 return std::make_shared<BoolNode>(std::get<bool>(args[index]));
//             } 
//             // Si es string, intentar convertirlo a bool
//             else if (std::holds_alternative<std::string>(args[index])) {
//                 std::string value = std::get<std::string>(args[index]);
//                 // Considerar "true", "True", "TRUE" como true
//                 bool boolValue = (value == "true" || value == "True" || value == "TRUE");
//                 return std::make_shared<BoolNode>(boolValue);
//             }
//             throw std::invalid_argument("BoolNode requires a bool or a string that can be converted to bool");
//         };
//     }},
//     {"BoolExprNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 1) {
//             throw std::invalid_argument("BoolExprNode requires exactly one index");
//         }
//         int index = indexes[0];
//         return [index](const std::vector<ElementType>& args) -> ElementType {
//             if (index >= args.size() || !std::holds_alternative<std::shared_ptr<ASTNode>>(args[index])) {
//                 throw std::invalid_argument("BoolExprNode requires a single ASTNode argument");
//             }
//             return std::make_shared<BoolExprNode>(
//                 std::get<std::shared_ptr<ASTNode>>(args[index]).get()
//             );
//         };
//     }},
//     {"StringNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 1) {
//             throw std::invalid_argument("StringNode requires exactly one index");
//         }
//         int index = indexes[0];
//         return [index](const std::vector<ElementType>& args) -> ElementType {
//             if (index >= args.size()) {
//                 throw std::invalid_argument("StringNode index out of range");
//             }
            
//             // If the value is already a string, use it directly
//             if (std::holds_alternative<std::string>(args[index])) {
//                 // Process the string (remove quotes if needed)
//                 std::string value = std::get<std::string>(args[index]);
//                 // Remove quotes if present at both ends
//                 if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
//                     value = value.substr(1, value.size() - 2);
//                 }
//                 return std::make_shared<StringNode>(value);
//             } 
//             // If it's a different type that can be converted to string
//             else {
//                 throw std::invalid_argument("StringNode requires a string argument");
//             }
//         };
//     }},
//     {"UnaryOpNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 2) {
//             throw std::invalid_argument("UnaryOpNode requires exactly two indexes");
//         }
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             if (args.size() <= std::max(indexes[0], indexes[1]) ||
//                 !std::holds_alternative<std::string>(args[indexes[0]]) ||
//                 !std::holds_alternative<std::shared_ptr<ASTNode>>(args[indexes[1]])) {
//                 throw std::invalid_argument("UnaryOpNode requires a string and an ASTNode argument");
//             }
//             return std::make_shared<UnaryOpNode>(
//                 std::get<std::string>(args[indexes[0]]),
//                 std::get<std::shared_ptr<ASTNode>>(args[indexes[1]]).get()
//             );
//         };
//     }},
//     {"BinOpNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 3) {
//             throw std::invalid_argument("BinOpNode requires exactly three indexes");
//         }
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             if (args.size() <= std::max({indexes[0], indexes[1], indexes[2]}) ||
//                 !std::holds_alternative<std::shared_ptr<ASTNode>>(args[indexes[0]]) ||
//                 !std::holds_alternative<std::string>(args[indexes[1]]) ||
//                 !std::holds_alternative<std::shared_ptr<ASTNode>>(args[indexes[2]])) {
//                 throw std::invalid_argument("BinOpNode requires an ASTNode, a string, and an ASTNode argument");
//             }
//             return std::make_shared<BinOpNode>(
//                 std::get<std::shared_ptr<ASTNode>>(args[indexes[0]]).get(),
//                 std::get<std::string>(args[indexes[1]]),
//                 std::get<std::shared_ptr<ASTNode>>(args[indexes[2]]).get()
//             );
//         };
//     }},
//     {"FunctionCallNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 2) {
//             throw std::invalid_argument("FunctionCallNode requires exactly two indexes");
//         }
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             if (args.size() <= std::max(indexes[0], indexes[1]) ||
//                 !std::holds_alternative<std::string>(args[indexes[0]]) ||
//                 !std::holds_alternative<std::shared_ptr<ASTNode>>(args[indexes[1]])) {
//                 throw std::invalid_argument("FunctionCallNode requires a string and an ASTNode argument");
//             }
//             return std::make_shared<FunctionCallNode>(
//                 std::get<std::string>(args[indexes[0]]),
//                 std::get<std::shared_ptr<ASTNode>>(args[indexes[1]]).get()
//             );
//         };
//     }},
//     {"IDNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() == 1) {
//             int index = indexes[0];
//             return [index](const std::vector<ElementType>& args) -> ElementType {
//                 if (index >= args.size() || !std::holds_alternative<std::string>(args[index])) {
//                     throw std::invalid_argument("IDNode requires a string argument");
//                 }
//                 return std::make_shared<IDNode>(std::get<std::string>(args[index]));
//             };
//         } else if (indexes.size() == 2) {
//             return [indexes](const std::vector<ElementType>& args) -> ElementType {
//                 if (args.size() <= std::max(indexes[0], indexes[1]) ||
//                     !std::holds_alternative<std::string>(args[indexes[0]]) ||
//                     !std::holds_alternative<std::string>(args[indexes[1]])) {
//                     throw std::invalid_argument("IDNode requires two string arguments");
//                 }
//                 return std::make_shared<IDNode>(
//                     std::get<std::string>(args[indexes[0]]),
//                     std::get<std::string>(args[indexes[1]])
//                 );
//             };
//         } else {
//             throw std::invalid_argument("IDNode requires one or two indexes");
//         }
//     }},
//     {"BlockNode", [](const std::vector<int>& indexes) {
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             std::vector<ASTNode*> nodes;
//             for (int index : indexes) {
//                 if (index >= args.size() || !std::holds_alternative<std::shared_ptr<ASTNode>>(args[index])) {
//                     throw std::invalid_argument("BlockNode requires ASTNode arguments");
//                 }
//                 nodes.push_back(std::get<std::shared_ptr<ASTNode>>(args[index]).get());
//             }
//             return std::make_shared<BlockNode>(nodes);
//         };
//     }},
//     {"ArgsList", [](const std::vector<int>& indexes) {
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             std::vector<IDNode*> nodes;
//             for (int index : indexes) {
//                 if (index >= args.size() || !std::holds_alternative<std::shared_ptr<IDNode>>(args[index])) {
//                     throw std::invalid_argument("ArgsList requires IDNode arguments");
//                 }
//                 nodes.push_back(std::get<std::shared_ptr<IDNode>>(args[index]).get());
//             }
//             return std::make_shared<ArgsList>(nodes);
//         };
//     }},
//     {"ExprsList", [](const std::vector<int>& indexes) {
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             std::vector<ASTNode*> nodes;
//             for (int index : indexes) {
//                 if (index >= args.size() || !std::holds_alternative<std::shared_ptr<ASTNode>>(args[index])) {
//                     throw std::invalid_argument("ExprsList requires ASTNode arguments");
//                 }
//                 nodes.push_back(std::get<std::shared_ptr<ASTNode>>(args[index]).get());
//             }
//             return std::make_shared<ExprsList>(nodes);
//         };
//     }},
//     {"AssignFuncNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() == 3) {
//             return [indexes](const std::vector<ElementType>& args) -> ElementType {
//                 if (args.size() <= std::max({indexes[0], indexes[1], indexes[2]}) ||
//                     !std::holds_alternative<std::shared_ptr<IDNode>>(args[indexes[0]]) ||
//                     !std::holds_alternative<std::shared_ptr<ArgsList>>(args[indexes[1]]) ||
//                     !std::holds_alternative<std::shared_ptr<ASTNode>>(args[indexes[2]])) {
//                     throw std::invalid_argument("AssignFuncNode requires IDNode, ArgsList, and ASTNode arguments");
//                 }
//                 return std::make_shared<AssignFuncNode>(
//                     std::get<std::shared_ptr<IDNode>>(args[indexes[0]]).get(),
//                     std::get<std::shared_ptr<ArgsList>>(args[indexes[1]]).get(),
//                     std::get<std::shared_ptr<ASTNode>>(args[indexes[2]]).get()
//                 );
//             };
//         } else if (indexes.size() == 4) {
//             return [indexes](const std::vector<ElementType>& args) -> ElementType {
//                 if (args.size() <= std::max({indexes[0], indexes[1], indexes[2], indexes[3]}) ||
//                     !std::holds_alternative<std::shared_ptr<IDNode>>(args[indexes[0]]) ||
//                     !std::holds_alternative<std::shared_ptr<ArgsList>>(args[indexes[1]]) ||
//                     !std::holds_alternative<std::shared_ptr<ASTNode>>(args[indexes[2]]) ||
//                     !std::holds_alternative<std::string>(args[indexes[3]])) {
//                     throw std::invalid_argument("AssignFuncNode requires IDNode, ArgsList, ASTNode, and string arguments");
//                 }
//                 return std::make_shared<AssignFuncNode>(
//                     std::get<std::shared_ptr<IDNode>>(args[indexes[0]]).get(),
//                     std::get<std::shared_ptr<ArgsList>>(args[indexes[1]]).get(),
//                     std::get<std::shared_ptr<ASTNode>>(args[indexes[2]]).get(),
//                     std::get<std::string>(args[indexes[3]])
//                 );
//             };
//         } else {
//             throw std::invalid_argument("AssignFuncNode requires three or four indexes");
//         }
//     }},
//     {"VarAssign", [](const std::vector<int>& indexes) {
//         if (indexes.size() == 2) {
//             return [indexes](const std::vector<ElementType>& args) -> ElementType {
//                 if (args.size() <= std::max(indexes[0], indexes[1]) ||
//                     !std::holds_alternative<std::shared_ptr<IDNode>>(args[indexes[0]]) ||
//                     !std::holds_alternative<std::shared_ptr<ASTNode>>(args[indexes[1]])) {
//                     throw std::invalid_argument("VarAssign requires IDNode and ASTNode arguments");
//                 }
//                 return std::make_shared<VarAssign>(
//                     std::get<std::shared_ptr<IDNode>>(args[indexes[0]]).get(),
//                     std::get<std::shared_ptr<ASTNode>>(args[indexes[1]]).get()
//                 );
//             };
//         } else if (indexes.size() == 3) {
//             return [indexes](const std::vector<ElementType>& args) -> ElementType {
//                 if (args.size() <= std::max({indexes[0], indexes[1], indexes[2]}) ||
//                     !std::holds_alternative<std::shared_ptr<IDNode>>(args[indexes[0]]) ||
//                     !std::holds_alternative<std::shared_ptr<ASTNode>>(args[indexes[1]]) ||
//                     !std::holds_alternative<std::string>(args[indexes[2]])) {
//                     throw std::invalid_argument("VarAssign requires IDNode, ASTNode, and string arguments");
//                 }
//                 return std::make_shared<VarAssign>(
//                     std::get<std::shared_ptr<IDNode>>(args[indexes[0]]).get(),
//                     std::get<std::shared_ptr<ASTNode>>(args[indexes[1]]).get(),
//                     std::get<std::string>(args[indexes[2]])
//                 );
//             };
//         } else {
//             throw std::invalid_argument("VarAssign requires two or three indexes");
//         }
//     }},
//     {"NewTypeNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 2) {
//             throw std::invalid_argument("NewTypeNode requires exactly two indexes");
//         }
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             if (args.size() <= std::max(indexes[0], indexes[1]) ||
//                 !std::holds_alternative<std::string>(args[indexes[0]]) ||
//                 !std::holds_alternative<std::vector<std::shared_ptr<ASTNode>>>(args[indexes[1]])) {
//                 throw std::invalid_argument("NewTypeNode requires a string and a vector of ASTNode arguments");
//             }
//             // Convert vector of shared_ptr<ASTNode> to vector of ASTNode*
//             std::vector<ASTNode*> rawNodes;
//             auto sharedNodes = std::get<std::vector<std::shared_ptr<ASTNode>>>(args[indexes[1]]);
//             for (const auto& node : sharedNodes) {
//                 rawNodes.push_back(node.get());
//             }
//             return std::make_shared<NewTypeNode>(
//                 std::get<std::string>(args[indexes[0]]),
//                 rawNodes
//             );
//         };
//     }},
//     {"VarAssignType", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 3) {
//             throw std::invalid_argument("VarAssignType requires exactly three indexes");
//         }
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             if (args.size() <= std::max({indexes[0], indexes[1], indexes[2]}) ||
//                 !std::holds_alternative<std::string>(args[indexes[0]]) ||
//                 !std::holds_alternative<std::shared_ptr<NewTypeNode>>(args[indexes[1]]) ||
//                 !std::holds_alternative<std::shared_ptr<ASTNode>>(args[indexes[2]])) {
//                 throw std::invalid_argument("VarAssignType requires string, NewTypeNode, and ASTNode arguments");
//             }
//             return std::make_shared<VarAssignType>(
//                 std::get<std::string>(args[indexes[0]]),
//                 std::get<std::shared_ptr<NewTypeNode>>(args[indexes[1]]).get(),
//                 std::get<std::shared_ptr<ASTNode>>(args[indexes[2]]).get()
//             );
//         };
//     }},
//     {"VarAssignList", [](const std::vector<int>& indexes) {
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             std::vector<VarAssign*> assigns;
//             for (int index : indexes) {
//                 if (index >= args.size() || !std::holds_alternative<std::shared_ptr<VarAssign>>(args[index])) {
//                     throw std::invalid_argument("VarAssignList requires VarAssign arguments");
//                 }
//                 assigns.push_back(std::get<std::shared_ptr<VarAssign>>(args[index]).get());
//             }
//             return std::make_shared<VarAssignList>(assigns);
//         };
//     }},
//     {"LetAssign", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 2) {
//             throw std::invalid_argument("LetAssign requires exactly two indexes");
//         }
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             if (args.size() <= std::max(indexes[0], indexes[1]) ||
//                 !std::holds_alternative<std::vector<std::shared_ptr<VarAssign>>>(args[indexes[0]]) ||
//                 !std::holds_alternative<std::shared_ptr<ASTNode>>(args[indexes[1]])) {
//                 throw std::invalid_argument("LetAssign requires a vector of VarAssign and an ASTNode argument");
//             }
//             // Convert vector of shared_ptr<VarAssign> to vector of VarAssign*
//             std::vector<VarAssign*> rawAssigns;
//             auto sharedAssigns = std::get<std::vector<std::shared_ptr<VarAssign>>>(args[indexes[0]]);
//             for (const auto& assign : sharedAssigns) {
//                 rawAssigns.push_back(assign.get());
//             }
//             return std::make_shared<LetAssign>(
//                 rawAssigns,
//                 std::get<std::shared_ptr<ASTNode>>(args[indexes[1]]).get()
//             );
//         };
//     }},
//     {"VarDesAssign", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 2) {
//             throw std::invalid_argument("VarDesAssign requires exactly two indexes");
//         }
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             if (args.size() <= std::max(indexes[0], indexes[1]) ||
//                 !std::holds_alternative<std::shared_ptr<IDNode>>(args[indexes[0]]) ||
//                 !std::holds_alternative<std::shared_ptr<ASTNode>>(args[indexes[1]])) {
//                 throw std::invalid_argument("VarDesAssign requires IDNode and ASTNode arguments");
//             }
//             return std::make_shared<VarDesAssign>(
//                 std::get<std::shared_ptr<IDNode>>(args[indexes[0]]).get(),
//                 std::get<std::shared_ptr<ASTNode>>(args[indexes[1]]).get()
//             );
//         };
//     }},
//     {"Conditional", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 3) {
//             throw std::invalid_argument("Conditional requires exactly three indexes");
//         }
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             if (args.size() <= std::max({indexes[0], indexes[1], indexes[2]}) ||
//                 !std::holds_alternative<std::shared_ptr<BoolExprNode>>(args[indexes[0]]) ||
//                 !std::holds_alternative<std::shared_ptr<ASTNode>>(args[indexes[1]]) ||
//                 !std::holds_alternative<std::shared_ptr<ASTNode>>(args[indexes[2]])) {
//                 throw std::invalid_argument("Conditional requires BoolExprNode and two ASTNode arguments");
//             }
//             return std::make_shared<Conditional>(
//                 std::get<std::shared_ptr<BoolExprNode>>(args[indexes[0]]).get(),
//                 std::get<std::shared_ptr<ASTNode>>(args[indexes[1]]).get(),
//                 std::get<std::shared_ptr<ASTNode>>(args[indexes[2]]).get()
//             );
//         };
//     }},
//     {"WhileNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 2) {
//             throw std::invalid_argument("WhileNode requires exactly two indexes");
//         }
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             if (args.size() <= std::max(indexes[0], indexes[1]) ||
//                 !std::holds_alternative<std::shared_ptr<BoolExprNode>>(args[indexes[0]]) ||
//                 !std::holds_alternative<std::shared_ptr<ASTNode>>(args[indexes[1]])) {
//                 throw std::invalid_argument("WhileNode requires BoolExprNode and ASTNode arguments");
//             }
//             return std::make_shared<WhileNode>(
//                 std::get<std::shared_ptr<BoolExprNode>>(args[indexes[0]]).get(),
//                 std::get<std::shared_ptr<ASTNode>>(args[indexes[1]]).get()
//             );
//         };
//     }},
//     {"ForNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 3) {
//             throw std::invalid_argument("ForNode requires exactly three indexes");
//         }
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             if (args.size() <= std::max({indexes[0], indexes[1], indexes[2]}) ||
//                 !std::holds_alternative<std::shared_ptr<IDNode>>(args[indexes[0]]) ||
//                 !std::holds_alternative<std::shared_ptr<ASTNode>>(args[indexes[1]]) ||
//                 !std::holds_alternative<std::shared_ptr<ASTNode>>(args[indexes[2]])) {
//                 throw std::invalid_argument("ForNode requires IDNode and two ASTNode arguments");
//             }
//             return std::make_shared<ForNode>(
//                 std::get<std::shared_ptr<IDNode>>(args[indexes[0]]).get(),
//                 std::get<std::shared_ptr<ASTNode>>(args[indexes[1]]).get(),
//                 std::get<std::shared_ptr<ASTNode>>(args[indexes[2]]).get()
//             );
//         };
//     }},
//     {"TypeDeclNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() == 3) {
//             return [indexes](const std::vector<ElementType>& args) -> ElementType {
//                 if (args.size() <= std::max({indexes[0], indexes[1], indexes[2]}) ||
//                     !std::holds_alternative<std::shared_ptr<IDNode>>(args[indexes[0]]) ||
//                     !std::holds_alternative<std::shared_ptr<ArgsList>>(args[indexes[1]]) ||
//                     !std::holds_alternative<std::vector<std::shared_ptr<ASTNode>>>(args[indexes[2]])) {
//                     throw std::invalid_argument("TypeDeclNode requires IDNode, ArgsList, and a vector of ASTNode arguments");
//                 }
                
//                 // Convert vector of shared_ptr<ASTNode> to vector of ASTNode*
//                 std::vector<ASTNode*> rawNodes;
//                 auto sharedNodes = std::get<std::vector<std::shared_ptr<ASTNode>>>(args[indexes[2]]);
//                 for (const auto& node : sharedNodes) {
//                     rawNodes.push_back(node.get());
//                 }
                
//                 return std::make_shared<TypeDeclNode>(
//                     std::get<std::shared_ptr<IDNode>>(args[indexes[0]]).get(),
//                     std::get<std::shared_ptr<ArgsList>>(args[indexes[1]]).get(),
//                     rawNodes
//                 );
//             };
//         } else if (indexes.size() == 4) {
//             return [indexes](const std::vector<ElementType>& args) -> ElementType {
//                 if (args.size() <= std::max({indexes[0], indexes[1], indexes[2], indexes[3]}) ||
//                     !std::holds_alternative<std::shared_ptr<IDNode>>(args[indexes[0]]) ||
//                     !std::holds_alternative<std::shared_ptr<ArgsList>>(args[indexes[1]]) ||
//                     !std::holds_alternative<std::vector<std::shared_ptr<ASTNode>>>(args[indexes[2]]) ||
//                     !std::holds_alternative<std::vector<std::string>>(args[indexes[3]])) {
//                     throw std::invalid_argument("TypeDeclNode requires IDNode, ArgsList, a vector of ASTNode, and a vector of string arguments");
//                 }
                
//                 // Convert vector of shared_ptr<ASTNode> to vector of ASTNode*
//                 std::vector<ASTNode*> rawNodes;
//                 auto sharedNodes = std::get<std::vector<std::shared_ptr<ASTNode>>>(args[indexes[2]]);
//                 for (const auto& node : sharedNodes) {
//                     rawNodes.push_back(node.get());
//                 }
                
//                 return std::make_shared<TypeDeclNode>(
//                     std::get<std::shared_ptr<IDNode>>(args[indexes[0]]).get(),
//                     std::get<std::shared_ptr<ArgsList>>(args[indexes[1]]).get(),
//                     rawNodes,
//                     std::get<std::vector<std::string>>(args[indexes[3]])
//                 );
//             };
//         } else if (indexes.size() == 5) {
//             return [indexes](const std::vector<ElementType>& args) -> ElementType {
//                 if (args.size() <= std::max({indexes[0], indexes[1], indexes[2], indexes[3], indexes[4]}) ||
//                     !std::holds_alternative<std::shared_ptr<IDNode>>(args[indexes[0]]) ||
//                     !std::holds_alternative<std::shared_ptr<ArgsList>>(args[indexes[1]]) ||
//                     !std::holds_alternative<std::vector<std::shared_ptr<ASTNode>>>(args[indexes[2]]) ||
//                     !std::holds_alternative<std::vector<std::string>>(args[indexes[3]]) ||
//                     !std::holds_alternative<std::shared_ptr<ArgsList>>(args[indexes[4]])) {
//                     throw std::invalid_argument("TypeDeclNode requires IDNode, ArgsList, a vector of ASTNode, a vector of string, and ArgsList arguments");
//                 }
                
//                 // Convert vector of shared_ptr<ASTNode> to vector of ASTNode*
//                 std::vector<ASTNode*> rawNodes;
//                 auto sharedNodes = std::get<std::vector<std::shared_ptr<ASTNode>>>(args[indexes[2]]);
//                 for (const auto& node : sharedNodes) {
//                     rawNodes.push_back(node.get());
//                 }
                
//                 return std::make_shared<TypeDeclNode>(
//                     std::get<std::shared_ptr<IDNode>>(args[indexes[0]]).get(),
//                     std::get<std::shared_ptr<ArgsList>>(args[indexes[1]]).get(),
//                     rawNodes,
//                     std::get<std::vector<std::string>>(args[indexes[3]]),
//                     std::get<std::shared_ptr<ArgsList>>(args[indexes[4]]).get()
//                 );
//             };
//         } else {
//             throw std::invalid_argument("TypeDeclNode requires three, four, or five indexes");
//         }
//     }},
//     {"AttributeMember", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 1) {
//             throw std::invalid_argument("AttributeMember requires exactly one index");
//         }
//         int index = indexes[0];
//         return [index](const std::vector<ElementType>& args) -> ElementType {
//             if (index >= args.size() || !std::holds_alternative<std::string>(args[index])) {
//                 throw std::invalid_argument("AttributeMember requires a string argument");
//             }
//             return std::make_shared<AttributeMember>(
//                 std::get<std::string>(args[index])
//             );
//         };
//     }},
//     {"MethodMember", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 2) {
//             throw std::invalid_argument("MethodMember requires exactly two indexes");
//         }
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             if (args.size() <= std::max(indexes[0], indexes[1]) ||
//                 !std::holds_alternative<std::string>(args[indexes[0]]) ||
//                 !std::holds_alternative<std::vector<std::shared_ptr<ASTNode>>>(args[indexes[1]])) {
//                 throw std::invalid_argument("MethodMember requires a string and a vector of ASTNode arguments");
//             }
            
//             // Convert vector of shared_ptr<ASTNode> to vector of ASTNode*
//             std::vector<ASTNode*> rawNodes;
//             auto sharedNodes = std::get<std::vector<std::shared_ptr<ASTNode>>>(args[indexes[1]]);
//             for (const auto& node : sharedNodes) {
//                 rawNodes.push_back(node.get());
//             }
            
//             return std::make_shared<MethodMember>(
//                 std::get<std::string>(args[indexes[0]]),
//                 rawNodes
//             );
//         };
//     }},
//     {"AccessNode", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 2) {
//             throw std::invalid_argument("AccessNode requires exactly two indexes");
//         }
//         return [indexes](const std::vector<ElementType>& args) -> ElementType {
//             if (args.size() <= std::max(indexes[0], indexes[1]) ||
//                 !std::holds_alternative<std::string>(args[indexes[0]]) ||
//                 !std::holds_alternative<std::shared_ptr<TypeAssMember>>(args[indexes[1]])) {
//                 throw std::invalid_argument("AccessNode requires a string and a TypeAssMember argument");
//             }
//             return std::make_shared<AccessNode>(
//                 std::get<std::string>(args[indexes[0]]),
//                 std::get<std::shared_ptr<TypeAssMember>>(args[indexes[1]]).get()
//             );
//         };
//     }},
//     //Caso $$ = $i
//     {"Reduce", [](const std::vector<int>& indexes) {
//         if (indexes.size() != 1) {
//             throw std::invalid_argument("Reduce requires exactly one index");
//         }
//         int index = indexes[0];
//         return [index](const std::vector<ElementType>& args) -> ElementType {
//             // if (!std::holds_alternative<std::shared_ptr<Node>>(args[index])) {
//             //     throw std::invalid_argument("args must contain a single SymbolNode argument");
//             // }
//             return std::get<std::shared_ptr<Node>>(args[index]);
//         };
//     }}
// };
