#include "Grammar/grammar.h"
#include "Parser/LR1Parser.h"
#include "Lexer/Token.h"
#include "hulkGrammar.hpp"
#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <fstream>

/**
 * @brief Tokeniza un script Hulk utilizando el léxico de la gramática.
 * 
 * Esta implementación es un tokenizador simple para pruebas.
 * No es un analizador léxico completo, pero es suficiente para
 * pruebas básicas de la gramática.
 * 
 * @param script_file Ruta al archivo del script Hulk a tokenizar.
 * @return Vector de tokens.
 */
std::vector<std::string> tokenizeHulkScript(const std::string& script_file) {
    std::ifstream file(script_file);
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + script_file);
    }
    
    // Leer todo el contenido del archivo
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    
    std::vector<std::string> tokens;
    std::string current_token;
    
    // Mapa de tokens especiales de un carácter
    std::unordered_map<char, std::string> special_chars = {
        {'(', "("}, {')', ")"}, {'{', "{"}, {'}', "}"},
        {';', ";"}, {'+', "+"}, {'-', "-"}, {'*', "*"},
        {'/', "/"}, {'^', "^"}, {'%', "%"}, {':', ":"},
        {'=', "="}, {'<', "<"}, {'>', ">"}, {',', ","},
        {'[', "["}, {']', "]"}
    };
    
    // Palabras clave
    std::unordered_set<std::string> keywords = {
        "let", "in", "if", "else", "function", "while", "for",
        "return", "true", "false", "print", "string", "number", "bool"
    };
    
    int line_num = 1;
    
    for (size_t i = 0; i < content.size(); ++i) {
        char c = content[i];
        
        // Contar líneas
        if (c == '\n') {
            line_num++;
            continue;
        }
        
        // Ignorar espacios en blanco
        if (isspace(c)) {
            continue;
        }
        
        // Manejar comentarios de línea
        if (c == '/' && i + 1 < content.size() && content[i + 1] == '/') {
            while (i < content.size() && content[i] != '\n') {
                i++;
            }
            continue;
        }
        
        // Manejar comentarios multi-línea
        if (c == '/' && i + 1 < content.size() && content[i + 1] == '*') {
            i += 2; // Saltar '/*'
            while (i + 1 < content.size() && !(content[i] == '*' && content[i + 1] == '/')) {
                if (content[i] == '\n') {
                    line_num++;
                }
                i++;
            }
            i += 1; // Saltar '*/'
            continue;
        }
        
        // Caracteres especiales
        if (special_chars.find(c) != special_chars.end()) {
            // Manejar operadores de dos caracteres
            if ((c == '=' || c == '!' || c == '<' || c == '>') && i + 1 < content.size() && content[i + 1] == '=') {
                tokens.push_back(std::string(1, c) + "=");
                i++;
            } else if (c == '&' && i + 1 < content.size() && content[i + 1] == '&') {
                tokens.push_back("&&");
                i++;
            } else if (c == '|' && i + 1 < content.size() && content[i + 1] == '|') {
                tokens.push_back("||");
                i++;
            } else {
                tokens.push_back(special_chars[c]);
            }
            continue;
        }
        
        // Números
        if (isdigit(c)) {
            std::string number;
            while (i < content.size() && (isdigit(content[i]) || content[i] == '.')) {
                number += content[i++];
            }
            i--; // Retroceder uno ya que el bucle principal incrementará i
            tokens.push_back(number);
            continue;
        }
        
        // Identificadores y palabras clave
        if (isalpha(c) || c == '_') {
            std::string identifier;
            while (i < content.size() && (isalnum(content[i]) || content[i] == '_')) {
                identifier += content[i++];
            }
            i--; // Retroceder uno ya que el bucle principal incrementará i
            
            // Verificar si es una palabra clave
            if (keywords.find(identifier) != keywords.end()) {
                tokens.push_back(identifier);
            } else if (identifier == "true" || identifier == "false") {
                tokens.push_back(identifier); // Tratar true/false como valores booleanos
            } else {
                // Verificar si es un identificador de tipo (comienza con mayúscula)
                if (isupper(identifier[0])) {
                    tokens.push_back("type_id"); // Token para identificadores de tipo
                } else {
                    tokens.push_back("var_id"); // Token para identificadores de variable
                }
            }
            continue;
        }
        
        // Cadenas
        if (c == '"') {
            std::string str = "\"";
            i++;
            while (i < content.size() && content[i] != '"') {
                // Manejar caracteres de escape
                if (content[i] == '\\' && i + 1 < content.size()) {
                    str += content[i++];
                }
                str += content[i++];
            }
            str += '"';
            tokens.push_back(str);
            continue;
        }
        
        // Caracteres no reconocidos
        std::cerr << "Caracter no reconocido en línea " << line_num << ": " << c << std::endl;
    }
    
    // Imprimir los tokens para depuración
    std::cout << "Tokens encontrados:" << std::endl;
    for (const auto& token : tokens) {
        std::cout << "  " << token << std::endl;
    }
    
    // Añadir el marcador de fin de entrada
    tokens.push_back("$");
    
    return tokens;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Uso: " << argv[0] << " <script.hulk>" << std::endl;
        return 1;
    }
    
    // Nombre del archivo script
    std::string script_file = argv[1];
    
    try {
        // Crear la gramática Hulk
        Grammar grammar = getHulkGrammar();
        // DefineHulkGrammar(grammar);
        
        // Imprimir información sobre la gramática
        std::cout << "Gramática Hulk cargada." << std::endl;
        std::cout << "Número de producciones: " << grammar.Productions().size() << std::endl;
        std::cout << "Símbolo inicial: " << grammar.GetStartSymbol()->Name() << std::endl;
        
        // Crear el parser LR1
        std::cout << "\nCreando parser LR(1)..." << std::endl;
        LR1Parser parser(grammar, true);
        
        // Tokenizar el script
        std::cout << "\nTokenizando el script..." << std::endl;
        auto tokens = tokenizeHulkScript(script_file);
        
        // Parsear el script
        std::cout << "\nAnalizando el script..." << std::endl;
        auto result = parser.Parse(tokens);
        
        // Mostrar resultado
        std::cout << "\nAnálisis exitoso!" << std::endl;
        std::cout << "Producciones utilizadas: " << result.first.size() << std::endl;
        
        // Mostrar las primeras 10 producciones utilizadas (o todas si hay menos)
        int max_to_show = std::min(10, static_cast<int>(result.first.size()));
        std::cout << "\nPrimeras " << max_to_show << " producciones:" << std::endl;
        for (int i = 0; i < max_to_show; ++i) {
            std::cout << "  " << i+1 << ". Producción " << result.first[i] 
                      << " (" << result.second[i] << ")" << std::endl;
        }
        
        // Obtener y mostrar el AST resultante con validación de punteros
        const auto& ast_nodes = parser.GetAST();
        std::cout << "\nÁrbol de Sintaxis Abstracta (AST):" << std::endl;
        if (!ast_nodes.empty() && ast_nodes.front() != nullptr) {
            ASTNode* ast = ast_nodes.front();
            ast->print(0);
        } else {
            std::cout << "No se generó un AST válido." << std::endl;
        }
        
        // Para evitar errores de segmentación, imprimir información sobre el AST
        std::cout << "\nInformación sobre el AST:\n";
        std::cout << "- Tamaño del vector AST: " << ast_nodes.size() << std::endl;
        if (!ast_nodes.empty()) {
            std::cout << "- Primer nodo del AST: " << (ast_nodes.front() ? "No nulo" : "Nulo") << std::endl;
            if (ast_nodes.front()) {
                std::cout << "- Tipo del primer nodo: " << typeid(*ast_nodes.front()).name() << std::endl;
            }
        }

    } catch (const ParsingError& e) {
        std::cerr << "Error de análisis en el estado " << e.getState() 
                  << " con el token '" << e.getToken() << "'" << std::endl;
        std::cerr << "Tokens esperados: ";
        for (const auto& expected : e.getExpectedTokens()) {
            std::cerr << expected << " ";
        }
        std::cerr << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
