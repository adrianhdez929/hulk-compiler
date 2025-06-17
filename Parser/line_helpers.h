#pragma once

#include <algorithm>
#include <limits>
#include <vector>
#include <variant>
#include "../Ast/ast.hpp"
#include "../Lexer/Token.h"
#include "../Lexer/SpecialTypes.h"

/**
 * @brief Extrae la línea de un Token
 * @param token Token del que extraer la línea
 * @return Línea del token o 0 si es nulo
 */
inline int getLineFromToken(const Token* token) {
    return token ? token->Line() : 0;
}

/**
 * @brief Extrae la línea de un ASTNode
 * @param node Nodo del que extraer la línea
 * @return Línea del nodo o 0 si es nulo o no tiene línea asignada
 */
inline int getLineFromNode(const ASTNode* node) {
    return (node && node->line > 0) ? node->line : 0;
}

/**
 * @brief Obtiene la línea mínima de dos valores de línea
 * @param line1 Primera línea
 * @param line2 Segunda línea
 * @return Línea mínima que sea válida (>0) o 0 si ninguna es válida
 */
inline int minLine(int line1, int line2) {
    if (line1 <= 0) return line2;
    if (line2 <= 0) return line1;
    return std::min(line1, line2);
}

/**
 * @brief Extrae la línea mínima de un vector de argumentos de ElementType
 * @param args Vector de argumentos (Token o ASTNode*)
 * @return Línea mínima encontrada o 0 si no se encuentra ninguna línea válida
 */
inline int getMinLineFromArgs(const std::vector<ElementType>& args) {
    int minLine = std::numeric_limits<int>::max();
    bool foundValidLine = false;
    
    for (const auto& arg : args) {
        int line = 0;
        
        if (std::holds_alternative<std::string>(arg)) {
            // Los strings no tienen información de línea
            continue;
        } 
        else if (std::holds_alternative<ASTNode*>(arg)) {
            ASTNode* node = std::get<ASTNode*>(arg);
            if (node) {
                // // Intentar convertir a Token primero
                // const Token* token = dynamic_cast<const Token*>(node);
                // if (token) {
                //     line = token->Line();
                // } else {
                //     line = node->line;
                // }
                line = node->line; // Usar la línea del nodo directamente
            }
        }
        else if (std::holds_alternative<Token>(arg)) {
            line = std::get<Token>(arg).Line();
        }
        
        if (line > 0) {
            minLine = std::min(minLine, line);
            foundValidLine = true;
        }
    }
    //Si no se encontró ninguna línea válida, devolver el maximo valor posible de int
    return foundValidLine ? minLine : std::numeric_limits<int>::max();
}

/**
 * @brief Helper function para casos simples de dos nodos
 * @param node1 Primer nodo
 * @param node2 Segundo nodo
 * @return Línea mínima entre los dos nodos
 */
inline int getMinLineFromNodes(ASTNode* node1, ASTNode* node2) {
    int line1 = getLineFromNode(node1);
    int line2 = getLineFromNode(node2);
    return minLine(line1, line2);
}
