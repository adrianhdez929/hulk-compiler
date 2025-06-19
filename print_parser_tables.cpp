// #include "Grammar/grammar.h"
// #include "Parser/LR1Parser.h"
// #include "hulkGrammar.hpp"
// #include "Lexer/grammar_parser.h"
// #include <iostream>
// #include <iomanip>
// #include <map>
// #include <string>
// #include <set>
// #include <fstream>
// #include <chrono>
// #include <ctime>

// // Función para imprimir los conjuntos FIRST en formato legible
// void printFirstSets(std::ostream& out, const std::map<Sentence, ContainerSet<std::string>>& firsts) {
//     out << "==================================" << std::endl;
//     out << "         CONJUNTOS FIRST          " << std::endl;
//     out << "==================================" << std::endl;
    
//     for (const auto& [key, value] : firsts) {
//         out << "FIRST(" << key.ToString() << ") = { ";
//         for (const auto& term : value.get_values()) {
//             out << term << " ";
//         }
//         if (value.contains_epsilon()) {
//             out << "ε ";
//         }
//         out << "}" << std::endl;
//     }
//     out << "----------------------------------------------------------" << std::endl;
// }

// // Función para imprimir los conjuntos FOLLOW en formato legible
// void printFollowSets(std::ostream& out, const std::map<Sentence, ContainerSet<std::string>>& follows) {
//     out << "==================================" << std::endl;
//     out << "         CONJUNTOS FOLLOW         " << std::endl;
//     out << "==================================" << std::endl;
    
//     for (const auto& [key, value] : follows) {
//         out << "FOLLOW(" << key.ToString() << ") = { ";
//         for (const auto& term : value.get_values()) {
//             out << term << " ";
//         }
//         if (value.contains_epsilon()) {
//             out << "ε ";
//         }
//         out << "}" << std::endl;
//     }
//     out << "----------------------------------------------------------" << std::endl;
// }

// // Clase modificada para análisis de parsers LR1
// class ParserAnalyzer {
// public:
//     ParserAnalyzer(Grammar& g, const std::string& outputFileName) : 
//         grammar(g)
//     {
//         // Abrir el archivo de salida
//         outputFile.open(outputFileName);
        
//         if (!outputFile.is_open()) {
//             std::cerr << "Error: No se pudo abrir el archivo " << outputFileName << " para escritura." << std::endl;
//             return;
//         }
//         parser = new LR1Parser(grammar, true);
        
//         // Explícitamente construimos la tabla para asegurar que esté inicializada
//         parser->BuildParsingTable();
        
//         // Calcular conjuntos FIRST y FOLLOW
//         first_sets = parser->compute_firsts();
//         follow_sets = parser->compute_follows(first_sets);
//     }
    
//     ~ParserAnalyzer() {
//         if (parser) {
//             delete parser;
//             parser = nullptr;
//         }
//         if (outputFile.is_open()) {
//             outputFile.close();
//         }
//     }
    
//     bool isOutputFileOpen() const {
//         return outputFile.is_open();
//     }
    
//     void printTables() {
//         // Obtener fecha y hora actual
//         auto now = std::chrono::system_clock::now();
//         std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        
//         outputFile << "==================================" << std::endl;
//         outputFile << "       ANÁLISIS DE PARSER LR1      " << std::endl;
//         outputFile << "==================================" << std::endl;
//         outputFile << "Generado el: " << std::ctime(&now_time);
        
//         // Imprimir información de la gramática
//         printGrammarInfo();
        
//         // Imprimir conjuntos FIRST calculados
//         printFirstSets(outputFile, first_sets);
        
//         // Imprimir conjuntos FOLLOW calculados
//         printFollowSets(outputFile, follow_sets);
//     }

// private:
//     Grammar& grammar;
//     LR1Parser* parser;
//     std::map<Sentence, ContainerSet<std::string>> first_sets;
//     std::map<Sentence, ContainerSet<std::string>> follow_sets;
//     std::ofstream outputFile;
    
//     void printGrammarInfo() {
//         outputFile << "\n==================================" << std::endl;
//         outputFile << "       INFORMACIÓN GRAMÁTICA      " << std::endl;
//         outputFile << "==================================" << std::endl;
        
//         // Información básica de la gramática
//         outputFile << "Símbolo inicial: " << grammar.GetStartSymbol()->Name() << std::endl;
//         outputFile << "Número de producciones: " << grammar.Productions().size() << std::endl;
//         outputFile << "Número de terminales: " << grammar.Terminals().size() << std::endl;
//         outputFile << "Número de no terminales: " << grammar.NonTerminals().size() << std::endl;
        
//         // Listar terminales
//         outputFile << "\nTerminales: ";
//         for (const auto& terminal : grammar.Terminals()) {
//             outputFile << terminal->Name() << " ";
//         }
//         outputFile << std::endl;
        
//         // Listar no terminales
//         outputFile << "No terminales: ";
//         for (const auto& nonTerminal : grammar.NonTerminals()) {
//             outputFile << nonTerminal->Name() << " ";
//         }
//         outputFile << std::endl;
        
//         // Listar producciones con sus IDs
//         outputFile << "\nProducciones:" << std::endl;
//         for (const auto& production : grammar.Productions()) {
//             outputFile << "[" << production.get_id() << "] " << production.ToString() << std::endl;
//         }
//     }
// };

// int main() {
//     // Nombre del archivo de salida
//     const std::string outputFileName = "parser_tables.txt";
    
//     std::cout << "Analizando gramáticas y parsers LR1..." << std::endl;
//     std::cout << "Este proceso puede tomar un tiempo..." << std::endl;
//     std::cout << "Los resultados se guardarán en el archivo: " << outputFileName << std::endl;
    
//     // Obtener la gramática de HULK desde hulkGrammar.hpp
//     Grammar hulk_grammar = getHulkGrammar();
    
//     // Crear un analizador para la gramática de HULK
//     ParserAnalyzer hulkAnalyzer(hulk_grammar, outputFileName);
    
//     if (!hulkAnalyzer.isOutputFileOpen()) {
//         std::cerr << "Error: No se pudo abrir el archivo de salida " << outputFileName << std::endl;
//         return 1;
//     }
    
//     // Imprimir la información del parser de HULK
//     hulkAnalyzer.printTables();
    
//     std::cout << "Análisis de gramática HULK completado." << std::endl;
    
//     // También podemos intentar analizar la gramática del lexer si está disponible
//     try {
//         std::cout << "Intentando analizar gramática del lexer..." << std::endl;
        
//         // Crear un nuevo archivo para la gramática del lexer
//         const std::string lexerOutputFileName = "lexer_parser_tables.txt";
        
//         // Obtener la gramática del lexer si está disponible
//         Grammar lexer_grammar = GrammarParser::Parse("Lexer/grammar.txt");
        
//         // Crear un analizador para la gramática del lexer
//         ParserAnalyzer lexerAnalyzer(lexer_grammar, lexerOutputFileName);
        
//         if (lexerAnalyzer.isOutputFileOpen()) {
//             // Imprimir la información del parser del lexer
//             lexerAnalyzer.printTables();
//             std::cout << "Análisis de gramática del lexer completado. Guardado en: " << lexerOutputFileName << std::endl;
//         } else {
//             std::cerr << "Error: No se pudo abrir el archivo de salida para el lexer." << std::endl;
//         }
//     } catch (const std::exception& e) {
//         std::cerr << "No se pudo analizar la gramática del lexer: " << e.what() << std::endl;
//         std::cerr << "Continuando solo con la gramática HULK." << std::endl;
//     }
    
//     std::cout << "Proceso completado. Revise el archivo " << outputFileName << " para los detalles." << std::endl;
//     return 0;
// }
