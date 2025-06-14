#pragma once
#include <string>
#include <vector>
#include <any>
#include <functional>
#include <unordered_map>
#include <stdexcept>
#include "SpecialTypes.h"
#include "node.h"
#include "../Ast/ast.hpp"

using AnyVec = std::vector<std::any>;
using InnerFun = std::function<std::any(const AnyVec&)>;
using FactoryFun = std::function<InnerFun(const std::vector<int>&)>;

InnerFun CreateSymbolNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> ElementType {
        if (indexes.size() != 1 || args.size() <= indexes[0]) {
            throw std::invalid_argument("SymbolNode requires a single string argument");
        }
        try {
            return std::make_shared<SymbolNode>(std::any_cast<std::string>(args[indexes[0]]));
        } catch(const std::bad_any_cast&) {
            throw std::invalid_argument("SymbolNode requires a string argument");
        }
    };
}
// REGISTER_FUN("SymbolNode", CreateSymbolNode);
InnerFun CreateUnionNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> ElementType {
        if (indexes.size() != 2 || args.size() <= std::max(indexes[0], indexes[1])) {
            throw std::invalid_argument("UnionNode requires two Node arguments");
        }
        try {
            return std::make_shared<UnionNode>(
                std::any_cast<std::shared_ptr<Node>>(args[indexes[0]]),
                std::any_cast<std::shared_ptr<Node>>(args[indexes[1]])
            );
        } catch(const std::bad_any_cast&) {
            throw std::invalid_argument("UnionNode requires Node arguments");
        }
    };
}
// REGISTER_FUN("UnionNode", CreateUnionNode);
InnerFun CreateConcatNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> ElementType {
        if (indexes.size() != 2 || args.size() <= std::max(indexes[0], indexes[1])) {
            throw std::invalid_argument("ConcatNode requires two Node arguments");
        }
        try {
            return std::make_shared<ConcatNode>(
                std::any_cast<std::shared_ptr<Node>>(args[indexes[0]]),
                std::any_cast<std::shared_ptr<Node>>(args[indexes[1]])
            );
        } catch(const std::bad_any_cast&) {
            throw std::invalid_argument("ConcatNode requires Node arguments");
        }
    };
}
// REGISTER_FUN("ConcatNode", CreateConcatNode);
InnerFun CreateRangeNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> ElementType {
        if (indexes.size() != 2 || args.size() <= std::max(indexes[0], indexes[1])) {
            throw std::invalid_argument("RangeNode requires two SymbolNode arguments");
        }
        try {
            return std::make_shared<RangeNode>(
                std::any_cast<std::shared_ptr<Node>>(args[indexes[0]]),
                std::any_cast<std::shared_ptr<Node>>(args[indexes[1]])
            );
        } catch(const std::bad_any_cast&) {
            throw std::invalid_argument("RangeNode requires SymbolNode arguments");
        }
    };
}
// REGISTER_FUN("RangeNode", CreateRangeNode);
InnerFun CreateClosureNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> ElementType {
        if (indexes.size() != 1 || args.size() <= indexes[0]) {
            throw std::invalid_argument("ClosureNode requires a single Node argument");
        }
        try {
            return std::make_shared<ClosureNode>(
                std::any_cast<std::shared_ptr<Node>>(args[indexes[0]])
            );
        } catch(const std::bad_any_cast&) {
            throw std::invalid_argument("ClosureNode requires a Node argument");
        }
    };
}
// REGISTER_FUN("ClosureNode", CreateClosureNode);
InnerFun CreatePositiveClosure(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> ElementType {
        if (indexes.size() != 1 || args.size() <= indexes[0]) {
            throw std::invalid_argument("PositiveClosure requires a single Node argument");
        }
        try {
            return std::make_shared<PositiveClosure>(
                std::any_cast<std::shared_ptr<Node>>(args[indexes[0]])
            );
        } catch(const std::bad_any_cast&) {
            throw std::invalid_argument("PositiveClosure requires a Node argument");
        }
    };
}
// REGISTER_FUN("PositiveClosure", CreatePositiveClosure);
InnerFun CreateZeroOrOneNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> ElementType {
        if (indexes.size() != 1 || args.size() <= indexes[0]) {
            throw std::invalid_argument("ZeroOrOneNode requires a single Node argument");
        }
        try {
            return std::make_shared<ZeroOrOneNode>(
                std::any_cast<std::shared_ptr<Node>>(args[indexes[0]])
            );
        } catch(const std::bad_any_cast&) {
            throw std::invalid_argument("ZeroOrOneNode requires a Node argument");
        }
    };
}
// REGISTER_FUN("ZeroOrOneNode", CreateZeroOrOneNode);
InnerFun CreateEpsilonNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> ElementType {
        if (indexes.size() != 0) {
            throw std::invalid_argument("EpsilonNode does not require any indexes");
        }
        return std::make_shared<EpsilonNode>();
    };
}
// REGISTER_FUN("EpsilonNode", CreateEpsilonNode);
// Factory function to create a Reduce node
InnerFun CreateReduceNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> ElementType {
        if (indexes.size() != 1 || args.size() <= indexes[0]) {
            throw std::invalid_argument("Reduce requires a single index");
        }
        int index = indexes[0];
        try {
            return std::any_cast<std::shared_ptr<Node>>(args[index]);
        } catch(const std::bad_any_cast&) {
            throw std::invalid_argument("Reduce requires a Node argument at index " + std::to_string(index));
        }
    };
}

//=====================Funciones Factory para los nodos AST===================================

InnerFun CreateProgramNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> std::any {
        if (indexes.size() != 1 || args.size() <= indexes[0]) {
            throw std::invalid_argument("ProgramNode requires a single ASTNode argument");
        }
        try {
            ASTNode* node = std::any_cast<ASTNode*>(args[indexes[0]]);
            ASTNode* program = new ProgramNode(node);
            return program;
        } catch(const std::bad_any_cast&) {
            throw std::invalid_argument("ProgramNode requires an ASTNode argument");
        }
    };
}

InnerFun CreateFloatNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> std::any {
        if (indexes.size() != 1 || args.size() <= indexes[0]) {
            throw std::invalid_argument("FloatNode requires a single float argument");
        }
        try {
            // Try to get a float directly
            float value = std::any_cast<float>(args[indexes[0]]);
            ASTNode* node = new FloatNode(value);
            return node;
        } catch(const std::bad_any_cast&) {
            // If not a float, try to get a string and convert it
            try {
                std::string str_val = std::any_cast<std::string>(args[indexes[0]]);
                ASTNode* node = new FloatNode(std::stof(str_val));
                return node;
            } catch(const std::exception&) {
                throw std::invalid_argument("FloatNode requires a float value or a string that can be converted to float");
            }
        }
    };
}

InnerFun CreateBoolNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> std::any {
        if (indexes.size() != 1 || args.size() <= indexes[0]) {
            throw std::invalid_argument("BoolNode requires a single boolean argument");
        }
        try {
            // Try to get a bool directly
            bool value = std::any_cast<bool>(args[indexes[0]]);
            ASTNode* node = new BoolNode(value);
            return node;
        } catch(const std::bad_any_cast&) {
            // If not a bool, try to get a string and convert it
            try {
                std::string str_val = std::any_cast<std::string>(args[indexes[0]]);
                if (str_val == "true" || str_val == "1") {
                    ASTNode* node = new BoolNode(true);
                    return node;
                } else if (str_val == "false" || str_val == "0") {
                    ASTNode* node = new BoolNode(false);
                    return node;
                }
                throw std::invalid_argument("Cannot convert string to bool: " + str_val);
            } catch(const std::bad_any_cast&) {
                throw std::invalid_argument("BoolNode requires a boolean value or a string 'true'/'false'");
            }
        }
    };
}

InnerFun CreateBoolExprNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> std::any {
        if (indexes.size() != 1 || args.size() <= indexes[0]) {
            throw std::invalid_argument("BoolExprNode requires a single ASTNode argument");
        }
        try {
            auto node = std::any_cast<ASTNode*>(args[indexes[0]]);
            ASTNode* result = new BoolExprNode(node);
            return result;
        } catch(const std::bad_any_cast&) {
            throw std::invalid_argument("BoolExprNode requires an ASTNode argument");
        }
    };
}

InnerFun CreateStringNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> std::any {
        if (indexes.size() != 1 || args.size() <= indexes[0]) {
            throw std::invalid_argument("StringNode requires a single string argument");
        }
        try {
            std::string value = std::any_cast<std::string>(args[indexes[0]]);
            ASTNode* node = new StringNode(value);
            return node;
        } catch(const std::bad_any_cast&) {
            throw std::invalid_argument("StringNode requires a string argument");
        }
    };
}

InnerFun CreateUnaryOpNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> std::any {
        if (indexes.size() != 2 || args.size() <= std::max(indexes[0], indexes[1])) {
            throw std::invalid_argument("UnaryOpNode requires an operator string and an ASTNode");
        }
        try {
            std::string op = std::any_cast<std::string>(args[indexes[0]]);
            auto node = std::any_cast<ASTNode*>(args[indexes[1]]);
            ASTNode* result = new UnaryOpNode(op, node);
            return result;
        } catch(const std::bad_any_cast&) {
            throw std::invalid_argument("UnaryOpNode requires a string for operator and an ASTNode");
        }
    };
}

InnerFun CreateBinOpNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> std::any {
        if (indexes.size() != 3 || args.size() <= std::max({indexes[0], indexes[1], indexes[2]})) {
            throw std::invalid_argument("BinOpNode requires a left ASTNode, operator string, and right ASTNode");
        }
        try {
            auto left = std::any_cast<ASTNode*>(args[indexes[0]]);
            std::string op = std::any_cast<std::string>(args[indexes[1]]);
            auto right = std::any_cast<ASTNode*>(args[indexes[2]]);
            ASTNode* result = new BinOpNode(left, op, right);
            return result;
        } catch(const std::bad_any_cast&) {
            throw std::invalid_argument("BinOpNode requires an ASTNode, a string operator, and an ASTNode");
        }
    };
}

InnerFun CreateFunctionCallNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> std::any {
        if (indexes.size() != 2 || args.size() <= std::max(indexes[0], indexes[1])) {
            throw std::invalid_argument("FunctionCallNode requires a function name and an ASTNode argument");
        }
        try {
            std::string func_name = std::any_cast<std::string>(args[indexes[0]]);
            auto arg = std::any_cast<ASTNode*>(args[indexes[1]]);
            ASTNode* result = new FunctionCallNode(func_name, arg);
            return result;
        } catch(const std::bad_any_cast&) {
            throw std::invalid_argument("FunctionCallNode requires a string for function name and an ASTNode");
        }
    };
}

InnerFun CreateIDNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> std::any {
        if (indexes.size() == 1 && args.size() > indexes[0]) {
            // Solo nombre
            try {
                std::string name = std::any_cast<std::string>(args[indexes[0]]);
                ASTNode* result = new IDNode(name);
                return result;
            } catch(const std::bad_any_cast&) {
                throw std::invalid_argument("IDNode requires a string for name");
            }
        } else if (indexes.size() == 2 && args.size() > std::max(indexes[0], indexes[1])) {
            // Nombre y tipo
            try {
                std::string name = std::any_cast<std::string>(args[indexes[0]]);
                std::string type = std::any_cast<std::string>(args[indexes[1]]);
                ASTNode* result = new IDNode(name, type);
                return result;
            } catch(const std::bad_any_cast&) {
                throw std::invalid_argument("IDNode requires strings for name and type");
            }
        } else {
            throw std::invalid_argument("IDNode requires either 1 or 2 string arguments");
        }
    };
}

InnerFun CreateBlockNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> std::any {
        std::vector<ASTNode*> nodes;
        for (int index : indexes) {
            if (args.size() <= index) {
                throw std::invalid_argument("BlockNode: index out of bounds");
            }
            try {
                auto node = std::any_cast<ASTNode*>(args[index]);
                nodes.push_back(node);
            } catch(const std::bad_any_cast&) {
                throw std::invalid_argument("BlockNode requires ASTNode arguments");
            }
        }
        ASTNode* result = new BlockNode(nodes);
        return result;
    };
}

InnerFun CreateArgsListNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> std::any {
        std::vector<IDNode*> nodes;
        for (int index : indexes) {
            if (args.size() <= index) {
                throw std::invalid_argument("ArgsList: index out of bounds");
            }
            try {
                auto node = std::any_cast<ASTNode*>(args[index]);
                IDNode* id_node = dynamic_cast<IDNode*>(node);
                if (!id_node) {
                    throw std::invalid_argument("ArgsList requires IDNode arguments");
                }
                nodes.push_back(id_node);
            } catch(const std::bad_any_cast&) {
                throw std::invalid_argument("ArgsList requires IDNode arguments");
            }
        }
        ASTNode* result = new ArgsList(nodes);
        return result;
    };
}

InnerFun CreateExprsListNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> std::any {
        std::vector<ASTNode*> nodes;
        for (int index : indexes) {
            if (args.size() <= index) {
                throw std::invalid_argument("ExprsList: index out of bounds");
            }
            try {
                auto node = std::any_cast<ASTNode*>(args[index]);
                nodes.push_back(node);
            } catch(const std::bad_any_cast&) {
                throw std::invalid_argument("ExprsList requires ASTNode arguments");
            }
        }
        ASTNode* result = new ExprsList(nodes);
        return result;
    };
}

InnerFun CreateConditionalNode(const std::vector<int>& indexes) {
    return [indexes](const AnyVec& args) -> std::any {
        if (indexes.size() != 3 || args.size() <= std::max({indexes[0], indexes[1], indexes[2]})) {
            throw std::invalid_argument("Conditional requires a BoolExprNode and two ASTNode arguments");
        }
        try {
            auto bool_expr = std::any_cast<ASTNode*>(args[indexes[0]]);
            BoolExprNode* bool_node = dynamic_cast<BoolExprNode*>(bool_expr);
            if (!bool_node) {
                throw std::invalid_argument("Conditional requires a BoolExprNode as first argument");
            }
            
            auto if_body = std::any_cast<ASTNode*>(args[indexes[1]]);
            auto else_body = std::any_cast<ASTNode*>(args[indexes[2]]);
            
            ASTNode* result = new Conditional(bool_node, if_body, else_body);
            return result;
        } catch(const std::bad_any_cast&) {
            throw std::invalid_argument("Conditional requires proper types for condition, if and else bodies");
        }
    };
}

// Declare extern in header file
extern std::unordered_map<std::string, FactoryFun> funRegistry;

// Define in this file (implementation)
std::unordered_map<std::string, FactoryFun> funRegistry = {
    {"SymbolNode", CreateSymbolNode},
    {"UnionNode", CreateUnionNode},
    {"ConcatNode", CreateConcatNode},
    {"RangeNode", CreateRangeNode},
    {"ClosureNode", CreateClosureNode},
    {"PositiveClosure", CreatePositiveClosure},
    {"ZeroOrOneNode", CreateZeroOrOneNode},
    {"EpsilonNode", CreateEpsilonNode},
    {"Reduce", CreateReduceNode},
    {"ProgramNode", CreateProgramNode},
    {"FloatNode", CreateFloatNode},
    {"BoolNode", CreateBoolNode},
    {"BoolExprNode", CreateBoolExprNode},
    {"StringNode", CreateStringNode},
    {"UnaryOpNode", CreateUnaryOpNode},
    {"BinOpNode", CreateBinOpNode},
    {"FunctionCallNode", CreateFunctionCallNode},
    {"IDNode", CreateIDNode},
    {"BlockNode", CreateBlockNode},
    {"ArgsList", CreateArgsListNode},
    {"ExprsList", CreateExprsListNode},
    {"Conditional", CreateConditionalNode}
};

inline FactoryFun getFactoryFunction(const std::string& name) {
    auto it = funRegistry.find(name);
    if (it != funRegistry.end()) {
        return it->second;
    } else {
        throw std::runtime_error("Function not registered: " + name);
    }
}
