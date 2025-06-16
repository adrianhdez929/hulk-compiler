#pragma once
// #include "Automata/nfa.hpp"
// #include "Automata/dfa.hpp"
// #include "Automata/utils/ContainerSet.hpp"
// #include "Automata/utils/aut_manipulation.hpp"
#include <iostream>
#include <vector>
#include <variant>
// #include "Automata/operations/operations.hpp"
#include "Grammar/grammar.h"
// #include "Automata/state.hpp"
// #include "Lexer/node.hpp"
// #include "Automata/utils/utils.hpp"
// #include "Parser/Item.hpp"
// #include "Lexer/grammar_parser.hpp"
// #include "Parser/LR1Parser.h"
// #include "Parser/SLR1Parser.hpp"
#include <cassert>
#include <stack>
// #include "Lexer/Regex.hpp"
// #include "Lexer/Lexer.hpp"
#include "Ast/ast.hpp"

Grammar getHulkGrammar(){
	Grammar g;

	std::vector<std::pair<std::string, std::string>> table = {
        {"string", "\"([\\x20-!#-\\x7e])*\""},
        {"number", "(0|[1-9][0-9]*)(.[0-9]+)?"}, // Regular expression for numbers
        {"bool", "true|false"},  // Regular expression for boolean values
        {"type_id", "[A-Z][_a-zA-Z0-9]*"},
        {"var_id", "[_a-z][_a-zA-Z0-9]*"},
        {"space", " +"}, // Regular expression for spaces
        // Regular expression for identifiers
        {"(", "\\("},            // Left parenthesis
        {")", "\\)"},            // Right parenthesis
		{"{", "\\{"},
		{"}", "\\}"},
		{";", "\\;"},
		{"+", "\\+"},
		{"-", "\\-"},
		{"*", "\\*"},
		{"/", "\\/"},
		{"^", "\\^"}
    };


	auto epsilon = g.GetEpsilon();

// definir Terminales
	auto LKEY = g.SetTerminal("{");
	auto RKEY = g.SetTerminal("}");
	auto SEMICOLON = g.SetTerminal(";");
	auto PLUS = g.SetTerminal("+");
	auto MINUS = g.SetTerminal("-");
	auto TIMES = g.SetTerminal("*");
	auto DIV = g.SetTerminal("/");
	auto POW = g.SetTerminal("^");
	auto NUMBER = g.SetTerminal("number");
	auto LPARENT = g.SetTerminal("(");
	auto RPARENT = g.SetTerminal(")");

// definir no Terminales
	auto input = g.SetNonTerminal("input", true);
	auto line = g.SetNonTerminal("line");
	auto lines_block = g.SetNonTerminal("lines_block");
	auto lines = g.SetNonTerminal("lines");
	auto non_empty_lines = g.SetNonTerminal("non_empty_lines");
	auto expr = g.SetNonTerminal("expr");
	auto func_assign = g.SetNonTerminal("func_assign");
	auto type_node_decl = g.SetNonTerminal("type_node_decl");
	auto arit_op = g.SetNonTerminal("arit_op");
	auto add_expr = g.SetNonTerminal("add_expr");
	auto mult_expr = g.SetNonTerminal("mult_expr");
	auto power_expr = g.SetNonTerminal("power_expr");
	auto unary_expr = g.SetNonTerminal("unary_expr");
	auto primary_expr = g.SetNonTerminal("primary_expr");

// definir Producciones

	// input
	g.AddProduction(AttrProd(input, Sentence(line), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* line_node = std::get<ASTNode*>(args[0]);
		return line_node;
	}));
	g.AddProduction(AttrProd(input, Sentence(lines_block), [](const std::vector<ElementType>& args) -> ElementType {
		BlockNode* lines_block_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[0]));
		return (lines_block_node);
	}));

	//lines_block
	g.AddProduction(AttrProd(lines_block, Sentence({LKEY, lines, RKEY}), [](const std::vector<ElementType>& args) -> ElementType {
		BlockNode* lines_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[1]));
		return lines_node;
	}));

	//lines
	g.AddProduction(AttrProd(lines, Sentence(epsilon), [](const std::vector<ElementType>& args) -> ElementType {
		return new BlockNode({});
	}));
	g.AddProduction(AttrProd(lines, Sentence( non_empty_lines ), [](const std::vector<ElementType>& args) -> ElementType {
		BlockNode* non_empty_lines_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[0]));
		return non_empty_lines_node;
	}));

	//non_empty_lines
	g.AddProduction(AttrProd(non_empty_lines, Sentence( line ), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* node = std::get<ASTNode*>(args[0]);
		return new BlockNode({node});
	}));
	g.AddProduction(AttrProd(non_empty_lines, Sentence({non_empty_lines, line}), [](const std::vector<ElementType>& args) -> ElementType {
		BlockNode* n_e_l_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[0]));
		ASTNode* line_node = std::get<ASTNode*>(args[1]);

		n_e_l_node->add_child(line_node);
		return n_e_l_node;
	}));

	//line
	g.AddProduction(AttrProd(line, Sentence({expr, SEMICOLON}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node = std::get<ASTNode*>(args[0]);
		return expr_node;
	}));
	g.AddProduction(AttrProd(line, Sentence({func_assign, SEMICOLON}), [](const std::vector<ElementType>& args) -> ElementType {
		AssignFuncNode* func_assign_node = static_cast<AssignFuncNode*>(std::get<ASTNode*>(args[0]));
		return func_assign_node;
	}));
	g.AddProduction(AttrProd(line, Sentence( type_node_decl ), [](const std::vector<ElementType>& args) -> ElementType {
		TypeDeclNode* type_node_decl_node = static_cast<TypeDeclNode*>(std::get<ASTNode*>(args[0]));
		return type_node_decl_node;
	}));

	//expr
	g.AddProduction(AttrProd(expr, Sentence( arit_op ), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* arit_op_node = std::get<ASTNode*>(args[0]);
		return arit_op_node;
	}));


	//arit_op
	g.AddProduction(AttrProd(arit_op, Sentence( add_expr ), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* add_expr_node = std::get<ASTNode*>(args[0]);
		return add_expr_node;
	}));

	//add_expr
	g.AddProduction(AttrProd(add_expr, Sentence( {mult_expr, PLUS, add_expr} ), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* mult_expr_node = std::get<ASTNode*>(args[0]);
		ASTNode* add_expr_node = std::get<ASTNode*>(args[2]);
		return new BinOpNode(mult_expr_node, "+", add_expr_node);
	}));
	g.AddProduction(AttrProd(add_expr, Sentence( {mult_expr, MINUS, add_expr} ), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* mult_expr_node = std::get<ASTNode*>(args[0]);
		ASTNode* add_espr_node = std::get<ASTNode*>(args[2]);
		return new BinOpNode(mult_expr_node, "-", add_espr_node);
	}));
	g.AddProduction(AttrProd(add_expr, Sentence( mult_expr ), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* mult_expr_node = std::get<ASTNode*>(args[0]);
		return mult_expr_node;
	}));

	//mult_expr
	g.AddProduction(AttrProd(mult_expr, Sentence( {power_expr, TIMES, mult_expr} ), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* power_expr_node = std::get<ASTNode*>(args[0]);
		ASTNode* mult_expr_node = std::get<ASTNode*>(args[2]);
		return new BinOpNode(power_expr_node, "*", mult_expr_node);
	}));
	g.AddProduction(AttrProd(mult_expr, Sentence( {power_expr, DIV, mult_expr} ), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* power_expr_node = std::get<ASTNode*>(args[0]);
		ASTNode* mult_expr_node = std::get<ASTNode*>(args[2]);
		return new BinOpNode(power_expr_node, "/", mult_expr_node);
	}));
	g.AddProduction(AttrProd(mult_expr, Sentence( power_expr ), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* power_expr_node = std::get<ASTNode*>(args[0]);
		return power_expr_node;
	}));

	//power_expr
	g.AddProduction(AttrProd(power_expr, Sentence( {unary_expr, POW, power_expr} ), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* unary_expr_node = std::get<ASTNode*>(args[0]);
		ASTNode* power_expr_node = std::get<ASTNode*>(args[2]);
		return new BinOpNode(unary_expr_node, "^", power_expr_node);
	}));
	g.AddProduction(AttrProd(power_expr, Sentence( unary_expr ), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* unary_expr_node = std::get<ASTNode*>(args[0]);
		return unary_expr_node;
	}));

	//unary_expr
	g.AddProduction(AttrProd(unary_expr, Sentence( {MINUS, unary_expr} ), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* unary_expr_node = std::get<ASTNode*>(args[1]);
		return new UnaryOpNode("-", unary_expr_node);
	}));
	g.AddProduction(AttrProd(unary_expr, Sentence(primary_expr), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* primary_expr_node = std::get<ASTNode*>(args[0]);
		return primary_expr_node;
	}));

	//primary_expr
	g.AddProduction(AttrProd(primary_expr, Sentence( NUMBER ), [](const std::vector<ElementType>& args) -> ElementType {
		std::string number = std::get<std::string>(args[0]);
		float number_ = std::stof(number);
		return new FloatNode(number_);
	}));
	g.AddProduction(AttrProd(primary_expr, Sentence( {LPARENT, expr, RPARENT} ), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node = std::get<ASTNode*>(args[1]);
		return expr_node;
	}));

	return g;
}


