#pragma once
#include <memory>
#include <queue>
#include <stack>
#include <map>
#include <vector>
#include <string>
#include "../Grammar/grammar.hpp"
#include "SLR1Parser.hpp"
#include "../Lexer/node.hpp"

std::shared_ptr<Node> reverse_evaluate(std::queue<std::shared_ptr<AttrProd>>& productions, 
                        const std::vector<std::string>& actions, 
                        const std::vector<std::pair<std::string, std::string>>& token_names, Grammar& grammar) {
    std::stack<std::string> token_stack;
    std::map<std::string, std::stack<std::shared_ptr<Node>>> node_stack;
    // Initialize the node stack for each non-terminal in the grammar
    // for (const auto& nt : grammar.NonTerminals()) {
    //     node_stack[nt->Name()].push(nullptr); // Push an empty node for each non-terminal
    // }

    int index = 0;
    for (const auto& action : actions) {
        if (action == SLR1Parser::SHIFT) {
            // Shift operation
            token_stack.push(token_names[index].first);
            index++;
        } else if (action == SLR1Parser::REDUCE) {
            // Reduce operation
            auto production = productions.front();
            productions.pop();
            auto attr = production->Attribute();
            std::vector<ElementType> args;
            auto prod_right = production->Right().Symbols();

            // Debug information
            // std::cout << "Processing production: " << production->ToString() << std::endl;
            // std::cout << "Production right side has " << prod_right.size() << " symbols" << std::endl;
            
            // Process symbols in reverse order since in LR parsing, we pop from stacks
            // We need to build the arguments vector in reverse order, then reverse it
            std::vector<ElementType> temp_args;
            for (int i = prod_right.size() - 1; i >= 0; --i) {
                // std::cout << "Processing symbol " << i << std::endl;
                
                // Check for null pointer
                if (!prod_right[i]) {
                    // std::cerr << "ERROR: Symbol at position " << i << " is null!" << std::endl;
                    throw std::runtime_error("Null symbol in production");
                }

                // std::cout << "Symbol name: " << prod_right[i]->Name() << std::endl;

                if (prod_right[i]->IsTerminal()) {
                    // If it's a terminal, we pop from the token stack
                    if (!token_stack.empty()) {
                        auto token_value = token_stack.top();
                        token_stack.pop();
                        temp_args.push_back(token_value);
                    } else {
                        throw std::runtime_error("Token stack is empty during reduce operation.");
                    }
                } else if (prod_right[i]->IsNonTerminal()) {
                    // If it's a non-terminal, we pop from the node stack
                    std::string nt_name = prod_right[i]->Name();
                    if (!node_stack[nt_name].empty()) {
                        auto node = node_stack[nt_name].top();
                        node_stack[nt_name].pop();
                        temp_args.push_back(node);
                    } else {
                        throw std::runtime_error("Node stack is empty for non-terminal during reduce operation.");
                    }
                }
            }
            // Reverse the arguments to get them in the correct order for semantic actions
            for (int i = temp_args.size() - 1; i >= 0; --i) {
                args.push_back(temp_args[i]);
            }
            // Call the attribute function with the collected arguments
            auto result = attr(args);
            // std::cout << "Attribute function returned: ";
            // if (std::holds_alternative<std::string>(result)) {
            //     std::cout << std::get<std::string>(result) << std::endl;
            // } else if (std::holds_alternative<std::shared_ptr<Node>>(result)) {
            //     std::cout << "Node" << std::endl;
            //     auto object = std::get<std::shared_ptr<Node>>(result);
            //     if (object) {
            //         auto nfa = object->evaluate();
            //         std::cout << "Node object is valid." << std::endl;
            //     } else {
            //         std::cerr << "ERROR: Node object is null!" << std::endl;
            //         throw std::runtime_error("Node object is null after attribute function call.");
            //     }
            // } else {
            //     std::cout << "Unknown type" << std::endl;
            // }
            // Check if the result is a Node
            if (std::holds_alternative<std::shared_ptr<Node>>(result)) {
                // If the result is a Node, we push it onto the node stack for the left-hand side non-terminal
                auto left_nt = production->Left();
                if (left_nt) {
                    node_stack[left_nt->Name()].push(std::get<std::shared_ptr<Node>>(result));
                } else {
                    throw std::runtime_error("Left-hand side of production is not a NonTerminal.");
                }
            } else {
                throw std::runtime_error("Attribute function did not return a Node.");
            }
        } else if (action == SLR1Parser::OK) {
            // Accept operation
            // std::cout << "Accept operation reached" << std::endl;
            if (!productions.empty()) {
                auto production = productions.front();
                productions.pop();
                auto attr = production->Attribute();
                std::vector<ElementType> args;
                auto prod_right = production->Right().Symbols();

                // Debug information
                // std::cout << "Processing production: " << production->ToString() << std::endl;
                // std::cout << "Production right side has " << prod_right.size() << " symbols" << std::endl;
                
                // Process symbols in reverse order since in LR parsing, we pop from stacks
                // We need to build the arguments vector in reverse order, then reverse it
                std::vector<ElementType> temp_args;
                for (int i = prod_right.size() - 1; i >= 0; --i) {
                    // std::cout << "Processing symbol " << i << std::endl;

                    // Check for null pointer
                    if (!prod_right[i]) {
                        // std::cerr << "ERROR: Symbol at position " << i << " is null!" << std::endl;
                        throw std::runtime_error("Null symbol in production");
                    }
                    
                    // std::cout << "Symbol name: " << prod_right[i]->Name() << std::endl;
                    
                    if (prod_right[i]->IsTerminal()) {
                        // If it's a terminal, we pop from the token stack
                        if (!token_stack.empty()) {
                            auto token_value = token_stack.top();
                            token_stack.pop();
                            temp_args.push_back(token_value);
                        } else {
                            throw std::runtime_error("Token stack is empty during reduce operation.");
                        }
                    } else if (prod_right[i]->IsNonTerminal()) {
                        // If it's a non-terminal, we pop from the node stack
                        std::string nt_name = prod_right[i]->Name();
                        if (!node_stack[nt_name].empty()) {
                            auto node = node_stack[nt_name].top();
                            node_stack[nt_name].pop();
                            temp_args.push_back(node);
                        } else {
                            throw std::runtime_error("Node stack is empty for non-terminal during reduce operation.");
                        }
                    }
                }
                // Reverse the arguments to get them in the correct order for semantic actions
                for (int i = temp_args.size() - 1; i >= 0; --i) {
                    args.push_back(temp_args[i]);
                }
                // Call the attribute function with the collected arguments
                auto result = attr(args);
                // std::cout << "Attribute function returned: ";
                // if (std::holds_alternative<std::string>(result)) {
                //     std::cout << std::get<std::string>(result) << std::endl;
                // } else if (std::holds_alternative<std::shared_ptr<Node>>(result)) {
                //     std::cout << "Node" << std::endl;
                //     auto object = std::get<std::shared_ptr<Node>>(result);
                //     if (object) {
                //         auto nfa = object->evaluate();
                //         std::cout << "Node object is valid." << std::endl;
                //     } else {
                //         std::cerr << "ERROR: Node object is null!" << std::endl;
                //         throw std::runtime_error("Node object is null after attribute function call.");
                //     }
                // } else {
                //     std::cout << "Unknown type" << std::endl;
                // }
                // Check if the result is a Node
                if (std::holds_alternative<std::shared_ptr<Node>>(result)) {
                    // If the result is a Node, we push it onto the node stack for the left-hand side non-terminal
                    auto left_nt = production->Left();
                    if (left_nt) {
                        node_stack[left_nt->Name()].push(std::get<std::shared_ptr<Node>>(result));
                    } else {
                        throw std::runtime_error("Left-hand side of production is not a NonTerminal.");
                    }
                } else {
                    throw std::runtime_error("Attribute function did not return a Node.");
                }
            }
            break;
        }
    }
    
    // std::cout << "Parsing completed. Checking node stacks:" << std::endl;
    for (const auto& [name, stack] : node_stack) {
        // std::cout << "Stack for " << name << " has " << stack.size() << " elements" << std::endl;
    }
    
    // At the end, we should have a single node for the start symbol
    // auto start_symbol = grammar.GetStartSymbol();
    // if (node_stack.find(*start_symbol) == node_stack.end() || node_stack[*start_symbol].empty()) {
    //     throw std::runtime_error("Node stack for start symbol is empty after parsing.");
    // }
    std::string start_name = grammar.GetStartSymbol()->Name();
    if (node_stack.find(start_name) == node_stack.end() || node_stack[start_name].empty()) {
        throw std::runtime_error("Node stack for start symbol is empty after parsing.");
    }
    // std::cout << "Final node stack for start symbol '" << start_name << "' has " 
    //       << node_stack[start_name].size() << " elements." << std::endl;
    return node_stack[start_name].top();
}