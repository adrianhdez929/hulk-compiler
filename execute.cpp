/// \file execute.cpp
/// \brief Archivo para la ejecución del compilador Hulk

#include "execute.h"
#include "Lexer/grammar_parser.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include "Parser/reverse_evaluate.h"
#include "hulkGrammar.hpp"
#include "Parser/LALR1Parser.h"
// #include "validate_grammar.hpp"

bool create_artifacts(Grammar& hulk_grammar, bool verbose) {
    // Initialize logger for this operation
    if (verbose) {
        LogInfo("Iniciando creación de artefactos del compilador");
    }

    // Reset production counter to ensure fresh start
    Grammar::ResetProductionCounter();
    LogDebug("Contador de producciones reiniciado");

    // Verificar y crear directorio hulk
    if (!std::filesystem::exists("hulk")) {
        std::filesystem::create_directory("hulk");
        LogInfo("Directorio 'hulk' creado");
    } else {
        LogDebug("Directorio 'hulk' ya existe");
    }

    // Crear gramática del lexer
    if (verbose) {
        LogInfo("Analizando gramática del lexer desde Lexer/grammar.txt");
    }
    Grammar lexer_grammar = GrammarParser::Parse("Lexer/grammar.txt");
    LogInfo("Gramática del lexer analizada con éxito (" + std::to_string(lexer_grammar.Productions().size()) + " producciones)");
    
    // Crear parser
    if (verbose) {
        LogInfo("Creando parser LALR1 para la gramática del lexer");
    }
    LALR1Parser parser(lexer_grammar, verbose); // Usando LALR1Parser con modo verbose
    LogDebug("Parser LALR1 para el lexer creado");
    
    // Serializar parser
    if (verbose) {
        LogInfo("Serializando parser en hulk/parser.p");
    }
    
    if (!parser.serialize_parser("parser.p", "hulk")) {
        std::cerr << "Error: No se pudo serializar el parser" << std::endl;
        LogError("Error: No se pudo serializar el parser del lexer");
        return false;
    }
    
    LogInfo("Parser serializado exitosamente en hulk/parser.p");
    
    // Crear tabla de tokens para el lexer
    std::vector<std::pair<std::string, std::string>> token_table = {
        {"string", "\"|([\\x20-!#-\\x7e])*|\""},
        {"number", "(0|[1-9]|[0-9]*)|(\\.|[0-9]+)?"}, // Regular expression for numbers
        {"bool", "true|false"},  // Regular expression for boolean values

        {"space", " +"}, // Regular expression for spaces
        // Regular expression for identifiers
        {"(", "\\("},            // Left parenthesis
        {")", "\\)"},            // Right parenthesis
		{"{", "\\{"},
		{"}", "\\}"},
		{";", "\\;"},
		{"+", "\\+"},
		{"-", "\\-"},
		{"*", "\\*"},
		{"/", "\\/"},
		{"^", "\\^"},
        {"%", "\\%"},
        {":=", ":="},          // Alternative assignment operator
        {":", "\\:"},
        {"|", "\\|"},
        {"&", "\\&"},
        {">=", ">="},
        {">", "\\>"},
        {"<=", "<="},
        {"<", "<"},
        {"==", "=="},
        {"!=", "!="},
        {"is", "is"},
        {"!", "!"},
        {",", "\\,"},           // Comma for lists
        {"=", "="},            // Assignment operator
        
        {"function", "function"}, // Function keyword
        {"=>", "=>"},          // Arrow for inline functions
        {"let", "let"},        // Let keyword
        {"in", "in"},          // In keyword for let expressions
        {"if", "if"},          // If keyword
        {"else", "else"},      // Else keyword
        {"elif", "elif"},      // Else-if keyword
        {"while", "while"},    // While keyword
        {"type", "type"},      // Type definition keyword
        {"inherits", "inherits"}, // Inheritance keyword
        {".", "\\."},          // Member access operator
        {"@", "@"},            // At operator
        {"@@", "@@"},          // Double at operator
        {"as", "as"},          // Type cast keyword
        {"for", "for"},        // For loop keyword 
        {"new", "new"},         // Object instantiation keyword
        // {"type_id", "[A-Z][_a-zA-Z0-9]*"},
        {"var_id", "[a-zA-Z][_a-zA-Z0-9]*"},
        {"tab", "\t"},
        {"newline", "\n"} // Regular expression for newlines
    };
    // 	std::vector<std::pair<std::string, std::string>> token_table = {
    //     {"string", "\"([\\x20-!#-\\x7e])*\""},
    //     {"number", "(0|[1-9][0-9]*)(.[0-9]+)?"}, // Regular expression for numbers
    //     {"bool", "true|false"},  // Regular expression for boolean values
    //     {"type_id", "[A-Z][_a-zA-Z0-9]*"},
    //     {"var_id", "[_a-z][_a-zA-Z0-9]*"},
    //     {"space", " +"}, // Regular expression for spaces
    //     // Regular expression for identifiers
    //     {"(", "\\("},            // Left parenthesis
    //     {")", "\\)"},            // Right parenthesis
	// 	{"{", "\\{"},
	// 	{"}", "\\}"},
	// 	{";", "\\;"},
	// 	{"+", "\\+"},
	// 	{"-", "\\-"},
	// 	{"*", "\\*"},
	// 	{"/", "\\/"},
	// 	{"^", "\\^"},
    //     {"tab", "\t"},
    //     {"newline", "\n"}, // Regular expression for newlines
    //     {"EOF", "EOF"}      // End of file token
    // };
    
    if (verbose) {
        LogInfo("Creando tabla de tokens para el lexer (" + std::to_string(token_table.size()) + " tokens definidos)");
    }
    
    // Crear lexer
    LogInfo("Creando lexer con la tabla de tokens y gramática");
    Lexer lexer(token_table, lexer_grammar, parser, verbose); // Pasando el parámetro verbose
    
    // Serializar lexer
    if (verbose) {
        LogInfo("Serializando lexer en hulk/lexer.l");
    }
    if (!lexer.serialize_lexer("lexer.l", "hulk")) {
        std::cerr << "Error: No se pudo serializar el lexer" << std::endl;
        LogError("Error: No se pudo serializar el lexer");
        return false;
    }
    
    LogInfo("Lexer serializado exitosamente en hulk/lexer.l");
    
    if (verbose) {

    }

    // Crear gramatica de Hulk
    Grammar::ResetProductionCounter();  // Reset before creating Hulk grammar
    LogInfo("Contador de producciones reiniciado para la gramática de Hulk");

    // Crear parser de Hulk
    if (verbose) {
        LogInfo("Creando parser LALR1 para la gramática de Hulk");
        LogDebug("La gramática de Hulk tiene " + std::to_string(hulk_grammar.Productions().size()) + 
                " producciones y " + std::to_string(hulk_grammar.NonTerminals().size()) + " no terminales");
    }
    LALR1Parser hulk_parser(hulk_grammar, verbose); // Modo verbose activado
    
    // Serializar parser de Hulk
    if (verbose) {
        LogInfo("Serializando parser de Hulk en hulk/hulk_parser.p");
    }
    
    if (!hulk_parser.serialize_parser("hulk_parser.p", "hulk")) {
        std::cerr << "Error: No se pudo serializar el parser de Hulk" << std::endl;
        LogError("Error: No se pudo serializar el parser de Hulk");
        return false;
    }
    
    LogInfo("Parser de Hulk serializado exitosamente en hulk/hulk_parser.p");
    LogInfo("Artefactos del compilador creados exitosamente");
    return true;
}

std::pair<Lexer*, LALR1Parser*> load_compiler_artifacts(std::string& error_message, Grammar& parser_grammar, bool verbose) {
    if (verbose) {
        LogInfo("Cargando artefactos del compilador");
    }
    
    // Verificar que existen los artefactos
    if (!std::filesystem::exists("hulk")) {
        if (verbose) {
            LogInfo("No se encontró el directorio 'hulk', creándolo");
        }
        std::filesystem::create_directory("hulk");
        LogDebug("Directorio 'hulk' creado");
    } else if (verbose) {
        LogDebug("Directorio 'hulk' encontrado");
    }
    
    // Verificar si necesitamos crear los artefactos
    static bool force_recreate = false;  // Esta variable se puede cambiar para forzar la recreación
    bool need_recreation = false;
    
    if (force_recreate) {
        if (verbose) {
            LogInfo("Forzando recreación de artefactos del compilador");
        }
        need_recreation = true;
    } else if (!std::filesystem::exists("hulk/lexer.l") || !std::filesystem::exists("hulk/parser.p")) {
        if (verbose) {
            LogInfo("Artefactos faltantes detectados:");
            if (!std::filesystem::exists("hulk/lexer.l")) {
                LogInfo("  - No se encontró 'hulk/lexer.l'");
            }
            if (!std::filesystem::exists("hulk/parser.p")) {
                LogInfo("  - No se encontró 'hulk/parser.p'");
            }
            LogInfo("Se procederá a crear los artefactos faltantes");
        }
        need_recreation = true;
    } else if (verbose) {
        LogDebug("Artefactos del compilador encontrados");
    }
    
    if (need_recreation) {
        LogInfo("Iniciando creación de artefactos necesarios");
        if (!create_artifacts(parser_grammar, verbose)) {
            error_message = "No se pudieron crear los artefactos necesarios para el compilador";
            LogError(error_message);
            return {nullptr, nullptr};
        }
        LogInfo("Artefactos creados exitosamente");
    }

    // Cargar Lexer y Parser
    if (verbose) {
        LogInfo("Cargando lexer desde hulk/lexer.l");
    }
    
    Lexer* lexer = Lexer::deserialize_lexer("lexer.l", "hulk");
    if (!lexer) {
        error_message = "No se pudo cargar el lexer";
        LogError(error_message);
        return {nullptr, nullptr};
    }
    
    if (verbose) {
        LogInfo("Lexer cargado exitosamente");
        
        // Logging información útil sobre la gramática
        LogDebug("Información de la gramática del parser:");
        LogDebug("  Total de producciones: " + std::to_string(parser_grammar.Productions().size()));
        LogDebug("  Total de no terminales: " + std::to_string(parser_grammar.NonTerminals().size()));
        LogDebug("  Total de terminales: " + std::to_string(parser_grammar.Terminals().size()));
        
        // Logging del símbolo inicial
        for (const auto& nonterminal : parser_grammar.NonTerminals()) {
            if (nonterminal == parser_grammar.GetStartSymbol()) {
                LogDebug("  Símbolo inicial: " + nonterminal->Name());
                break;
            }
        }

        // // Mostrar no terminales clave
        // std::cout << "  No terminales clave:" << std::endl;
        // for (const auto& nonterminal : parser_grammar.NonTerminals()) {
        //     if (nonterminal == parser_grammar.GetStartSymbol()) {
        //         std::cout << "    - Símbolo inicial: " << nonterminal->Name() << std::endl;
        //     }
        // }
        
        // Logging algunas producciones importantes
        int shown_prods = 0;
        for (const auto& prod : parser_grammar.Productions()) {
            // Mostrar producciones del símbolo inicial y algunas que involucren paréntesis o rangos
            if (prod.Left() == parser_grammar.GetStartSymbol() || 
                prod.ToString().find("(") != std::string::npos ||
                prod.ToString().find("[") != std::string::npos) {
                LogDebug("  Producción: " + prod.ToString());
                shown_prods++;
            }
            if (shown_prods >= 10) break; // Limitar el número de producciones mostradas
        }
    }

    if (verbose) {
        LogInfo("Cargando parser desde hulk/hulk_parser.p");
    }

    LALR1Parser* parser = LALR1Parser::deserialize_parser("hulk_parser.p", "hulk", parser_grammar);
    if (!parser) {
        delete lexer; // Liberar memoria del lexer cargado
        error_message = "No se pudo cargar el parser. Es posible que la gramática utilizada para crear el parser serializado sea diferente de la gramática actual. Intenta recrear los artefactos.";
        LogError(error_message);
        if (verbose) {
            LogError("ERROR: No se pudo cargar el parser desde 'hulk/hulk_parser.p'");
        }
        return {nullptr, nullptr};
    }
    
    if (verbose) {
        LogInfo("Parser cargado correctamente");
    }
    
    // Verificar que el parser tenga producciones válidas
    if (verbose) {
        LogInfo("Validando producciones del parser después de deserialización");
        
        // Imprimir algunas producciones para verificar que estén correctas
        const auto& productions = parser_grammar.Productions();
        if (!productions.empty()) {
            LogDebug("Primera producción: " + productions[0].ToString() + " (ID: " + std::to_string(productions[0].get_id()) + ")");
            if (productions.size() > 1) {
                LogDebug("Segunda producción: " + productions[1].ToString() + " (ID: " + std::to_string(productions[1].get_id()) + ")");
            }
        }
    }

    return {lexer, parser};
}

// Sobrecarga eliminada: usamos parámetros por defecto en lugar de sobrecargas

std::vector<Token> tokenize_source(const std::string& source_code, Lexer* lexer, 
                                  Grammar& hulk_grammar, std::string& error_message) {
    try {
        LogInfo("Tokenizando código fuente (" + std::to_string(source_code.size()) + " bytes)");
        // Tokenizar el código fuente con información de posición
        auto tokens = lexer->tokenize_with_positions(source_code, hulk_grammar);
        
        if (tokens.empty()) {
            error_message = "No se generaron tokens del código fuente";
            LogError(error_message);
            return {};
        }
        
        LogInfo("Tokenización completada con éxito, generados " + std::to_string(tokens.size()) + " tokens");
        return tokens;
    } catch (const std::runtime_error& e) {
        error_message = std::string("Error de tokenización: ") + e.what();
        LogError(error_message);
        return {};
    }
}

ASTNode* build_ast_from_tokens(const std::vector<Token>& tokens, LALR1Parser* parser, 
                              Grammar& hulk_grammar, std::string& error_message, bool verbose) {
    try {
        if (verbose) {
            LogInfo("Iniciando análisis sintáctico con " + std::to_string(tokens.size()) + " tokens");
            
            // Logging de tokens (solo en modo DEBUG)
            LogDebug("Tokens generados:");
            int token_count = 0;
            for (const auto& token : tokens) {
                if (token_count < 20) { // Limitar número de tokens mostrados en log
                    LogDebug("  " + token.ToString());
                    token_count++;
                } else if (token_count == 20) {
                    LogDebug("  ... y " + std::to_string(tokens.size() - 20) + " tokens más");
                    token_count++;
                }
            }

            // Verificar integridad de producciones antes del parsing
            LogDebug("Verificando integridad de producciones...");
            LogDebug("Total de producciones: " + std::to_string(hulk_grammar.Productions().size()));
        }
        
        // Realizar análisis sintáctico
        LogInfo("Ejecutando análisis sintáctico");
        auto parse_result = parser->Parse(tokens);
        
        if (verbose) {
            LogInfo("Análisis sintáctico exitoso. Obtenidos " + std::to_string(parse_result.first.size()) +
                   " IDs de producción y " + std::to_string(parse_result.second.size()) + " acciones");
        }
        
        // Preparar producciones y acciones
        std::queue<std::shared_ptr<AttrProd>> productions;
        std::vector<std::string> actions;
        
        // Obtener producciones y acciones del resultado del parser
        if (verbose) {
            LogInfo("Obteniendo producciones a partir de IDs");
        }
        
        int count = 0;
        for (const auto& id : parse_result.first) {
            try {
                if (verbose && count < 5) { // Solo mostrar las primeras 5 producciones en log para evitar sobrecarga
                    LogDebug("  Buscando producción con ID: " + std::to_string(id));
                }
                const AttrProd& prod = hulk_grammar.GetProduction(id);
                if (verbose && count < 5) {
                    LogDebug("  Producción encontrada: " + prod.ToString());
                }
                productions.push(make_shared<AttrProd>(prod));
                count++;
            } catch (const std::runtime_error& e) {
                LogError("  Error al obtener producción con ID " + std::to_string(id) + ": " + e.what());
                throw;
            }
        }
        
        if (verbose) {
            LogInfo("Se obtuvieron " + std::to_string(count) + " producciones correctamente");
            LogInfo("Procesando " + std::to_string(parse_result.second.size()) + " acciones");
        }
        
        for (const auto& action : parse_result.second) {
            actions.push_back(action);
        }
        
        // Construir el AST
        LogInfo("Construyendo AST a partir de producciones y acciones");
        auto ast = build_ast(productions, actions, tokens, hulk_grammar);
        if (!ast) {
            error_message = "No se pudo construir el AST";
            LogError(error_message);
            return nullptr;
        }
        
        LogInfo("AST construido exitosamente");
        return ast;
    } catch (const ParsingError& e) {
        error_message = std::string("Error sintáctico: ") + e.what();
        LogError(error_message);
        return nullptr;
    } catch (const std::exception& e) {
        error_message = std::string("Error durante el análisis: ") + e.what();
        LogError(error_message);
        return nullptr;
    }
}

ASTNode* compile_hulk(const std::string& source_code, std::string& error_message, Grammar& hulk_grammar, bool verbose) {
    // Iniciar el proceso de compilación
    LogInfo("Iniciando proceso de compilación HULK");
    LogInfo("Longitud del código fuente: " + std::to_string(source_code.size()) + " bytes");

    // Cargar artefactos del compilador
    LogInfo("Cargando artefactos del compilador (lexer y parser)");
    std::string artifact_error;
    auto [lexer, parser] = load_compiler_artifacts(artifact_error, hulk_grammar, verbose);
    if (!lexer || !parser) {
        error_message = artifact_error;
        LogError("Error al cargar artefactos: " + error_message);
        return nullptr;
    }
    LogInfo("Artefactos del compilador cargados exitosamente");
    
    // Tokenizar el código fuente
    LogInfo("Iniciando tokenización del código fuente");
    std::string tokenize_error;
    auto tokens = tokenize_source(source_code, lexer, hulk_grammar, tokenize_error);
    if (tokens.empty()) {
        error_message = tokenize_error;
        LogError("Error en la tokenización: " + error_message);
        delete lexer;
        delete parser;
        return nullptr;
    }
    
    // Construir el AST
    LogInfo("Iniciando construcción del AST");
    std::string ast_error;
    ASTNode* ast = build_ast_from_tokens(tokens, parser, hulk_grammar, ast_error, verbose);
    if (!ast) {
        error_message = ast_error;
        LogError("Error en la construcción del AST: " + error_message);
        delete lexer;
        delete parser;
        return nullptr;
    }
    
    // Liberar memoria de lexer y parser una vez que hemos terminado con ellos
    LogDebug("Liberando recursos (lexer y parser)");
    delete lexer;
    delete parser;
    
    LogInfo("Compilación completada exitosamente");
    
    return ast;
}

ASTNode* compile_hulk_file(const std::string& file_path, std::string& error_message, Grammar& hulk_grammar, bool verbose) {
    // Leer contenido del archivo
    std::string source_code = read_source_file(file_path, error_message);
    if (source_code.empty()) {
        return nullptr; // Error ya está en error_message
    }
    
    // if (verbose) {
    //     std::cout << "Archivo '" << file_path << "' cargado exitosamente." << std::endl;
    // }
    
    // Compilar el código fuente
    return compile_hulk(source_code, error_message, hulk_grammar, verbose);
}

// Sobrecarga eliminada: usamos parámetros por defecto en lugar de sobrecargas

std::string read_source_file(const std::string& file_path, std::string& error_message) {
    LogInfo("Intentando leer archivo fuente: " + file_path);
    // Intentar abrir el archivo
    std::ifstream source_file(file_path);
    if (!source_file.is_open()) {
        error_message = "No se pudo abrir el archivo: " + file_path;
        LogError(error_message);
        return "";
    }

    try {
        // Leer todo el contenido del archivo
        LogDebug("Archivo abierto, leyendo contenido");
        std::string source_content((std::istreambuf_iterator<char>(source_file)), 
                                   std::istreambuf_iterator<char>());
        source_file.close();
        LogInfo("Archivo leído exitosamente: " + file_path + " (" + 
                std::to_string(source_content.size()) + " bytes)");
        return source_content;
    }
    catch(const std::exception& e) {
        error_message = std::string("Error al leer el archivo: ") + e.what();
        LogError(error_message);
        if (source_file.is_open()) {
            source_file.close();
        }
        return "";
    }
}

bool validate_grammar_compatibility(const Grammar& g1, const Grammar& g2, bool verbose) {
    bool compatible = true;
    
    // Check number of productions
    if (g1.Productions().size() != g2.Productions().size()) {
        // if (verbose) {
        //     std::cout << "Warning: Grammars have different number of productions: " 
        //               << g1.Productions().size() << " vs " << g2.Productions().size() << std::endl;
        // }
        compatible = false;
    }
    
    // Check productions by ID
    for (const auto& prod1 : g1.Productions()) {
        try {
            const AttrProd& prod2 = g2.GetProduction(prod1.get_id());
            // Check if productions are equivalent
            if (prod1.ToString() != prod2.ToString()) {
                // if (verbose) {
                //     std::cout << "Warning: Production with ID " << prod1.get_id() 
                //               << " differs between grammars:" << std::endl;
                //     std::cout << "  G1: " << prod1.ToString() << std::endl;
                //     std::cout << "  G2: " << prod2.ToString() << std::endl;
                // }
                compatible = false;
            }
        } catch (const std::runtime_error& e) {
            // if (verbose) {
            //     std::cout << "Warning: Production with ID " << prod1.get_id() 
            //               << " not found in second grammar: " << e.what() << std::endl;
            // }
            compatible = false;
        }
    }
    
    return compatible;
}



// int main(int argc, char* argv[]) {
//     std::cout << "=== Ejecutando el compilador Hulk ===" << std::endl;
    
//     // Verificar si se solicita recrear los artefactos
//     bool recreate_artifacts = false;
//     std::string file_path = "script.hulk";
    
//     // for (int i = 1; i < argc; i++) {
//     //     std::string arg = argv[i];
//     //     if (arg == "--recreate" || arg == "-r") {
//     //         recreate_artifacts = true;
//     //     } else {
//     //         file_path = arg;
//     //     }
//     // }

//     // Verificar si estan los artefactos del compilador
//     if (!std::filesystem::exists("hulk/lexer.l") || !std::filesystem::exists("hulk/parser.p")) {
//         recreate_artifacts = true; // Forzar recreación si no existen los artefactos
//     }
    
//     if (recreate_artifacts) {
//         std::cout << "Recreando artefactos del compilador..." << std::endl;
//         std::string error_msg;
//         Grammar hulk_grammar = getHulkGrammar();
//         bool success = create_artifacts(hulk_grammar, true);
//         if (!success) {
//             std::cerr << "Error al recrear los artefactos" << std::endl;
//             return 1;
//         }
//         std::cout << "Artefactos recreados exitosamente" << std::endl;
//     }
    
//     // Cargar archivo de código fuente
//     std::string error_message;
//     std::string script_content = read_source_file(file_path, error_message);
//     if (script_content.empty()) {
//         std::cerr << "Error: " << error_message << std::endl;
//         return 1;
//     }
//     std::cout << "Contenido de '" << file_path << "' cargado exitosamente." << std::endl;
    
//     // Compilar el código utilizando nuestra función principal con modo verbose
//     // Nota: Utilizamos una única gramática para todo el proceso de compilación
//     Grammar hulk_grammar = getHulkGrammar();
//     ASTNode* ast = compile_hulk(script_content, error_message, hulk_grammar, false);
    
//     if (!ast) {
//         std::cerr << "Error al compilar: " << error_message << std::endl;
//         return 1;
//     }
    
//     // El AST se ha construido correctamente, podemos imprimirlo o procesarlo
//     std::cout << "\nImprimiendo el AST construido:" << std::endl;
//     ast->print();
    
//     // Aquí podrías agregar más procesamiento del AST, como:
//     // - Análisis semántico
//     // - Optimización
//     // - Generación de código
    
//     // Liberar la memoria del AST cuando hayamos terminado con él
//     // delete ast;  // Descomenta cuando termines de usarlo
    
//     return 0;
// }
