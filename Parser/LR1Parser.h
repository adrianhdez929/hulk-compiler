#pragma once

#include "SLR1Parser.h"
#include <vector>
#include <string>

class ASTNode;

/**
 * @class LR1Parser
 * @brief Implementa un parser LR(1) para el análisis sintáctico.
 * 
 * Esta es una versión simplificada para comenzar a depurar el proyecto.
 */
class LR1Parser : public SLR1Parser {
public:
    /**
     * @brief Constructor del parser LR(1).
     * @param G Gramática sobre la que se construye el parser.
     * @param verbose Si es true, muestra información adicional durante la construcción.
     */
    LR1Parser(Grammar& G, bool verbose = false);
    
    /**
     * @brief Destructor virtual.
     */
    virtual ~LR1Parser() = default;
    
    /**
     * @brief Obtiene el AST construido después de analizar una entrada.
     * @return Vector con los nodos AST generados durante el análisis.
     */
    const std::vector<ASTNode*>& GetAST() const;

    /**
     * @brief Redefine el método Parse para manejar la construcción del AST.
     * @param tokens Vector de strings que representan los tokens de entrada.
     * @return Pair con producciones usadas y nombres de acciones.
     */
    std::pair<std::vector<int>, std::vector<std::string>>
    Parse(const std::vector<std::string>& tokens) override;

protected:
    /**
     * @brief Construye las tablas de análisis sintáctico (action y goto).
     * 
     * Este método sobrescribe el de la clase base para utilizar algoritmos LR(1).
     */
    void BuildParsingTable() override;

private:
    /**
     * @brief Vector para almacenar los nodos AST generados durante el análisis.
     */
    std::vector<ASTNode*> ast_nodes_;
};
};
