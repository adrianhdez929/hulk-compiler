#pragma once
#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <filesystem>
#include "../Grammar/grammar.h"
#include "../Automata/state.h"

/**
 * @class SLR1Parser
 * @brief Implementa un parser SLR(1) para analizar cadenas según una gramática dada.
 * 
 * Esta clase construye las tablas de análisis sintáctico (action y goto) para una gramática,
 * permite parsear secuencias de tokens, y soporta serialización/deserialización del parser.
 * Utiliza autómatas LR(0) y los conjuntos FIRST y FOLLOW para construir las tablas.
 */
class SLR1Parser {
public:
    static constexpr const char* SHIFT = "SHIFT";
    static constexpr const char* REDUCE = "REDUCE";
    static constexpr const char* OK = "OK";

    /**
     * @brief Constructor del parser SLR(1).
     * @param G Gramática sobre la que se construye el parser.
     * @param verbose Si es true, muestra información adicional durante la construcción.
     */
    SLR1Parser(Grammar& G, bool verbose = false);
    
    /**
     * @brief Destructor virtual.
     */
    virtual ~SLR1Parser();

    /**
     * @brief Parsea una secuencia de tokens Terminal.
     * @param tokens Vector de tokens Terminal.
     * @return Par de vectores: ids de producciones y acciones realizadas.
     */
    std::pair<std::vector<int>, std::vector<std::string>> 
    Parse(const std::vector<Terminal>& tokens);
    
    /**
     * @brief Parsea una secuencia de tokens como strings.
     * @param tokens Vector de strings (nombres de tokens).
     * @return Par de vectores: ids de producciones y acciones realizadas.
     */
    std::pair<std::vector<int>, std::vector<std::string>>
    Parse(const std::vector<std::string>& tokens);

    /**
     * @brief Construye las tablas de análisis sintáctico (action y goto).
     */
    void BuildParsingTable();

    /**
     * @brief Calcula los conjuntos FIRST para la gramática.
     */
    map<Sentence, ContainerSet<string>> compute_firsts();
    /**
     * @brief Calcula los conjuntos FIRST locales para una sentencia.
     */
    ContainerSet<string> compute_local_firsts(const Sentence& sentence, const map<Sentence, ContainerSet<string>>& symbol_firsts);
    /**
     * @brief Calcula los conjuntos FOLLOW para la gramática.
     */
    map<Sentence, ContainerSet<string>> compute_follows(const map<Sentence, ContainerSet<string>>& symbol_firsts);

    /**
     * @brief Construye el autómata LR(0) para la gramática.
     */
    State BuildLR0Automaton();

    /**
     * @brief Registra una acción en la tabla action.
     */
    void Register(std::map<std::pair<int, Symbol>, std::pair<std::string, int>>& table, 
                  const std::pair<int, Symbol>& key, 
                  const std::pair<std::string, int>& value);
    /**
     * @brief Registra una transición en la tabla goto.
     */
    void Register(std::map<std::pair<int, Symbol>, int>& table,
                  const std::pair<int, Symbol>& key, 
                  int value);
    /**
     * @brief Libera la memoria de los estados del autómata LR(0).
     */
    void CleanupAutomatonStates();

    // ============= MÉTODOS DE SERIALIZACIÓN DEL PARSER =============
    
    /**
     * Serializa el Parser completo a un archivo
     * @param filename Nombre del archivo donde guardar el parser
     * @return true si la serialización fue exitosa, false en caso contrario
     */
    bool serialize_parser(const std::string& filename) const;
    
    /**
     * Serializa el Parser completo a un archivo en una carpeta específica
     * @param filename Nombre del archivo donde guardar el parser
     * @param directory Carpeta donde guardar el archivo (por defecto "hulk")
     * @return true si la serialización fue exitosa, false en caso contrario
     */
    bool serialize_parser(const std::string& filename, const std::string& directory) const;
    
    /**
     * Crea un nuevo Parser desde un archivo serializado
     * @param filename Nombre del archivo con el parser serializado
     * @param grammar Referencia a la gramática que usará el parser
     * @return Puntero a un nuevo Parser deserializado, o nullptr si hay error
     */
    static SLR1Parser* deserialize_parser(const std::string& filename, Grammar& grammar);
    
    /**
     * Crea un nuevo Parser desde un archivo serializado en una carpeta específica
     * @param filename Nombre del archivo con el parser serializado
     * @param directory Carpeta donde buscar el archivo
     * @param grammar Referencia a la gramática que usará el parser
     * @return Puntero a un nuevo Parser deserializado, o nullptr si hay error
     */
    static SLR1Parser* deserialize_parser(const std::string& filename, const std::string& directory, Grammar& grammar);

private:
    Grammar& G_;
    bool verbose_;
    std::map<std::pair<int, Symbol>, std::pair<std::string, int>> action_;
    std::map<std::pair<int, Symbol>, int> goto_;
    std::vector<State*> automaton_states_;
    
    // Constructor privado para deserialización
    SLR1Parser(Grammar& G, 
               const std::map<std::pair<int, Symbol>, std::pair<std::string, int>>& action,
               const std::map<std::pair<int, Symbol>, int>& goto_table,
               bool verbose = false);
};
