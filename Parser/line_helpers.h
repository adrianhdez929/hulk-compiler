#include <cstddef>  // Para offsetof
#pragma once

#include <algorithm>
#include <limits>
#include <vector>
#include <variant>
#include "../Ast/ast.hpp"
#include "../Lexer/Token.h"
#include "../Lexer/SpecialTypes.h"

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
 * @brief Extrae la línea de un vector de argumentos de ElementType siguiendo la lógica de Bison
 * @param args Vector de argumentos (Token o ASTNode*)
 * @return Línea del primer token o nodo válido, o 0 si no hay argumentos válidos
 */
inline int getMinLineFromArgs(const std::vector<ElementType>& args) {
    // Bison asigna la línea del primer símbolo en la regla
    for (size_t i = 0; i < args.size(); ++i) {
        const auto& arg = args[i];
        
        if (std::holds_alternative<Token>(arg)) {
            // Si es un token, obtener su línea directamente
            int line = std::get<Token>(arg).Line();
            if (line > 0) return line;
        } 
        else if (std::holds_alternative<ASTNode*>(arg)) {
            // Si es un nodo, verificar que su línea sea válida
            ASTNode* node = std::get<ASTNode*>(arg);
            if (node && node->line > 0) {
                return node->line;
            }
        }
        // Los strings no tienen información de línea, ignorar
    }
    
    // Si no encontramos ninguna línea válida, devolver 0
    return 0;
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
