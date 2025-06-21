#include "LR1Parser.h"
#include <iostream>
#include <set>
#include <algorithm>
#include <stack>
#include <cassert>
#include <fstream>
#include <filesystem>

// Definición de la constante estática
const std::string LR1Parser::END_MARKER = "EOF";

LR1Parser::LR1Parser(Grammar& G, bool verbose)
    : SLR1Parser(G, verbose) {
    // Note: Constructor de la clase base SLR1Parser es llamado primero
    // Ahora sobrescribimos las tablas action_ y goto_ con nuestra implementación LR(1)
    if (verbose_) {
        std::cout << "Construyendo parser LR(1)..." << std::endl;
    }
    BuildParsingTable();
}

const std::vector<ASTNode*>& LR1Parser::GetAST() const {
    return ast_nodes_;
}

ContainerSet<std::string> LR1Parser::CalculateFirst(const std::shared_ptr<Symbol>& symbol) {
    // Comprobar si ya tenemos el resultado en caché
    auto it = first_cache_.find(symbol->Name());
    if (it != first_cache_.end()) {
        return it->second;
    }

    ContainerSet<std::string> result;
    
    // Si es terminal, el conjunto FIRST solo contiene ese terminal
    if (symbol->IsTerminal()) {
        result.add(symbol->Name());
        first_cache_[symbol->Name()] = result;
        return result;
    }

    // Para no terminales, calculamos recursivamente
    auto productions_for_symbol = G_.GetProductionsFrom(symbol->Name());
    for (const auto& production : productions_for_symbol) {
        const auto& right_symbols = production.Right().Symbols();
        
        // Si la producción deriva en ε, añadimos ε al conjunto FIRST
        if (right_symbols.empty()) {
            result.add(G_.GetEpsilon()->Name());
            continue;
        }

        // Calculamos el conjunto FIRST para la secuencia de la parte derecha
        bool all_derive_epsilon = true;
        for (size_t i = 0; i < right_symbols.size(); ++i) {
            const auto& sym = right_symbols[i];
            auto first_sym = CalculateFirst(sym);
            
            // Añadir todos los terminales excepto ε
            for (const auto& terminal : first_sym) {
                if (terminal != G_.GetEpsilon()->Name()) {
                    result.add(terminal);
                }
            }
            
            // Si este símbolo no puede derivar en ε, no seguimos
            if (!first_sym.contains(G_.GetEpsilon()->Name())) {
                all_derive_epsilon = false;
                break;
            }
        }

        // Si todos los símbolos de la parte derecha pueden derivar en ε, añadimos ε al resultado
        if (all_derive_epsilon) {
            result.add(G_.GetEpsilon()->Name());
        }
    }
    
    // Guardar en caché para futuras consultas
    first_cache_[symbol->Name()] = result;
    
    return result;
}

ContainerSet<std::string> LR1Parser::CalculateFirst(const std::vector<std::shared_ptr<Symbol>>& symbols) {
    ContainerSet<std::string> result;
    
    // Si la secuencia está vacía, solo contiene ε
    if (symbols.empty()) {
        result.add(G_.GetEpsilon()->Name());
        return result;
    }
    
    // Calculamos el conjunto FIRST para la secuencia
    bool all_derive_epsilon = true;
    for (size_t i = 0; i < symbols.size(); ++i) {
        const auto& sym = symbols[i];
        auto first_sym = CalculateFirst(sym);
        
        // Añadir todos los terminales excepto ε
        for (const auto& terminal : first_sym) {
            if (terminal != G_.GetEpsilon()->Name()) {
                result.add(terminal);
            }
        }
        
        // Si este símbolo no puede derivar en ε, no seguimos
        if (!first_sym.contains(G_.GetEpsilon()->Name())) {
            all_derive_epsilon = false;
            break;
        }
    }
    
    // Si todos los símbolos de la secuencia pueden derivar en ε, añadimos ε al resultado
    if (all_derive_epsilon) {
        result.add(G_.GetEpsilon()->Name());
    }
    
    return result;
}

LR1ItemSet LR1Parser::CalculateLR1Closure(const LR1ItemSet& items) {
    LR1ItemSet closure = items;
    bool changes;
    
    do {
        changes = false;
        std::vector<std::shared_ptr<LR1Item>> current_items = closure.items();
        
        for (const auto& item : current_items) {
            // Si no hay siguiente símbolo o no es no terminal, no hacemos nada
            auto next_symbol = item->NextSymbol();
            if (!next_symbol || next_symbol->IsTerminal()) {
                continue;
            }
            
            // Calcular el conjunto FIRST de lo que sigue al no terminal y el lookahead
            std::vector<std::shared_ptr<Symbol>> beta_a;
            const auto& right_symbols = item->production()->Right().Symbols();
            for (size_t i = item->pos() + 1; i < right_symbols.size(); ++i) {
                beta_a.push_back(right_symbols[i]);
            }
            // Añadir el lookahead al final (como si fuera un símbolo)
            auto lookahead_symbol = G_.GetSymbol(item->lookahead());
            if (!lookahead_symbol) {
                // Si el lookahead no está definido como símbolo, lo tratamos como terminal
                lookahead_symbol = std::make_shared<Terminal>(item->lookahead(), G_);
            }
            beta_a.push_back(lookahead_symbol);
            
            auto first_beta_a = CalculateFirst(beta_a);
            if (first_beta_a.contains(G_.GetEpsilon()->Name())) {
                // Si beta_a puede derivar en ε, añadimos el lookahead original
                first_beta_a.add(item->lookahead());
                first_beta_a.erase(G_.GetEpsilon()->Name());
            }
            
            // Añadir ítems para todas las producciones del no terminal
            for (const auto& production : G_.Productions()) {
                if (production.Left()->Name() != next_symbol->Name()) {
                    continue; // Solo consideramos producciones que empiezan con el símbolo dado
                }
                for (const auto& terminal : first_beta_a) {
                    auto new_item = std::make_shared<LR1Item>(production, 0, terminal);
                    if (closure.Add(new_item)) {
                        changes = true;
                    }
                }
            }
        }
    } while (changes);
    
    return closure;
}

LR1ItemSet LR1Parser::CalculateGoto(const LR1ItemSet& items, const std::shared_ptr<Symbol>& symbol) {
    LR1ItemSet goto_set;
    
    for (const auto& item : items) {
        auto next_symbol = item->NextSymbol();
        if (next_symbol && next_symbol->Name() == symbol->Name()) {
            auto next_item = item->NextItem();
            if (next_item) {
                goto_set.Add(next_item);
            }
        }
    }
    
    if (goto_set.size() > 0) {
        return CalculateLR1Closure(goto_set);
    }
    
    return goto_set;
}

std::vector<LR1ItemSet> LR1Parser::BuildLR1Automaton() {
    std::vector<LR1ItemSet> states;
    std::map<std::pair<int, std::string>, int> goto_indices;
    
    // Obtener la producción inicial
    auto start_symbol = G_.GetStartSymbol();
    G_.Augment(); // Asegurarse de que la gramática está aumentada
    assert(G_.IsAugmented() && "La gramática debe estar aumentada antes de construir el autómata LR(1)");
    auto augmented_start_symbol = G_.GetStartSymbol(); // Usamos el método que acabamos de añadir
    
    // Obtener la primera producción del símbolo de inicio aumentado
    auto augmented_production = G_.GetProductionsFrom(augmented_start_symbol->Name());

    if (augmented_production.empty()) {
        throw std::runtime_error("No se encontró la producción inicial para el símbolo aumentado");
    }

    auto start_production = augmented_production[0];

    // Crear el ítem inicial con lookahead "$" (fin de entrada)
    auto initial_item = std::make_shared<LR1Item>(start_production, 0, END_MARKER);
    LR1ItemSet initial_state({initial_item});
    
    // Calcular la clausura del estado inicial
    initial_state = CalculateLR1Closure(initial_state);
    states.push_back(initial_state);
    
    // Calcular los estados del autómata
    std::set<std::string> all_symbols;
    for (const auto& symbol : G_.Symbols()) {
        if (symbol->Name() != G_.GetEpsilon()->Name()) {
            all_symbols.insert(symbol->Name());
        }
    }
    
    for (size_t i = 0; i < states.size(); ++i) {
        const auto& state = states[i];
        
        for (const auto& symbol_name : all_symbols) {
            auto symbol = G_.GetSymbol(symbol_name);
            auto goto_set = CalculateGoto(state, symbol);
            
            if (goto_set.size() > 0) {
                // Comprobar si este estado ya existe
                auto it = std::find(states.begin(), states.end(), goto_set);
                int goto_index;
                
                if (it == states.end()) {
                    // Nuevo estado
                    states.push_back(goto_set);
                    goto_index = states.size() - 1;
                } else {
                    // Estado existente
                    goto_index = std::distance(states.begin(), it);
                }
                
                // Registrar la transición
                goto_indices[{i, symbol_name}] = goto_index;
            }
        }
    }
    
    // Guardar las transiciones para construir la tabla de análisis
    transitions_ = goto_indices;
    
    return states;
}

void LR1Parser::PrintLR1Automaton(const std::vector<LR1ItemSet>& automaton) {
    std::cout << "=== Autómata LR(1) ===\n";
    for (size_t i = 0; i < automaton.size(); ++i) {
        std::cout << "Estado " << i << ":\n" << automaton[i].ToString() << "\n\n";
    }
    std::cout << "======================" << std::endl;
}

void LR1Parser::BuildParsingTable() {
    // Limpiar las tablas action y goto
    action_.clear();
    goto_.clear();
    
    // Construir el autómata LR(1)
    auto automaton = BuildLR1Automaton();
    
    if (verbose_) {
        PrintLR1Automaton(automaton);
        std::cout << "Construyendo tablas de análisis LR(1)..." << std::endl;
    }
    
    // Construir las tablas action y goto
    for (size_t i = 0; i < automaton.size(); ++i) {
        const auto& state = automaton[i];
        
        // Procesar todos los ítems del estado
        for (const auto& item : state) {
            auto next_symbol = item->NextSymbol();
            
            if (next_symbol) {
                // Shift o Goto
                if (next_symbol->IsTerminal()) {
                    // Acción Shift
                    auto it = transitions_.find({i, next_symbol->Name()});
                    if (it != transitions_.end()) {
                        int next_state = it->second;
                        
                        // Verificar conflictos Shift/Reduce o Shift/Shift
                        auto action_key = std::make_pair(i, *next_symbol);
                        auto action_it = action_.find(action_key);
                        
                        if (action_it != action_.end() && action_it->second.first != SHIFT) {
                            if (verbose_) {
                                std::cout << "Conflicto en el estado " << i 
                                          << " con símbolo " << next_symbol->Name() 
                                          << ": " << action_it->second.first 
                                          << " y Shift" << std::endl;
                            }
                            // Aquí podrías implementar una estrategia para resolver conflictos
                            // Por ahora, priorizamos Shift sobre Reduce (común en muchos parsers)
                            action_[action_key] = std::make_pair(SHIFT, next_state);
                        } else if (action_it == action_.end()) {
                            action_[action_key] = std::make_pair(SHIFT, next_state);
                        }
                    }
                } else {
                    // Transición Goto para no terminales
                    auto it = transitions_.find({i, next_symbol->Name()});
                    if (it != transitions_.end()) {
                        int next_state = it->second;
                        goto_[std::make_pair(i, *next_symbol)] = next_state;
                    }
                }
            } else if (item->IsReduceItem()) {
                // Acción Reduce
                auto production = item->production();
                
                // Para el caso especial de la producción aumentada: S' -> S
                if (G_.IsAugmented() && production->Left()->Name() == G_.GetStartSymbol()->Name()) {
                    // Acción de aceptación, solo con el lookahead "$"
                    if (item->lookahead() == END_MARKER) {
                        auto accept_symbol = G_.GetSymbol(END_MARKER);
                        if (!accept_symbol) {
                            accept_symbol = std::make_shared<Terminal>(END_MARKER, G_);
                        }
                        action_[std::make_pair(i, *accept_symbol)] = std::make_pair(OK, 0);
                    }
                } else {
                    // Acción de reducción normal
                    auto lookahead_symbol = G_.GetSymbol(item->lookahead());
                    if (!lookahead_symbol) {
                        lookahead_symbol = std::make_shared<Terminal>(item->lookahead(), G_);
                    }
                    
                    auto action_key = std::make_pair(i, *lookahead_symbol);
                    auto prod_id = production->get_id();
                    
                    // Verificar conflictos Reduce/Reduce
                    auto action_it = action_.find(action_key);
                    if (action_it != action_.end()) {
                        if (action_it->second.first == REDUCE && action_it->second.second != prod_id) {
                            if (verbose_) {
                                std::cout << "Conflicto Reduce/Reduce en el estado " << i 
                                          << " con lookahead " << item->lookahead() 
                                          << " entre producciones " << action_it->second.second 
                                          << " y " << prod_id << std::endl;
                            }
                            
                            // Resolver conflicto Reduce/Reduce (por ejemplo, favoreciendo la producción con ID menor)
                            if (prod_id < action_it->second.second) {
                                action_[action_key] = std::make_pair(REDUCE, prod_id);
                            }
                        } else if (action_it->second.first == SHIFT) {
                            if (verbose_) {
                                std::cout << "Conflicto Shift/Reduce en el estado " << i 
                                          << " con símbolo " << item->lookahead() 
                                          << ": Shift y Reduce " << prod_id << std::endl;
                            }
                            
                            // Ya hemos decidido priorizar Shift sobre Reduce, así que no hacemos nada
                        }
                    } else {
                        action_[action_key] = std::make_pair(REDUCE, prod_id);
                    }
                }
            }
        }
    }
    
    if (verbose_) {
        std::cout << "Tablas de análisis LR(1) construidas correctamente." << std::endl;
        
        // Imprimir estadísticas
        std::cout << "Número de estados: " << automaton.size() << std::endl;
        std::cout << "Número de entradas en la tabla ACTION: " << action_.size() << std::endl;
        std::cout << "Número de entradas en la tabla GOTO: " << goto_.size() << std::endl;
    }
}

// Implementamos la función helper para obtener la ruta personalizada
std::string get_custom_path(const std::string& filename, const std::string& directory) {
    if (directory.empty()) {
        return filename;
    }
    return directory + "/" + filename;
}

bool LR1Parser::serialize_parser(const std::string& filename, const std::string& directory) {
    std::string filepath = get_custom_path(filename, directory);
    
    // Crear directorio si no existe
    if (!directory.empty()) {
        std::filesystem::create_directories(directory);
    }
    
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo para escritura: " << filepath << std::endl;
        return false;
    }
    
    try {
        // Escribir firma para identificación
        const std::string signature = "LR1PARSER1";
        file.write(signature.c_str(), 10);
        
        // Escribir versión
        uint32_t version = 1;
        file.write(reinterpret_cast<const char*>(&version), sizeof(version));
        
        // Escribir flag verbose_
        file.write(reinterpret_cast<const char*>(&verbose_), sizeof(verbose_));
        
        // Serializar tabla action_
        size_t action_size = action_.size();
        file.write(reinterpret_cast<const char*>(&action_size), sizeof(action_size));
        
        for (const auto& [key, value] : action_) {
            // Escribir key
            int state_id = key.first;
            file.write(reinterpret_cast<const char*>(&state_id), sizeof(state_id));
            
            // Escribir Symbol
            const std::string& symbol_name = key.second.Name();
            size_t name_length = symbol_name.size();
            file.write(reinterpret_cast<const char*>(&name_length), sizeof(name_length));
            file.write(symbol_name.c_str(), name_length);
            
            bool is_terminal = key.second.IsTerminal();
            file.write(reinterpret_cast<const char*>(&is_terminal), sizeof(is_terminal));
            
            // Escribir value
            const std::string& action_type = value.first;
            size_t action_length = action_type.size();
            file.write(reinterpret_cast<const char*>(&action_length), sizeof(action_length));
            file.write(action_type.c_str(), action_length);
            
            int action_value = value.second;
            file.write(reinterpret_cast<const char*>(&action_value), sizeof(action_value));
        }
        
        // Serializar tabla goto_
        size_t goto_size = goto_.size();
        file.write(reinterpret_cast<const char*>(&goto_size), sizeof(goto_size));
        
        for (const auto& [key, value] : goto_) {
            // Escribir key
            int state_id = key.first;
            file.write(reinterpret_cast<const char*>(&state_id), sizeof(state_id));
            
            // Escribir Symbol
            const std::string& symbol_name = key.second.Name();
            size_t name_length = symbol_name.size();
            file.write(reinterpret_cast<const char*>(&name_length), sizeof(name_length));
            file.write(symbol_name.c_str(), name_length);
            
            bool is_terminal = key.second.IsTerminal();
            file.write(reinterpret_cast<const char*>(&is_terminal), sizeof(is_terminal));
            
            // Escribir value
            int goto_value = value;
            file.write(reinterpret_cast<const char*>(&goto_value), sizeof(goto_value));
        }
        
        file.close();
        std::cout << "Parser serializado exitosamente en: " << filepath << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error durante la serialización del parser: " << e.what() << std::endl;
        file.close();
        return false;
    }
}

LR1Parser* LR1Parser::deserialize_parser(const std::string& filename, Grammar& grammar) {
    return deserialize_parser(filename, "hulk", grammar);
}

LR1Parser* LR1Parser::deserialize_parser(const std::string& filename, const std::string& directory, Grammar& grammar) {
    std::string filepath = get_custom_path(filename, directory);
    std::ifstream file(filepath, std::ios::binary);
    
    if (!file.is_open()) {
        std::cerr << "Error: No se pudo abrir el archivo para lectura: " << filepath << std::endl;
        return nullptr;
    }
    
    try {
        // Verificar firma
        char signature[11] = {0};
        file.read(signature, 10);
        if (std::string(signature) != "LR1PARSER1") {
            std::cerr << "Error: Archivo no es un parser LR1 serializado válido" << std::endl;
            file.close();
            return nullptr;
        }
        
        // Leer versión
        uint32_t version;
        file.read(reinterpret_cast<char*>(&version), sizeof(version));
        if (version != 1) {
            std::cerr << "Error: Versión de parser no soportada: " << version << std::endl;
            file.close();
            return nullptr;
        }
        
        // Leer flag verbose_
        bool verbose;
        file.read(reinterpret_cast<char*>(&verbose), sizeof(verbose));
        
        // Deserializar tabla action_
        std::map<std::pair<int, Symbol>, std::pair<std::string, int>> action;
        size_t action_size;
        file.read(reinterpret_cast<char*>(&action_size), sizeof(action_size));
        
        for (size_t i = 0; i < action_size; ++i) {
            // Leer key
            int state_id;
            file.read(reinterpret_cast<char*>(&state_id), sizeof(state_id));
            
            // Leer Symbol
            size_t name_length;
            file.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));
            std::string symbol_name(name_length, '\0');
            file.read(&symbol_name[0], name_length);
            
            bool is_terminal;
            file.read(reinterpret_cast<char*>(&is_terminal), sizeof(is_terminal));
            
            // Buscar el símbolo en la gramática
            auto symbol_ptr = grammar.GetSymbol(symbol_name);
            if (!symbol_ptr) {
                std::cerr << "Error: Símbolo no encontrado en gramática: " << symbol_name << std::endl;
                file.close();
                return nullptr;
            }
            
            // Leer value
            size_t action_length;
            file.read(reinterpret_cast<char*>(&action_length), sizeof(action_length));
            std::string action_type(action_length, '\0');
            file.read(&action_type[0], action_length);
            
            int action_value;
            file.read(reinterpret_cast<char*>(&action_value), sizeof(action_value));
            
            action[{state_id, *symbol_ptr}] = {action_type, action_value};
        }
        
        // Deserializar tabla goto_
        std::map<std::pair<int, Symbol>, int> goto_table;
        size_t goto_size;
        file.read(reinterpret_cast<char*>(&goto_size), sizeof(goto_size));
        
        for (size_t i = 0; i < goto_size; ++i) {
            // Leer key
            int state_id;
            file.read(reinterpret_cast<char*>(&state_id), sizeof(state_id));
            
            // Leer Symbol
            size_t name_length;
            file.read(reinterpret_cast<char*>(&name_length), sizeof(name_length));
            std::string symbol_name(name_length, '\0');
            file.read(&symbol_name[0], name_length);
            
            bool is_terminal;
            file.read(reinterpret_cast<char*>(&is_terminal), sizeof(is_terminal));
            
            // Buscar el símbolo en la gramática
            auto symbol_ptr = grammar.GetSymbol(symbol_name);
            if (!symbol_ptr) {
                std::cerr << "Error: Símbolo no encontrado en gramática: " << symbol_name << std::endl;
                file.close();
                return nullptr;
            }
            
            // Leer value
            int goto_value;
            file.read(reinterpret_cast<char*>(&goto_value), sizeof(goto_value));
            
            goto_table[{state_id, *symbol_ptr}] = goto_value;
        }
        
        file.close();
        std::cout << "Parser LR1 deserializado exitosamente desde: " << filepath << std::endl;
        
        // Crear nuevo parser con las tablas deserializadas
        LR1Parser* parser = new LR1Parser(grammar, verbose);
        parser->action_ = action;
        parser->goto_ = goto_table;
        
        return parser;
        
    } catch (const std::exception& e) {
        std::cerr << "Error durante la deserialización del parser: " << e.what() << std::endl;
        file.close();
        return nullptr;
    }
}

// Implementamos el método Parse para Tokens
std::pair<std::vector<int>, std::vector<std::string>>
LR1Parser::Parse(const std::vector<Token>& tokens) {
    // Convertir los tokens a strings para usar el parser base
    std::vector<std::string> token_strings;
    token_strings.reserve(tokens.size());
    
    for (const auto& token : tokens) {
        token_strings.push_back(token.Lexeme());
    }
    
    // Llamar al método Parse que acepta strings
    return Parse(token_strings);
}

std::pair<std::vector<int>, std::vector<std::string>> LR1Parser::Parse(const std::vector<std::string>& tokens) {
    // Limpiar el AST anterior si existe
    for (auto* node : ast_nodes_) {
        delete node;
    }
    ast_nodes_.clear();
    
    // Debug info
    std::cout << "LR1Parser::Parse - Comenzando análisis con " << tokens.size() << " tokens" << std::endl;
    
    try {
        // Llamar al método Parse de la clase base
        auto result = SLR1Parser::Parse(tokens);
        
        // Debug info
        std::cout << "LR1Parser::Parse - Análisis sintáctico completado exitosamente" << std::endl;
        std::cout << "LR1Parser::Parse - Producciones utilizadas: " << result.first.size() << std::endl;
        
        // Crear un nodo de programa dummy para el AST si no se creó durante el parsing
        if (ast_nodes_.empty()) {
            std::cout << "LR1Parser::Parse - Creando AST vacío" << std::endl;
            ast_nodes_.push_back(new ProgramNode(new ASTNodeVector({}, 1), 1));
        }
        
        return result;
    }
    catch (const std::exception& e) {
        std::cout << "LR1Parser::Parse - Excepción capturada: " << e.what() << std::endl;
        throw;
    }
    catch (...) {
        std::cout << "LR1Parser::Parse - Excepción desconocida capturada" << std::endl;
        throw;
    }
}
