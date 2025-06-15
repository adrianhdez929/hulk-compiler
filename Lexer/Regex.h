#pragma once
#include <string>
#include <vector>
#include <regex>
#include <cctype>
#include <iostream>
#include "../Automata/dfa.h"
#include "../Automata/nfa.h"
#include "../Automata/utils/aut_manipulation.h"
#include "../Automata/operations/operations.h"
#include "../Parser/SLR1Parser.h"
#include "../Parser/reverse_evaluate.h"

using namespace std;


class Regex {
public:
    // Constructor
    Regex(const std::string& pattern, Grammar& grammar, SLR1Parser& parser) 
                    : pattern_(pattern), grammar_(grammar), automaton_(createEmptyDFA()) {
        automaton_ = build_dfa(parser);
    }

    std::vector<std::pair<std::string, std::string>> regex_tokenizer(const std::string& input) {
        std::vector<std::pair<std::string, std::string>> tokens;
        std::vector<std::string> fixed_tokens = {"(", ")", "|", "*", "+", "-", "?", "[", "]", "symbol"};
        bool string_class = false;
        
        for (int i = 0; i < input.length(); ++i) {
            char c = input[i];

            // Handle escape sequences
            if (c == '\\' && i + 1 < input.length()) {
                char next_char = input[i + 1];
                
                if (next_char == 'x' && i + 3 < input.length() && 
                    std::isxdigit(input[i + 2]) && std::isxdigit(input[i + 3])) {
                    // Handle hex escape sequences like \x00, \x7f
                    std::string hex_string = input.substr(i + 2, 2);
                    try {
                        int hex_value = std::stoi(hex_string, nullptr, 16);
                        char current_char = static_cast<char>(hex_value);
                        tokens.push_back({std::string(1, current_char), "symbol"});
                        i += 3; // Skip '\', 'x', and two hex digits
                        continue;
                    } catch (const std::exception& e) {
                        // Not a valid hex sequence, treat \ as literal
                        tokens.push_back({std::string(1, c), "symbol"});
                        continue;
                    }
                } else {
                    tokens.push_back({std::string(1, next_char), "symbol"});
                    i += 1; // Skip the escaped character
                    continue;
                }
            }
            
            // Handle character classes [...]
            if (c == ']' && string_class) {
                string_class = false;
                tokens.push_back({std::string(1, c), std::string(1, c)});
            } else if (c == '[') {
                string_class = true;
                tokens.push_back({std::string(1, c), std::string(1, c)});
            } else if (string_class) {
                // Inside character class, treat everything as symbols except '-'
                if (c == '-') {
                    tokens.push_back({std::string(1, c), std::string(1, c)});
                } else {
                    tokens.push_back({std::string(1, c), "symbol"});
                }
            } else {
                // Handle regular tokens outside character classes
                if (std::find(fixed_tokens.begin(), fixed_tokens.end(), std::string(1, c)) != fixed_tokens.end()) {
                    tokens.push_back({std::string(1, c), std::string(1, c)});
                } else {
                    tokens.push_back({std::string(1, c), "symbol"});
                }
            }
        }
        
        tokens.push_back({"EOF", "EOF"}); // Add EOF token
        return tokens;
    }

    DFA build_dfa(SLR1Parser& parser) {
        std::vector<std::pair<std::string, std::string>> token_names = regex_tokenizer(pattern_);
        vector<string> tokens;
        for (int i = 0; i < token_names.size(); ++i) {
            tokens.push_back(token_names[i].second);
        }
        auto [production_ids, actions] = parser.Parse(tokens);
        //Debugg
        // std::cout << "Productions:" << std::endl;
        // for (const auto& production_id : production_ids) {
        //     std::cout << grammar_.GetProduction(production_id).ToString() << std::endl;
        // }
        // std::cout << "Actions:" << std::endl;
        // for (const auto& action : actions) {
        //     std::cout << action << std::endl;
        // }
        // Create a queue of productions from the production IDs
        std::queue<std::shared_ptr<AttrProd>> productions;
        for (const auto& production_id : production_ids) {
            // Use the production ID to get the corresponding AttrProd
            const auto& attr_prod_ref = grammar_.GetProduction(production_id);
            auto attr_prod = std::make_shared<AttrProd>(attr_prod_ref);
            productions.push(attr_prod);
        }
        auto ast = reverse_evaluate(productions, actions, token_names, grammar_);
        auto nfa = ast->evaluate();
        // Convert NFA to DFA
        DFA dfa = nfa_to_dfa(*nfa);
        DFA mini_dfa = automata_minimization(dfa);
        return mini_dfa;
    }
    DFA& Automaton() {
        return automaton_;
    }
    const std::string& Pattern() const {
        return pattern_;
    }

private:
    std::string pattern_;
    DFA automaton_; // DFA representation of the regex
    Grammar& grammar_; // Reference to the grammar used for parsing
    
    // Helper method to create an empty DFA for initialization
    static DFA createEmptyDFA() {
        return DFA(1, {}, {}, 0);
    }
};