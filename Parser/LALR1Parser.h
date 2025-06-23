#pragma once
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include "../Grammar/grammar.h"
#include "../Automata/state.h"
#include "../Lexer/Token.h"
#include "Item.h"
#include <stack>


class LALR1Parser {
public:
    // Constants for parsing actions
    static constexpr const char* SHIFT = "SHIFT";
    static constexpr const char* REDUCE = "REDUCE";
    static constexpr const char* OK = "OK";
    static constexpr const char* ERROR = "ERROR";

    /**
     * @brief Constructor del parser LR(1).
     * @param G Gramática sobre la que se construye el parser.
     * @param verbose Si es true, muestra información adicional durante la construcción.
     */
    LALR1Parser(Grammar& G, bool verbose = false);

    /**
     * @brief Destructor.
     */
    ~LALR1Parser();

    /**
     * @brief Construye las tablas de acción y goto del parser LR(1).
     */
    void BuildParsingTable();

    /**
     * @brief Parsea una secuencia de tokens Terminal.
     * @param tokens Vector de tokens Terminal.
     * @return Par de vectores: ids de producciones y acciones realizadas.
     * @throws LR1ParsingError Si ocurre un error de sintaxis, con información detallada.
     */
    std::pair<std::vector<int>, std::vector<std::string>> 
    Parse(const std::vector<Terminal>& tokens);
    
    /**
     * @brief Parsea una secuencia de tokens como strings.
     * @param tokens Vector de strings (nombres de tokens).
     * @return Par de vectores: ids de producciones y acciones realizadas.
     * @throws LR1ParsingError Si ocurre un error de sintaxis, con información detallada.
     */
    std::pair<std::vector<int>, std::vector<std::string>>
    Parse(const std::vector<std::string>& tokens);

    /**
     * @brief Parsea una secuencia de tokens con información de posición.
     * @param tokens Vector de objetos Token con información de línea y columna.
     * @return Par de vectores: ids de producciones y acciones realizadas.
     * @throws LR1ParsingError Si ocurre un error de sintaxis, con información detallada.
     */
    std::pair<std::vector<int>, std::vector<std::string>>
    Parse(const std::vector<Token>& tokens);

    void BuildParsingTable();

private:
    Grammar& G_;
    bool verbose_;
    std::map<std::string, std::set<std::string>> firsts_sets_;
    // std::map<Symbol, std::set<Symbol>> follows_sets_;
    std::vector<State*> states_;
    std::map<std::pair<int, Symbol>, std::pair<std::string, int>> action_;
    std::map<std::pair<int, Symbol>, int> goto_;

    void ComputeFirstSets();
    void BuildAutomaton();
    void GenerateParsingTable();
    std::set<Item> Closure(const std::set<Item>& items) const;
    void ComputeFirstsForSentence(const Sentence& sentence, std::set<std::string>& first_set) const;
    bool IsNullable(const Sentence& sentence) const;
    std::set<Item> GetKernel(const std::set<Item>& items) const;
};