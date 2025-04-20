//Nodes for an AST
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <variant>

#ifndef AST_H
#define AST_H



// Clase base para nodos AST de la q heredaran los demas nodos
// Todos los nodos poseen un metodo evaluate.
// Todo nodo que herede de Node, puede redefinir el metodo evaluate
// Clase base para nodos AST
using namespace std;
#include <memory>
#include <stdexcept>
#include <string>

// Clase base abstracta
class Node {
public:
    virtual ~Node() = default;
    //Funcion virtual evaluate destinada a ser reeescrita por sus hijos
    //Devuelve tipo generico (para el lexer devuelve una instancia de automata, para parser devuelve una instancia de un nodo)
    virtual double evaluate() const = 0;
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
        : node(std::move(child)) {}
    
    double evaluate() const override {
        const double value = node->evaluate();
        return operate(value);
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
    
    double evaluate() const override {
        const double lvalue = left->evaluate();
        const double rvalue = right->evaluate();
        return operate(lvalue, rvalue);
    }
    
    virtual double operate(double lvalue, double rvalue) const = 0;
};

#endif
