/// \file execute.h
/// \brief Header para las funciones de compilación de Hulk

#pragma once

#include "Lexer/Lexer.h"
#include "Parser/LALR1Parser.h"
#include "Grammar/grammar.h"
#include "Ast/ast.hpp"
#include "Logger/Logger.h"
#include <string>
#include <vector>

/**
 * @brief Crea los artefactos necesarios para el compilador Hulk
 * @param verbose Si es true, imprime mensajes informativos durante el proceso
 * @return true si los artefactos se crearon correctamente, false en caso contrario
 */
bool create_artifacts(Grammar& hulk_grammar, bool verbose = false);

/**
 * @brief Compila un código fuente Hulk y devuelve su AST
 * @param source_code Código fuente Hulk a compilar
 * @param error_message Mensaje de error si ocurrió algún problema durante la compilación
 * @param verbose Si es true, imprime mensajes informativos durante el proceso
 * @return Puntero al AST generado, o nullptr si hubo errores
 */
ASTNode* compile_hulk(const std::string& source_code, std::string& error_message, Grammar& hulk_grammar, bool verbose = false);

/**
 * @brief Compila un archivo de código fuente Hulk y devuelve su AST
 * @param file_path Ruta al archivo de código fuente Hulk a compilar
 * @param error_message Mensaje de error si ocurrió algún problema durante la compilación
 * @param verbose Si es true, imprime mensajes informativos durante el proceso
 * @return Puntero al AST generado, o nullptr si hubo errores
 */
ASTNode* compile_hulk_file(const std::string& file_path, std::string& error_message, Grammar& hulk_grammar, bool verbose = false);

/**
 * @brief Carga y devuelve los artefactos necesarios para compilar (lexer y parser)
 * @param error_message Mensaje de error si ocurrió algún problema durante la carga
 * @param parser_grammar Referencia a la gramática a utilizar con el parser
 * @param verbose Si es true, imprime mensajes informativos durante el proceso
 * @return Par con el lexer y parser cargados, o nullptr si hubo errores
 */
std::pair<Lexer*, SLR1Parser*> load_compiler_artifacts(std::string& error_message, Grammar& parser_grammar, bool verbose = false);

/**
 * @brief Tokeniza un código fuente Hulk
 * @param source_code Código fuente a tokenizar
 * @param lexer Lexer a utilizar
 * @param hulk_grammar Gramática de Hulk
 * @param error_message Mensaje de error si ocurrió algún problema
 * @return Vector de tokens generados, o vector vacío si hubo errores
 */
std::vector<Token> tokenize_source(const std::string& source_code, Lexer* lexer, 
                                   Grammar& hulk_grammar, std::string& error_message);

/**
 * @brief Construye el AST a partir de tokens
 * @param tokens Vector de tokens a analizar
 * @param parser Parser a utilizar
 * @param hulk_grammar Gramática de Hulk
 * @param error_message Mensaje de error si ocurrió algún problema
 * @param verbose Si es true, imprime mensajes informativos durante el proceso
 * @return Puntero al AST generado, o nullptr si hubo errores
 */
ASTNode* build_ast_from_tokens(const std::vector<Token>& tokens, LALR1Parser* parser, 
                             Grammar& hulk_grammar, std::string& error_message, bool verbose = false);

/**
 * @brief Lee el contenido de un archivo de código fuente
 * @param file_path Ruta al archivo de código fuente (por defecto "script.hulk")
 * @param error_message Mensaje de error si ocurrió algún problema
 * @return Contenido del archivo como string, o string vacío si hubo errores
 */
std::string read_source_file(const std::string& file_path, std::string& error_message);

/**
 * @brief Valida la compatibilidad entre dos gramáticas
 * @param g1 Primera gramática
 * @param g2 Segunda gramática
 * @param verbose Si es true, imprime mensajes informativos durante la validación
 * @return true si las gramáticas son compatibles para parsing, false en caso contrario
 */
bool validate_grammar_compatibility(const Grammar& g1, const Grammar& g2, bool verbose = false);

/**
 * @brief Asegura el uso de la misma gramática para tokenización y parsing
 * @param grammar Gramática a utilizar
 * @return Referencia a la misma gramática
 */
Grammar& ensure_same_grammar(Grammar& grammar);


