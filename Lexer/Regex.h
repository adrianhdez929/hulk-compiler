#pragma once
#include <string>
#include <vector>
#include <regex>
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
        for (char c : input) {
            if (std::find(fixed_tokens.begin(), fixed_tokens.end(), std::string(1, c)) != fixed_tokens.end()) {
                tokens.push_back({std::string(1, c), std::string(1, c)});
            } else {
                tokens.push_back({std::string(1, c), "symbol"});
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
        dfa = automata_minimization(dfa);
        return dfa;
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