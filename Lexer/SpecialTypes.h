#pragma once
#include <variant>
#include <memory>
#include "Token.h"

class SymbolNode;
// class UnionNode;
// class ConcatNode;
// class StringClassNode;
// class RangeNode;
// class ClosureNode;
// class PositiveClosure;
// class ZeroOrOneNode;
// class EpsilonNode;
class Node;
class ASTNode;
class ASTNodeVector;
class ProgramNode;
class FloatNode;
class BoolNode;
class BoolExprNode;
class StringNode;
class UnaryOpNode;
class BinOpNode;
class FunctionCallNode;
class IDNode;
class BlockNode;
class ArgsList;
class ExprsList;
class AssignFuncNode;
class VarAssign;
class NewTypeNode;
class VarAssignType;
class VarAssignList;
class LetAssign;
class VarDesAssign;
class Conditional;
class WhileNode;
class ForNode;
class TypeDeclNode;
class TypeAssMember;
class AttributeMember;
class MethodMember;
class AccessNode;
class Token;

using ElementType = std::variant<
    std::string,
    std::shared_ptr<Node>,
    ASTNode*,
	Token
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