#include "execute.h"
#include "hulkGrammar.hpp"
#include <iostream>

/**
 * Programa para crear los artefactos serializados del compilador Hulk
 */
int main(int argc, char* argv[]) {
    // Obtener la gramática de Hulk
    Grammar hulk_grammar = getHulkGrammar();
    // for (const auto& production : hulk_grammar.Productions()) {
    //     std::cout << "Producción: " << production.ToString() << std::endl;
    // }
    //buscar las producciones q tienen epsilon en parte derecha
    std::cout << "Producciones con epsilon:" << std::endl;
    for (const auto& production : hulk_grammar.Productions()) {
        for (const auto& symbol : production.Right().Symbols()) {
            if (symbol->IsEpsilon()) {
                std::cout << "Producción: " << production.ToString() << std::endl;
                break; // Solo necesitamos una vez por producción
            }
        }
    }

    // Crear los artefactos con salida verbosa
    bool verbose = true;
    
    // Ejecutar la función de creación de artefactos
    if (create_artifacts(hulk_grammar, verbose)) {
        std::cout << "Artefactos creados exitosamente en la carpeta 'hulk'" << std::endl;
        return 0;
    } else {
        std::cerr << "Error al crear los artefactos" << std::endl;
        return 1;
    }
}
