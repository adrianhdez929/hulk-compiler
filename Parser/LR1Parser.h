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

/**
 * @class LR1ParsingError
 * @brief Excepción especializada para errores de análisis sintáctico en un parser LR(1).
 * 
 * Proporciona información detallada sobre errores durante el parseo LR(1), incluyendo
 * la posición del error, el token inesperado, y los tokens esperados.
 */
class LR1ParsingError : public std::runtime_error {
public:
    /**
     * @brief Constructor para error de token inesperado.
     * @param message Mensaje descriptivo del error
     * @param state Estado en que ocurrió el error
     * @param token Token que causó el error
     * @param expected_tokens Lista de tokens que serían válidos en este punto
     */
    LR1ParsingError(const std::string& message, int state, const std::string& token, 
                   const std::vector<std::string>& expected_tokens)
        : std::runtime_error(message),
          state_(state),
          token_(token),
          expected_tokens_(expected_tokens) {}
    
    /**
     * @brief Obtiene el estado donde ocurrió el error.
     */
    int getState() const { return state_; }
    
    /**
     * @brief Obtiene el token que causó el error.
     */
    std::string getToken() const { return token_; }
    
    /**
     * @brief Obtiene la lista de tokens esperados.
     */
    std::vector<std::string> getExpectedTokens() const { return expected_tokens_; }

private:
    int state_;
    std::string token_;
    std::vector<std::string> expected_tokens_;
};

// Funciones auxiliares para el analizador LR(1)
std::vector<std::shared_ptr<Symbol>> get_lr1_symbols(const std::vector<std::string>& symbols, Grammar& G);
std::vector<Item> lr1_expand(const Item& item, const std::map<Sentence, ContainerSet<string>>& firsts, Grammar& G);
std::set<Item> lr1_compress(const std::vector<Item>& items);
std::vector<Item> closure_lr1(const std::vector<Item>& items, const std::map<Sentence, ContainerSet<string>>& firsts, Grammar& G);
std::vector<Item> goto_lr1(const std::vector<Item>& items, std::shared_ptr<Symbol> symbol, const std::map<Sentence, ContainerSet<string>>& firsts, bool just_kernel, Grammar& G);
State build_lr1_automaton(Grammar& G);

/**
 * @class LR1Parser
 * @brief Implementa un parser LR(1) para analizar cadenas según una gramática dada.
 * 
 * Esta clase implementa un parser LR(1) con su propia clase de error LR1ParsingError.
 * A diferencia del SLR1Parser, usa la construcción de tablas específica del 
 * algoritmo LR(1) para mayor poder de análisis sintáctico.
 */
class LR1Parser {
public:
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
    LR1Parser(Grammar& G, bool verbose = false);
    
    /**
     * @brief Destructor.
     */
    ~LR1Parser();

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

    /**
     * @brief Calcula los conjuntos FIRST para la gramática.
     */
    map<Sentence, ContainerSet<string>> compute_firsts();
    
    /**
     * @brief Calcula los conjuntos FIRST locales para una sentencia.
     * @param sentence La sentencia para la que calcular FIRST
     * @param firsts Mapa con los conjuntos FIRST ya calculados
     * @param G La gramática
     * @param verbose Si es true, muestra información de depuración
     */
    static ContainerSet<string> compute_local_firsts(const Sentence& sentence, const map<Sentence, ContainerSet<string>>& firsts, const Grammar& G, bool verbose = false);
    
    /**
     * @brief Calcula los conjuntos FOLLOW para la gramática.
     */
    map<Sentence, ContainerSet<string>> compute_follows(const map<Sentence, ContainerSet<string>>& firsts);

    /**
     * @brief Construye el autómata LR(1) para la gramática.
     */
    State BuildLR1Automaton();

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
     * @brief Libera la memoria de los estados del autómata LR(1).
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
    static LR1Parser* deserialize_parser(const std::string& filename, Grammar& grammar);
    
    /**
     * Crea un nuevo Parser desde un archivo serializado en una carpeta específica
     * @param filename Nombre del archivo con el parser serializado
     * @param directory Carpeta donde buscar el archivo
     * @param grammar Referencia a la gramática que usará el parser
     * @return Puntero a un nuevo Parser deserializado, o nullptr si hay error
     */
    static LR1Parser* deserialize_parser(const std::string& filename, const std::string& directory, Grammar& grammar);

private:
    Grammar& G_;
    bool verbose_;
    std::map<std::pair<int, Symbol>, std::pair<std::string, int>> action_;
    std::map<std::pair<int, Symbol>, int> goto_;
    std::vector<State*> automaton_states_;
    
    // Constructor privado para deserialización
    LR1Parser(Grammar& G, 
               const std::map<std::pair<int, Symbol>, std::pair<std::string, int>>& action,
               const std::map<std::pair<int, Symbol>, int>& goto_table,
               bool verbose = false);

    /**
     * @brief Obtiene los tokens esperados para un estado dado.
     * @param state_id El ID del estado para el que se buscan tokens válidos
     * @return Vector de nombres de tokens que son válidos en ese estado
     */
    std::vector<std::string> getExpectedTokens(int state_id) const;
    
    /**
     * @brief Genera un mensaje de error para un error de sintaxis.
     * @param state_id El estado en que se encontró el error
     * @param token El token inesperado
     * @return Par con mensaje de error y lista de tokens esperados
     */
    std::pair<std::string, std::vector<std::string>> generateErrorMessage(int state_id, const std::string& token) const;

    /**
     * @brief Genera un mensaje de error con contexto para un error de sintaxis.
     * @param tokens Vector de tokens de entrada
     * @param error_position Posición donde ocurrió el error
     * @param error_message Mensaje base del error
     * @return Mensaje de error con contexto visual
     */
    static std::string formatErrorWithContext(const std::vector<Terminal>& tokens, 
                                             int error_position, 
                                             const std::string& error_message);
                                            
    /**
     * @brief Genera un mensaje de error con contexto para un error de sintaxis, usando tokens con posición.
     * @param tokens Vector de tokens con información de posición
     * @param error_position Posición donde ocurrió el error
     * @param error_message Mensaje base del error
     * @return Mensaje de error con contexto visual y posición precisa
     */
    static std::string formatErrorWithContext(const std::vector<Token>& tokens, 
                                             int error_position, 
                                             const std::string& error_message);
};
