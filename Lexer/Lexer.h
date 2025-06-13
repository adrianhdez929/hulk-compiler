#pragma once
#include <string>
#include <vector>
#include <cassert>
#include "../Automata/state.h"
#include "Regex.h"
#include "../Automata/nfa.h"

class Lexer {
public:
    Lexer(std::vector<std::pair<std::string, std::string>> token_table,  
        Grammar& grammar, SLR1Parser& parser): regexs_(build_regex(token_table, grammar, parser)), automaton_(build_automaton()) {}

    std::vector<State> build_regex(std::vector<std::pair<std::string, std::string>> table, Grammar& grammar_, SLR1Parser& parser) {
        std::vector<State> states;
        int index = 0;
        for (const auto& [name, pattern] : table) {
            NFA nfa = Regex(pattern, grammar_, parser).Automaton();
            State state = *State::from_nfa(nfa);
            for (const auto& s : state.get_all_states()) {
                if (s->is_final()) {
                    s->set_tag(name, index++);
                }
            }
            index++;
            states.push_back(state);
        }
        return states;
    }

    State build_automaton() {
        int id = 1;
        for (auto& state : regexs_) {
            for (auto& s : state.get_all_states()) {
                s->set_id(id++);
            }
        }
        State start = State(0);
        for (auto& state : regexs_) {
            start.add_epsilon_transition(&state);
        }
        return *start.to_deterministic();
    }

    std::pair<State, std::string> walk(const std::string& input) {
        State* current_state = &automaton_;
        State final_state;// final_state: id = -1, is_final = false
                        // CUIDADO CON EL ID. Ningun id al final debe ser -1.
        if (current_state->is_final()) {
            final_state = *current_state;
        }
        std::string token;
        std::string lex;
        for (char c : input) {
            bool found_transition = false;
            for (const auto& [symbol, next_states] : current_state->transitions()) {
                if (symbol == std::string(1, c)) {
                    // assert (next_states.size() == 1) && "Automaton should be deterministic.";
                    if (next_states.size() != 1) {
                        throw std::runtime_error("Automaton is not deterministic, multiple transitions found for symbol: " + std::string(1, c));
                    }
                    current_state = next_states[0];
                    lex += c;
                    found_transition = true;
                    if (current_state->is_final()) {
                        token = lex;
                        final_state = *current_state;
                    }
                }
                if (found_transition) break;
            }
            if (!found_transition) {
                if (!token.empty()) {
                    // If we found a token, return it
                    return {final_state, token};
                }
                // If no token found, return error state
                return {State(-1), ""}; // No transition found, return error state
            }
        }
        return {final_state, token};
    }

    std::vector<std::pair<std::string, std::string>> tokenize(const std::string& input) {
        std::vector<std::pair<std::string, std::string>> tokens;
        std::string remaining_text = input;
        while (!remaining_text.empty()) {
            auto [state, token] = walk(remaining_text);
            //Si hay un estado
            if (state.id() != -1) {
                std::vector<State> sorted_states;
                for (const State* s : state.get_nfa_states()) {
                    sorted_states.push_back(*s);
                }
                // Ordenarlos de menor a mayor segun el State.n_
                std::sort(sorted_states.begin(), sorted_states.end(), [](const State& a, const State& b) {
                    return a.get_n() < b.get_n();
                });
                tokens.push_back(std::make_pair(token, sorted_states[0].tag()));
                remaining_text = remaining_text.substr(token.size());
            }
            else {
                tokens.push_back(std::make_pair(std::string(1, remaining_text[0]), "ERROR"));
                remaining_text = remaining_text.substr(1); // Skip one character on error
            }
        }
        tokens.push_back(std::make_pair("EOF","EOF"));
        return tokens;
    }

private:
    std::vector<State> regexs_;
    State automaton_;
};
