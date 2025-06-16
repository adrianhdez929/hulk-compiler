#include <string>
#include <vector>
#include <memory>

#ifndef SYMBOL_H
#define SYMBOL_H

/**
 * @class Symbol
 * @brief Representa un símbolo de una gramática (terminal o no terminal).
 * 
 * Clase base para todos los símbolos usados en la definición de gramáticas.
 */

class Grammar;
class Sentence;

class Symbol : public std::enable_shared_from_this<Symbol> {
    public:
        // enum Type { TERMINAL, NON_TERMINAL };
        Symbol(const std::string& name, Grammar& grammar);
        virtual ~Symbol() = default;
    
        const std::string& Name() const;
        virtual bool IsTerminal() const;
        virtual bool IsNonTerminal() const;
        virtual bool IsEpsilon() const;
        virtual bool IsEndOfFile() const;
    
        bool operator==(const Symbol& other) const;
        bool operator<(const Symbol& other) const {
            return name < other.name;
        }
        //sobrecargar operador +
        // friend Sentence operator+(std::shared_ptr<Symbol> lhs, 
        //                         std::shared_ptr<Symbol> rhs);
        // friend Sentence operator+(std::shared_ptr<Symbol> lhs, 
        //                         const Sentence& rhs);
        // Sentence operator+(const std::vector<const Symbol*>& symbols);

        std::string ToString() const {
            return name;
        }
        Sentence GetSentence() const;
    protected:
        std::string name;
        // Type type;
        Grammar& grammar;
};

class Terminal : public Symbol {
    public:
        /**
         * @class Terminal
         * @brief Representa un símbolo terminal de la gramática.
         * 
         * Los terminales son los símbolos que aparecen en la entrada (tokens).
         */
        Terminal(const std::string& name, Grammar& grammar);
        bool IsTerminal() const override;
};
// class NonTerminal : public Symbol {
//     public:
//         NonTerminal(const std::string& name, Grammar& grammar);
//         bool IsNonTerminal() const override;
     
// };

namespace std {
    template<> 
    struct hash<Symbol> {
        size_t operator()(const Symbol& sym) const {
            return std::hash<std::string>{}(sym.Name());
        }
    };
    
    template<> 
    struct hash<std::shared_ptr<Symbol>> {
        size_t operator()(const std::shared_ptr<Symbol>& sym) const {
            return std::hash<std::string>{}(sym->Name());
        }
    };
    
    template<> 
    struct equal_to<std::shared_ptr<Symbol>> {
        bool operator()(const std::shared_ptr<Symbol>& a, 
                       const std::shared_ptr<Symbol>& b) const {
            return a->Name() == b->Name();
        }
    };
}

#endif