#pragma once
#include <variant>
#include <memory>

// using AnyVec = std::vector<any>;
// using InnerFun = std::function<std::any(const AnyVec&)>;
// using FactoryFun = std::function<InnerFun(const std::vector<int>&)>;

class SymbolNode;
class UnionNode;
class ConcatNode;
class StringClassNode;
class RangeNode;
class ClosureNode;
class PositiveClosure;
class ZeroOrOneNode;
class EpsilonNode;
class Node;

using ElementType = std::variant<
    std::string,
    std::shared_ptr<Node>,
    std::shared_ptr<SymbolNode>
>;
            // std::shared_ptr<SymbolNode>,
        // std::shared_ptr<UnionNode>,
        // std::shared_ptr<ConcatNode>,
        // std::shared_ptr<StringClassNode>,
        // std::shared_ptr<RangeNode>,
        // std::shared_ptr<ClosureNode>,
        // std::shared_ptr<PositiveClosure>,
        // std::shared_ptr<ZeroOrOneNode>,
        // std::shared_ptr<EpsilonNode>,