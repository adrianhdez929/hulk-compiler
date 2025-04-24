
#include <string>
#include <vector>
#include <memory>

#ifndef SYMBOL_H
#define SYMBOL_H

class Grammar;
class Sentence;

class Symbol {
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
        //sobrecargar operador +
        Sentence operator+(const Symbol& other) const;
        Sentence operator+(const Sentence& other) const;
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
        Terminal(const std::string& name, Grammar& grammar);
        bool IsTerminal() const override;
};
// class NonTerminal : public Symbol {
//     public:
//         NonTerminal(const std::string& name, Grammar& grammar);
//         bool IsNonTerminal() const override;
     
// };

#endif