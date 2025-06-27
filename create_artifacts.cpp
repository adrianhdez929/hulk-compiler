#include "execute.h"
#include "hulkGrammar.hpp"
#include "Logger/Logger.h"
#include <iostream>

/**
 * Programa para crear los artefactos serializados del compilador Hulk
 */
int main(int argc, char* argv[]) {
    // Inicializar el sistema de registro
    Logger::initialize("hulk_artifacts.log", LogLevel::DEBUG, "Artifact Creator");
    LogInfo("=== HULK Artifact Creator Starting ===");

    // Obtener la gramática de Hulk
    LogInfo("Obteniendo la gramática de Hulk");
    Grammar hulk_grammar = getHulkGrammar();
    LogInfo("Gramática obtenida con " + std::to_string(hulk_grammar.Productions().size()) + " producciones");
    
    // Verificar producciones con epsilon
    int epsilon_productions = 0;
    for (const auto& production : hulk_grammar.Productions()) {
        for (const auto& symbol : production.Right().Symbols()) {
            if (symbol->IsEpsilon()) {
                epsilon_productions++;
                LogDebug("Producción con epsilon: " + production.ToString());
                break; // Solo necesitamos una vez por producción
            }
        }
    }
    LogInfo("Se encontraron " + std::to_string(epsilon_productions) + " producciones con epsilon");

    // Crear los artefactos con salida verbosa
    bool verbose = true;
    
    LogInfo("Comenzando la creación de artefactos");
    // Ejecutar la función de creación de artefactos
    if (create_artifacts(hulk_grammar, verbose)) {
        LogInfo("=== Artefactos creados correctamente ===");
        return 0;
    } else {
        LogError("Error al crear los artefactos");
        return 1;
    }
}
