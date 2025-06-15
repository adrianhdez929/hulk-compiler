#pragma once
#include <memory>
#include "../Automata/nfa.h"
#include "../Automata/operations/operations.h"

//BASIC NODES
class Node {
public:
    virtual ~Node() = default;
    virtual std::shared_ptr<NFA> evaluate() = 0;
    virtual std::string ToString() const {
        return "Node";
    }
};

class AtomicNode : public Node {
public:
    /// @brief Evaluates the atomic node and returns an NFA.
    /// This method should be overridden by derived classes to provide specific NFA evaluations.
    /// @return A shared pointer to an NFA that represents the atomic node.
    virtual std::shared_ptr<NFA> evaluate() = 0;

    /// @brief Returns a string representation of the atomic node.
    /// @return A string describing the atomic node.
    std::string ToString() const override {
        return "AtomicNode";
    }

};

class UnaryNode : public Node {
protected:
    std::shared_ptr<Node> child_;  // Hijo único
public:
    explicit UnaryNode(std::shared_ptr<Node> child) : child_(std::move(child)) {}
    virtual ~UnaryNode() = default;
    /// @brief Returns a string representation of the unary node.
    /// @return A string describing the unary node.
    std::string ToString() const override {
        return "UnaryNode(" + (child_ ? child_->ToString() : "nullptr") + ")";
    }
};

class BinaryNode : public Node {
protected:
    std::shared_ptr<Node> left;  // Hijo izquierdo
    std::shared_ptr<Node> right;  // Hijo derecho
public:
    BinaryNode(std::shared_ptr<Node> l, std::shared_ptr<Node> r) : left(std::move(l)), right(std::move(r)) {}
    virtual ~BinaryNode() = default;
    /// @brief Returns a string representation of the binary node.
    /// @return A string describing the binary node.
    std::string ToString() const override {
        return "BinaryNode(" + (left ? left->ToString() : "nullptr") + ", " + (right ? right->ToString() : "nullptr") + ")";
    }
};

//LEXER NODES/////////////////////////////////////////////////////////////////////
class EpsilonNode : public AtomicNode {
public:
    /// @brief Evaluates the epsilon node and returns an NFA that accepts epsilon transitions.
    /// @return A shared pointer to an NFA that accepts epsilon transitions.
    std::shared_ptr<NFA> evaluate() override {
        NFA::Transitions transitions;
        transitions[{0, ""}] = {1};  // Transición epsilon desde el estado 0 al estado 1
        return std::make_shared<NFA>(2, std::initializer_list<int>{1}, transitions, 0);  // Automata con 2 estados, estado final 1, transiciones y estado inicial 0
    }
    /// @brief Returns a string representation of the epsilon node.
    /// @return A string describing the epsilon node.
    std::string ToString() const override {
        return "EpsilonNode";
    }
};

class SymbolNode : public AtomicNode {
    std::string symbol_;
public:
    explicit SymbolNode(const std::string& symbol) : symbol_(symbol) {}
    const std::string& lex() const {
        return symbol_;
    }
    /// @brief Evaluates the symbol node and returns an NFA that accepts the symbol.
    /// @return A shared pointer to an NFA that accepts the symbol.
    std::shared_ptr<NFA> evaluate() override {
        NFA::Transitions transitions;
        transitions[{0, symbol_}] = {1};
        return std::make_shared<NFA>(2, std::initializer_list<int>{1}, transitions, 0);  // Automata con 2 estados, estado final 1, transiciones y estado inicial 0
    }
    /// @brief Returns a string representation of the symbol node.
    /// @return A string describing the symbol node.
    std::string ToString() const override {
        return "SymbolNode(" + symbol_ + ")";
    }
        
};

class ClosureNode : public UnaryNode {
public:
    explicit ClosureNode(std::shared_ptr<Node> child) : UnaryNode(std::move(child)) {
        if (!child_) {
            throw std::invalid_argument("ClosureNode must have a child node.");
        }
    }
    /// @brief Evaluates the closure of the child NFA node.
    /// This node represents the Kleene star operation, which allows for zero or more occurrences of the child NFA.
    /// @return A shared pointer to an NFA that represents the closure of the child NFA node.
    std::shared_ptr<NFA> evaluate() override {
        std::shared_ptr<NFA> child_nfa = child_->evaluate();
        NFA closure = closure_nfa(*child_nfa);
        return std::make_shared<NFA>(closure);
    }
    ~ClosureNode() override = default;  // Ensure proper cleanup of child node
    /// @brief Returns a string representation of the closure node.
    /// @return A string describing the closure node.
    std::string ToString() const override {
        return "ClosureNode(" + (child_ ? child_->ToString() : "nullptr") + ")";
    }
};

class UnionNode : public BinaryNode {
public:
    UnionNode(std::shared_ptr<Node> left, std::shared_ptr<Node> right) 
        : BinaryNode(std::move(left), std::move(right)) {}
    /// @brief Evaluates the union of two NFA nodes.
    /// This node represents the union operation between two NFA nodes.
    /// @return A shared pointer to an NFA that represents the union of the two NFA nodes.
    std::shared_ptr<NFA> evaluate() override {
        std::shared_ptr<NFA> left_nfa = left->evaluate();
        std::shared_ptr<NFA> right_nfa = right->evaluate();
        NFA automata_union = union_nfa(*left_nfa, *right_nfa);
        return std::make_shared<NFA>(automata_union);
    }
    ~UnionNode() override = default;  // Ensure proper cleanup of child nodes
    /// @brief Returns a string representation of the union node.
    /// @return A string describing the union node.
    std::string ToString() const override {
        return "UnionNode(" + (left ? left->ToString() : "nullptr") + ", " + (right ? right->ToString() : "nullptr") + ")";
    }
};

class ConcatNode : public BinaryNode {
public:
    ConcatNode(std::shared_ptr<Node> left, std::shared_ptr<Node> right) 
        : BinaryNode(std::move(left), std::move(right)) {}
    /// @brief Evaluates the concatenation of two NFA nodes.
    /// @return A shared pointer to an NFA that represents the concatenation of the two NFA nodes.
    std::shared_ptr<NFA> evaluate() override {
        std::shared_ptr<NFA> left_nfa = left->evaluate();
        std::shared_ptr<NFA> right_nfa = right->evaluate();
        NFA automata_concat = concat_nfa(*left_nfa, *right_nfa);
        return std::make_shared<NFA>(automata_concat);
    }
    ~ConcatNode() override = default;  // Ensure proper cleanup of child nodes
    /// @brief Returns a string representation of the concatenation node.
    /// @return A string describing the concatenation node.
    std::string ToString() const override {
        return "ConcatNode(" + (left ? left->ToString() : "nullptr") + ", " + (right ? right->ToString() : "nullptr") + ")";
    }
};

class StringClassNode : public Node {
    std::vector<std::shared_ptr<SymbolNode>> symbols_;  // Vector de símbolos que componen la clase de cadenas
public:
    explicit StringClassNode(std::vector<shared_ptr<SymbolNode>> symbols) : symbols_(std::move(symbols)) {
        if (symbols_.empty()) {
            throw std::invalid_argument("StringClassNode must have at least one symbol.");
        }
    }
    /// @brief Evaluates the string class node by creating an NFA that accepts any of the symbols in the class.
    /// This node represents a set of symbols that can be accepted by the NFA.
    /// @return A shared pointer to an NFA that accepts any of the symbols in the class.
    std::shared_ptr<NFA> evaluate() override {
        std::shared_ptr<NFA> result = symbols_[0]->evaluate();
        for(size_t i = 1; i < symbols_.size(); ++i) {
            std::shared_ptr<NFA> next_nfa = symbols_[i]->evaluate();
            result = std::make_shared<NFA>(union_nfa(*result, *next_nfa));
        }
        return result;
    }
    /// @brief Returns a string representation of the string class node.
    /// @return A string describing the string class node.
    std::string ToString() const override {
        std::string result = "StringClassNode(";
        for (const auto& symbol : symbols_) {
            result += symbol->ToString() + ", ";
        }
        if (!symbols_.empty()) {
            result.pop_back();  // Eliminar la última coma
            result.pop_back();  // Eliminar el espacio
        }
        result += ")";
        return result;
    }
};

class RangeNode : public Node {
    std::shared_ptr<SymbolNode> first_;  // Usar unique_ptr
    std::shared_ptr<SymbolNode> last_;
public:
    RangeNode(std::shared_ptr<Node> first, std::shared_ptr<Node> last) 
        : first_(nullptr), last_(nullptr) {
        // Verificar que first y last no sean nullptr
        if (!first || !last) {
            throw std::invalid_argument("RangeNode: ambos nodos deben ser SymbolNode");
        }

        // Si first y last no son SymbolNode, lanzar excepción
        if (!std::dynamic_pointer_cast<SymbolNode>(first) || !std::dynamic_pointer_cast<SymbolNode>(last)) {
            throw std::invalid_argument("RangeNode: ambos nodos deben ser SymbolNode");
        }
        auto new_first_ = std::dynamic_pointer_cast<SymbolNode>(first);
        auto new_last_ = std::dynamic_pointer_cast<SymbolNode>(last);
        if (!new_first_ || !new_last_) {
            throw std::invalid_argument("RangeNode: símbolos no pueden ser nullptr");
        }
        first_ = new_first_;
        last_ = new_last_;
    }

    /// @brief Evaluates the range from first_ to last_ and returns an NFA that accepts all symbols in that range.
    /// @return A shared pointer to an NFA that accepts the range of symbols.
    std::shared_ptr<NFA> evaluate() override {
        if (first_->lex().empty() || last_->lex().empty()) {
            throw std::invalid_argument("RangeNode: los símbolos no pueden estar vacíos");
        }
        std::shared_ptr<NFA> result = first_->evaluate();
        char start = first_->lex()[0];
        char end = last_->lex()[0];
        if (start > end) {
            throw std::invalid_argument("RangeNode: el primer símbolo debe ser menor o igual al segundo");
        }
        for (char c = start + 1; c <= end; ++c) {
            std::string symbol(1, c);
            std::shared_ptr<NFA> next_nfa = std::make_shared<SymbolNode>(symbol)->evaluate();
            result = std::make_shared<NFA>(union_nfa(*result, *next_nfa));
        }
        return result;
    }
    /// @brief Returns a string representation of the range node.
    /// @return A string describing the range node.
    std::string ToString() const override {
        return "RangeNode(" + (first_ ? first_->ToString() : "nullptr") + ", " + (last_ ? last_->ToString() : "nullptr") + ")";
    }
};

class ZeroOrOneNode : public UnaryNode {
public:
    explicit ZeroOrOneNode(std::shared_ptr<Node> child)  // Recibir unique_ptr
        : UnaryNode(std::move(child)) {}
    /// @brief Evaluates the child node and returns an NFA that accepts the child NFA or an epsilon transition.
    /// @return A shared pointer to an NFA that accepts the child NFA or an epsilon transition.

    std::shared_ptr<NFA> evaluate() override {
        std::shared_ptr<NFA> child_nfa = child_->evaluate();
        std::shared_ptr<NFA> epsilon_nfa = std::make_shared<EpsilonNode>()->evaluate();
        NFA union_nfa_result = union_nfa(*child_nfa, *epsilon_nfa);
        return std::make_shared<NFA>(union_nfa_result);
    }
    ~ZeroOrOneNode() override = default;  // Ensure proper cleanup of child node
    /// @brief Returns a string representation of the zero or one node.
    /// @return A string describing the zero or one node.
    std::string ToString() const override {
        return "ZeroOrOneNode(" + (child_ ? child_->ToString() : "nullptr") + ")";
    }
};

class PositiveClosure : public UnaryNode {
public:
    explicit PositiveClosure(std::shared_ptr<Node> child)  // Recibir unique_ptr
        : UnaryNode(std::move(child)) {}
    
    /// @brief Evaluates the child node and returns an NFA that accepts one or more occurrences of the child NFA.
    /// @return A shared pointer to an NFA that accepts one or more occurrences of the child NFA.
    std::shared_ptr<NFA> evaluate() override {
        std::shared_ptr<NFA> child_nfa = child_->evaluate();
        // Concatenation of child NFA with its closure
        NFA closure = closure_nfa(*child_nfa);
        NFA concat_nfa_result = concat_nfa(*child_nfa, closure);
        return std::make_shared<NFA>(concat_nfa_result);
    }
    ~PositiveClosure() override = default;  // Ensure proper cleanup of child node
    /// @brief Returns a string representation of the positive closure node.
    /// @return A string describing the positive closure node.
    std::string ToString() const override {
        return "PositiveClosure(" + (child_ ? child_->ToString() : "nullptr") + ")";
    }
};