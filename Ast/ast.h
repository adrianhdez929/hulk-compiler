#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <variant>
#include <stdexcept>
#include <string>

#ifndef AST_H
#define AST_H

using namespace std;
class Node {
public:
    virtual ~Node() = default;
    //Funcion virtual evaluate destinada a ser reeescrita por sus hijos
    //Devuelve tipo generico (para el lexer devuelve una instancia de automata, para parser devuelve una instancia de un nodo)
    virtual void evaluate() const = 0;
    virtual void operate() const = 0;
};
    
// Nodo atómico (hoja del AST)
class AtomicNode : public Node {
protected:
    std::string lex;  // Token léxico
    
public:
    explicit AtomicNode(const std::string& lexeme) : lex(lexeme) {}
    
    // // En la práctica, aquí iría la lógica para convertir el lexema a valor
    // double evaluate() const override {
    //     // Ejemplo básico: convertir string a double
    //     return std::stod(lex);  // En realidad dependería del tipo de token
    // }
};
    
// Nodo unario abstracto
class UnaryNode : public Node {
protected:
    std::unique_ptr<const Node> node;  // Hijo único
    
public:
    explicit UnaryNode(std::unique_ptr<const Node> child) 
        : node(std::move(child)) {
        
    // double evaluate() const override {
    //     const double value = node->evaluate();
    //     return operate(value);
    }
        
    void evaluate() const override {
        std::cout << "Unary node. " << std::endl;
    }
    
    virtual double operate(double value) const = 0;
};

// Nodo binario abstracto
class BinaryNode : public Node {
protected:
    std::unique_ptr<const Node> left;   // Hijo izquierdo
    std::unique_ptr<const Node> right;  // Hijo derecho
        
public:
    BinaryNode(std::unique_ptr<const Node> l, std::unique_ptr<const Node> r)
        : left(std::move(l)), right(std::move(r)) {}
    
    // double evaluate() const override {
    //     const double lvalue = left->evaluate();
    //     const double rvalue = right->evaluate();
    //     return operate(lvalue, rvalue);
    // }
    void evaluate() const override {
        std::cout << "Binary node. " << std::endl;
    }

    virtual double operate(double lvalue, double rvalue) const = 0;
};
class SymbolNode : public AtomicNode {
public:
    SymbolNode(const std::string& lexeme) : AtomicNode(lexeme) {}
    
    // double evaluate() const override {
    //     // Aquí iría la lógica para convertir el lexema a valor
    //     return std::stod(lex);  // En realidad dependería del tipo de token
    // }

    void evaluate() const override {
        std::cout << "Symbol node: " << lex << std::endl;
    }
};

class UnionNode : public BinaryNode {
public:
    UnionNode(std::unique_ptr<const Node> l, std::unique_ptr<const Node> r)
        : BinaryNode(std::move(l), std::move(r)) {}
    void evaluate() const override {
        std::cout << "Union node. " << std::endl;
    }
    double operate(double lvalue, double rvalue) const override {
        // Implementar la operación de unión
        return 0.0;  // Placeholder
    }
};

#endif