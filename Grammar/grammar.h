#include <vector>
#include <unordered_map>
#include <map>
#include <memory>
#include <variant>
#include <typeindex>
#include <stdexcept>
#include "symbol.h"
#include "production.h"

#ifndef GRAMMAR_H
#define GRAMMAR_H

using namespace std;

/**
 * @class Epsilon
 * @brief Representa el símbolo épsilon (cadena vacía) en la gramática.
 */
class Epsilon : public Symbol {
    public:
        Epsilon(Grammar& grammar);
        bool IsEpsilon() const override;
};

/**
 * @class EndOfFile
 * @brief Representa el símbolo de fin de archivo (EOF) en la gramática.
 */
class EndOfFile : public Terminal {
    public:
        EndOfFile(Grammar& grammar);
        bool IsEndOfFile() const override;
};

/**
 * @class Grammar
 * @brief Representa una gramática formal, con sus símbolos, producciones y reglas.
 * 
 * Permite definir terminales, no terminales, producciones y obtener información sobre la gramática.
 */
class Grammar {
    public:
        Grammar();
        std::shared_ptr<NonTerminal> SetNonTerminal(const std::string& name, bool isStart = false);
        std::shared_ptr<Terminal> SetTerminal(const std::string& name);
        std::shared_ptr<Epsilon> SetEpsilon();
        std::shared_ptr<EndOfFile> SetEndOfFile();

        vector<std::shared_ptr<NonTerminal>> NonTerminals();
        vector<std::shared_ptr<Terminal>> Terminals();

        shared_ptr<Symbol> GetSymbol(const string& name);
        const AttrProd& GetProduction(const std::string& name) const {
            for (const auto& production : productions) {
                if (production.Left()->Name() == name) {
                    return production;
                }
            }
            throw std::runtime_error("Production not found: " + name);
        }
        
        const AttrProd& GetProduction(int id) const {
            for (const auto& production : productions) {
                if (production.get_id() == id) {
                    return production;
                }
            }
            throw std::runtime_error("Production not found with id: " + std::to_string(id));
        }
        
        const AttrProd& GetProduction(const Production& production) const {
            // Use ID if available, otherwise fallback to string comparison
            int prod_id = production.get_id();
            if (prod_id >= 0) {
                return GetProduction(prod_id);
            }
            
            // Fallback to string comparison, but check for null pointers first
            if (!production.Left()) {
                throw std::runtime_error("Production has null left side");
            }
            
            std::string prod_str = production.ToString();
            for (const auto& prod : productions) {
                if (prod.ToString() == prod_str) {
                    return prod;
                }
            }
            throw std::runtime_error("Production not found");
        }
        // void AddProduction(const AttributeProduction& production);
        // using ProductionVariant = std::variant<Production, AttrProd>;
        void AddProduction(const AttrProd& production);
        void AddProduction(const Production& production);
        static void ResetProductionCounter();  // Función para resetear el contador
        // const std::vector<ProductionVariant>& Productions() const;
        // const std::vector<Production>& Productions() const;
        const std::vector<AttrProd>& Productions() const;
        const std::shared_ptr<NonTerminal>& GetStartSymbol() const;
        const std::shared_ptr<Epsilon>& GetEpsilon() const;
        const std::shared_ptr<EndOfFile>& GetEndOfFile() const;
        const std::vector<std::shared_ptr<Symbol>>& Symbols() const;
        //Metodo para obtener todos los simbolos
        // const std::vector<const Symbol*>& Symbols() const;

        void Augment();
        bool IsAugmented() const;
        //Sobreescribir el .tostring
        std::string ToString() const;

        // ====== Asosiatividad y precedencia ======
        enum Associativity {
            LEFT,
            RIGHT,
            NONASSOC
        };

        void AddPrecedence(const std::string& symbol, int precedence, Associativity associativity) {
            if (precedences_.find(symbol) != precedences_.end()) {
                throw std::runtime_error("Symbol already has precedence defined: " + symbol);
            }
            precedences_[symbol] = {precedence, associativity};
        };

        void SetProductionPrecedence(int id, const std::string& symbol) {
            if (production_precedences_.find(id) != production_precedences_.end()) {
                throw std::runtime_error("Production ID already has precedence defined: " + std::to_string(id));
            }
            production_precedences_[id] = symbol;
        };

        std::pair<int, Associativity> GetPrecedence(const std::string& symbol) const {
            auto it = precedences_.find(symbol);
            if (it == precedences_.end()) {
                throw std::runtime_error("No precedence defined for symbol: " + symbol);
            }
            return it->second;
        };

        std::string GetProductionPrecedence(int id) const {
            auto it = production_precedences_.find(id);
            if (it == production_precedences_.end()) {
                throw std::runtime_error("No precedence defined for production ID: " + std::to_string(id));
            }
            return it->second;
        };




    private:
        std::vector<std::shared_ptr<Symbol>> symbols;
        std::vector<std::shared_ptr<NonTerminal>> nonTerminals;
        std::vector<std::shared_ptr<Terminal>> terminals;
        std::shared_ptr<NonTerminal> augmentedStartSymbol;
        // std::vector<ProductionVariant> productions;
        // std::vector<Production> productions;
        std::vector<AttrProd> productions;
        std::type_index productionType;
        std::shared_ptr<NonTerminal> startSymbol;
        std::shared_ptr<Epsilon> epsilon;
        std::shared_ptr<EndOfFile> eof;
        std::unordered_map<std::string, Symbol*> symbolMap;


        std::map<std::string, std::pair<int, Associativity>> precedences_;
        std::map<int, std::string> production_precedences_;
};

#endif