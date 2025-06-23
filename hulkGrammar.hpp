#pragma once
// #include "Automata/nfa.hpp"
// #include "Automata/dfa.hpp"
// #include "Automata/utils/ContainerSet.hpp"
// #include "Automata/utils/aut_manipulation.hpp"
#include <iostream>
#include <vector>
#include <variant>
#include <functional>
// #include "Automata/operations/operations.hpp"
#include "Grammar/grammar.h"
// #include "Automata/state.h"
// #include "Lexer/node.h"
// #include "Automata/utils/utils.h"
// #include "Parser/Item.h"
// #include "Lexer/grammar_parser.h"
// #include "Parser/LR1Parser.h"
// #include "Parser/SLR1Parser.h"
#include <cassert>
#include <stack>
// #include "Lexer/Regex.h"
// #include "Lexer/Lexer.h"
#include "Ast/ast.hpp"
#include "Parser/line_helpers.h" // Utilidades para calcular líneas mínimas

inline Grammar getHulkGrammar(){
	Grammar g;

	// Esta tabla es de guia, para saber los terminales y su representacion en el lexer.
	// Debe ser muy similar si no identica a la tabla de tokens del lexer.
	std::vector<std::pair<std::string, std::string>> table = {
        {"string", "\"([\\x20-!#-\\x7e])*\""},
        {"number", "((0|[1-9][0-9]*)(\\.[0-9]+)?)"}, // Regular expression for numbers
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
		{"^", "\\^"},
		{"%", "\\%"},
        {":", "\\:"},
        {"|", "\\|"},
        {"&", "\\&"},
        {">=", ">="},
        {">", "\\>"},
        {"<=", "<="},
        {"<", "<"},
        {"==", "=="},
        {"!=", "!="},
        {"is", "is"},
        {"!", "!"},
        {",", "\\,"},           // Comma for lists
        {"=", "="},            // Assignment operator
        {":=", ":="},          // Alternative assignment operator
        {"function", "function"}, // Function keyword
        {"=>", "=>"},          // Arrow for inline functions
        {"let", "let"},        // Let keyword
        {"in", "in"},          // In keyword for let expressions
        {"if", "if"},          // If keyword
        {"else", "else"},      // Else keyword
        {"elif", "elif"},      // Else-if keyword
        {"while", "while"},    // While keyword
        {"type", "type"},      // Type definition keyword
        {".", "\\."},          // Member access operator
        {"@", "@"},            // At operator
        {"@@", "@@"},          // Double at operator
        {"as", "as"},          // Type cast keyword
        {"inherits", "inherits"}, // Inheritance keyword
        {"for", "for"},        // For loop keyword
        {"new", "new"}         // Object instantiation keyword
    };


	auto epsilon = g.SetEpsilon();

// definir Terminales
	auto LKEY = g.SetTerminal("{");
	auto RKEY = g.SetTerminal("}");
	auto SEMICOLON = g.SetTerminal(";");
	auto PLUS = g.SetTerminal("+");
	auto MINUS = g.SetTerminal("-");
	auto TIMES = g.SetTerminal("*");
	auto DIV = g.SetTerminal("/");
	auto POW = g.SetTerminal("^");
	auto MOD = g.SetTerminal("%");
	auto NUMBER = g.SetTerminal("number");
	auto LPARENT = g.SetTerminal("(");
	auto RPARENT = g.SetTerminal(")");
	auto STRING = g.SetTerminal("string");
	auto ID_ = g.SetTerminal("var_id");
	auto TWO_POINTS = g.SetTerminal(":");
	auto OR_ = g.SetTerminal("|");
	auto AND_ = g.SetTerminal("&");
	auto GR_EQ_ = g.SetTerminal(">=");
	auto GR_ = g.SetTerminal(">");	
	auto LESS_EQ_ = g.SetTerminal("<=");
	auto LESS_ = g.SetTerminal("<");
	auto EQ_ = g.SetTerminal("==");
	auto DISTINCT_ = g.SetTerminal("!=");
	auto IS_ = g.SetTerminal("is");
	auto NOT_ = g.SetTerminal("!");
	auto BOOLEAN = g.SetTerminal("bool");
	auto COLON = g.SetTerminal(",");
	auto FUNCTION_ = g.SetTerminal("function");
	auto INLINE = g.SetTerminal("=>");
	auto EQUAL = g.SetTerminal("=");
	auto ASS_DES = g.SetTerminal(":=");
	auto IF = g.SetTerminal("if");
	auto ELSE = g.SetTerminal("else");
	auto ELIF = g.SetTerminal("elif");
	auto WHILE = g.SetTerminal("while");
	auto LET = g.SetTerminal("let");
	auto IN = g.SetTerminal("in");
	auto TYPE_ = g.SetTerminal("type");
	auto ACCESS = g.SetTerminal(".");
	auto ARROBA_ = g.SetTerminal("@");
	auto D_ARROBA_ = g.SetTerminal("@@");
	auto AS_ = g.SetTerminal("as");
	auto INHERITS = g.SetTerminal("inherits");
	auto FOR = g.SetTerminal("for");
	auto NEW = g.SetTerminal("new");
	auto TYPE_ID = g.SetTerminal("type_id");

// definir no Terminales
	auto input = g.SetNonTerminal("input", true);
	auto line = g.SetNonTerminal("line");
	auto lines_block = g.SetNonTerminal("lines_block");
	auto lines = g.SetNonTerminal("lines");
	auto decl = g.SetNonTerminal("decl");
	auto decl_list = g.SetNonTerminal("decl_list");
	auto declarations = g.SetNonTerminal("declarations");
	auto non_empty_lines = g.SetNonTerminal("non_empty_lines");
	auto expr = g.SetNonTerminal("expr");
	auto func_assign = g.SetNonTerminal("func_assign");
	auto func_full_assign = g.SetNonTerminal("func_full_assign");
	auto type_node_decl = g.SetNonTerminal("type_node_decl");
	auto type_elements = g.SetNonTerminal("type_elements");
	auto arit_op = g.SetNonTerminal("arit_op");
	// auto add_expr = g.SetNonTerminal("add_expr");
	// auto mult_expr = g.SetNonTerminal("mult_expr");
	// auto power_expr = g.SetNonTerminal("power_expr");
	// auto unary_expr = g.SetNonTerminal("unary_expr");
	// auto primary_expr = g.SetNonTerminal("primary_expr");
	auto id_expr = g.SetNonTerminal("id_expr");
	auto bool_expr = g.SetNonTerminal("bool_expr");
	// auto or_expr = g.SetNonTerminal("or_expr");
	// auto and_expr = g.SetNonTerminal("and_expr");
	// auto comp_expr = g.SetNonTerminal("comp_expr");
	// auto sum_expr = g.SetNonTerminal("sum_expr");
	// auto not_expr = g.SetNonTerminal("not_expr");
	// auto bool_primary = g.SetNonTerminal("bool_primary");
	auto args_list = g.SetNonTerminal("args_list");
	auto let_exp = g.SetNonTerminal("let_exp");
	auto var_assign_list = g.SetNonTerminal("var_assign_list");
	auto conditional = g.SetNonTerminal("conditional");
	auto while_expr = g.SetNonTerminal("while_expr");
	// auto for_expr = g.SetNonTerminal("for_expr");
	auto type_body_elements = g.SetNonTerminal("type_body_elements");
	auto attribute = g.SetNonTerminal("attribute");
	auto attributes = g.SetNonTerminal("attributes");
	auto method = g.SetNonTerminal("method");
	auto methods = g.SetNonTerminal("method_list");
	auto member_access_expr = g.SetNonTerminal("member_access_expr");
	auto new_expr = g.SetNonTerminal("new_expr");
	auto expr_list = g.SetNonTerminal("expr_list");
	auto func_call = g.SetNonTerminal("func_call");
	auto term = g.SetNonTerminal("term");
	auto factor = g.SetNonTerminal("factor");
	auto atom = g.SetNonTerminal("atom");
	auto sign = g.SetNonTerminal("sign");

// ====================== DEFINICIÓN DE PRODUCCIONES ======================
/*
 * La gramática del lenguaje HULK se organiza jerárquicamente, comenzando con la producción
 * inicial "input" que representa un programa completo. Un programa puede ser una única línea
 * o un bloque de código entre llaves. Las producciones se han diseñado para manejar expresiones, 
 * declaraciones, estructuras de control y definiciones de tipos.
 */

	// --------- Producción inicial del programa ---------
	// // input -> line
	// // Una línea de código como programa completo
	// g.AddProduction(AttrProd(input, Sentence(line), [](const std::vector<ElementType>& args) -> ElementType {
	// 	ASTNode* line_node = std::get<ASTNode*>(args[0]);
	// 	return line_node;
	// }));

	// input -> decl_list lines_block
	g.AddProduction(AttrProd(input, Sentence({decl_list, lines_block}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNodeVector* decl_list_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[0]));
		BlockNode* lines_block_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[1]));
		// Añadir las declaraciones al bloque de líneas
		for (ASTNode* decl : decl_list_node->children) {
			lines_block_node->add_child(decl);
		}
		return lines_block_node;
	}));
	//decl_list -> ε
	g.AddProduction(AttrProd(decl_list, Sentence(epsilon), [](const std::vector<ElementType>& args) -> ElementType {
		int minLine = getMinLineFromArgs(args);
		return new ASTNodeVector({}, minLine);
	}));
	// // decl_list -> decl decl_list
	// g.AddProduction(AttrProd(decl_list, Sentence({decl, decl_list}), [](const std::vector<ElementType>& args) -> ElementType {
	// 	ASTNodeVector* decl_list_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[1]));
	// 	ASTNode* decl_node = std::get<ASTNode*>(args[0]);
	// 	decl_list_node->add_child(decl_node);
	// 	return decl_list_node;
	// }));

	// decl_list -> declarations
	g.AddProduction(AttrProd(decl_list, Sentence(declarations), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNodeVector* decl_list_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[0]));
		return decl_list_node;
	}));

	//declarations -> decl
	g.AddProduction(AttrProd(declarations, Sentence(decl), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* decl_node = std::get<ASTNode*>(args[0]);
		int minline = getMinLineFromArgs(args);
		ASTNodeVector* decl_list_node = new ASTNodeVector({}, minline);
		decl_list_node->add_child(decl_node);
		return decl_list_node;
	}));

	//declerations -> declerations decl
	g.AddProduction(AttrProd(declarations, Sentence({declarations, decl}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNodeVector* decl_list_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[0]));
		ASTNode* decl_node = std::get<ASTNode*>(args[1]);
		decl_list_node->add_child(decl_node);
		return decl_list_node;
	}));

	//decl -> func_full_assign
	g.AddProduction(AttrProd(decl, Sentence(func_full_assign), [](const std::vector<ElementType>& args) -> ElementType {
		AssignFuncNode* func_assign_node = static_cast<AssignFuncNode*>(std::get<ASTNode*>(args[0]));
		return func_assign_node;
	}));

	//decl -> type_node_decl
	g.AddProduction(AttrProd(decl, Sentence(type_node_decl), [](const std::vector<ElementType>& args) -> ElementType {
		TypeDeclNode* type_node_decl_node = static_cast<TypeDeclNode*>(std::get<ASTNode*>(args[0]));
		return type_node_decl_node;
	}));
	

	// // decl_list -> decl_list func_full_assign
	// g.AddProduction(AttrProd(decl_list, Sentence({decl_list, func_full_assign}), [](const std::vector<ElementType>& args) -> ElementType {
	// 	ASTNodeVector* decl_list_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[0]));
	// 	ASTNode* decl_node = std::get<ASTNode*>(args[1]);
	// 	decl_list_node->add_child(decl_node);
	// 	return decl_list_node;
	// }));
	// // decl -> type_node_decl
	// // Una declaración de tipo
	// g.AddProduction(AttrProd(decl, Sentence(type_node_decl), [](const std::vector<ElementType>& args) -> ElementType {
	// 	TypeDeclNode* type_node_decl_node = static_cast<TypeDeclNode*>(std::get<ASTNode*>(args[0]));
	// 	return type_node_decl_node;
	// }));

	// // decl -> func_full_assign
	// // Una asignación de función
	// g.AddProduction(AttrProd(decl, Sentence(func_full_assign), [](const std::vector<ElementType>& args) -> ElementType {
	// 	AssignFuncNode* func_assign_node = static_cast<AssignFuncNode*>(std::get<ASTNode*>(args[0]));
	// 	return func_assign_node;
	// }));

	// input -> lines_block
	// Un bloque de código como programa completo
	g.AddProduction(AttrProd(input, Sentence(lines_block), [](const std::vector<ElementType>& args) -> ElementType {
		BlockNode* lines_block_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[0])); 
		return (lines_block_node);
	}));

	// input -> line lines_block
	g.AddProduction(AttrProd(input, Sentence({line, lines_block}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* line_node = std::get<ASTNode*>(args[0]);
		BlockNode* lines_block_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[1]));
		lines_block_node->add_child(line_node);
		return lines_block_node;
	}));

	// --------- Bloques de código ---------
	// lines_block -> { lines }
	// Un bloque de código delimitado por llaves
	g.AddProduction(AttrProd(lines_block, Sentence({LKEY, lines, RKEY}), [](const std::vector<ElementType>& args) -> ElementType {
		BlockNode* lines_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[1]));
		return lines_node;
	}));

	// --------- Líneas de código ---------
	// lines -> ε
	// Un bloque vacío (sin líneas de código)
	g.AddProduction(AttrProd(lines, Sentence(epsilon), [](const std::vector<ElementType>& args) -> ElementType {
		int minLine = getMinLineFromArgs(args);
		return new BlockNode({}, minLine);
	}));
	
	// lines -> non_empty_lines
	// Un bloque con una o más líneas de código
	g.AddProduction(AttrProd(lines, Sentence( non_empty_lines ), [](const std::vector<ElementType>& args) -> ElementType {
		BlockNode* non_empty_lines_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[0]));
		return non_empty_lines_node;
	}));

	// --------- Secuencias de líneas no vacías ---------
	// non_empty_lines -> line
	// Una única línea de código
	g.AddProduction(AttrProd(non_empty_lines, Sentence( line ), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* node = std::get<ASTNode*>(args[0]);
		int minLine = getMinLineFromArgs(args);
		return new BlockNode({node}, minLine);
	}));
	
	// non_empty_lines -> non_empty_lines line
	// Una secuencia de líneas de código (recursión a izquierda)
	g.AddProduction(AttrProd(non_empty_lines, Sentence({non_empty_lines, line}), [](const std::vector<ElementType>& args) -> ElementType {
		BlockNode* n_e_l_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[0]));
		ASTNode* line_node = std::get<ASTNode*>(args[1]);

		n_e_l_node->add_child(line_node);
		return n_e_l_node;
	}));

	// --------- Líneas individuales de código ---------
	// line -> expr ;
	// Una expresión que termina con punto y coma
	g.AddProduction(AttrProd(line, Sentence({expr, SEMICOLON}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node = std::get<ASTNode*>(args[0]);
		return expr_node;
	}));
	
	// line -> func_def ;
	// Una declaración de función que termina con punto y coma
	g.AddProduction(AttrProd(line, Sentence({func_def, SEMICOLON}), [](const std::vector<ElementType>& args) -> ElementType {
		AssignFuncNode* func_assign_node = static_cast<AssignFuncNode*>(std::get<ASTNode*>(args[0]));
		return func_assign_node;
	}));
	
	// // line -> type_node_decl
	// // Una declaración de tipo (no requiere punto y coma)
	// g.AddProduction(AttrProd(line, Sentence( type_node_decl ), [](const std::vector<ElementType>& args) -> ElementType {
	// 	TypeDeclNode* type_node_decl_node = static_cast<TypeDeclNode*>(std::get<ASTNode*>(args[0]));
	// 	return type_node_decl_node;
	// }));

	// ====================== EXPRESIONES ======================
	/* 
	 * Las expresiones son la base del lenguaje HULK. Pueden ser operaciones aritméticas,
	 * expresiones booleanas, estructuras de control, strings, identificadores, llamadas a
	 * funciones, asignaciones let-in, condicionales if-else, acceso a miembros, asignaciones,
	 * y operaciones con tipo (@, @@, as).
	 */
	
	// expr -> arit_op
	// Expresión aritmética
	g.AddProduction(AttrProd(expr, Sentence( arit_op ), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* arit_op_node = std::get<ASTNode*>(args[0]);
		return arit_op_node;
	}));
	
	// expr -> bool_expr
	// Expresión booleana
	g.AddProduction(AttrProd(expr, Sentence( bool_expr ), [](const std::vector<ElementType>& args) -> ElementType {
		BoolExprNode* bool_expr_node = static_cast<BoolExprNode*>(std::get<ASTNode*>(args[0]));
		return bool_expr_node;
	}));
	
	// expr -> WHILE while_expr
	// Expresión de bucle while
	g.AddProduction(AttrProd(expr, Sentence({WHILE, while_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		WhileNode* while_expr_node = static_cast<WhileNode*>(std::get<ASTNode*>(args[1]));
		return while_expr_node;
	}));
	
	// expr -> STRING
	// Literal de cadena de texto
	g.AddProduction(AttrProd(expr, Sentence(STRING), [](const std::vector<ElementType>& args) -> ElementType {
		Token string_token = std::get<Token>(args[0]);
		return new StringNode(string_token.Lexeme(), string_token.Line());
	}));
	
	// expr -> id_expr
	// Identificador (variable)
	g.AddProduction(AttrProd(expr, Sentence(id_expr), [](const std::vector<ElementType>& args) -> ElementType {
		IDNode* id_expr_node = static_cast<IDNode*>(std::get<ASTNode*>(args[0]));
		return id_expr_node;
	}));
	
	// expr -> func_call
	// Llamada a función
	g.AddProduction(AttrProd(expr, Sentence(func_call), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* func_call_node = std::get<ASTNode*>(args[0]);
		return func_call_node;
	}));
	
	// expr -> let_exp
	// Expresión let-in para declaraciones locales
	g.AddProduction(AttrProd(expr, Sentence(let_exp), [](const std::vector<ElementType>& args) -> ElementType {
		LetAssign* let_assign_node = static_cast<LetAssign*>(std::get<ASTNode*>(args[0]));
		return let_assign_node;
	}));
	
	// expr -> IF conditional
	// Expresión condicional if-else/elif
	g.AddProduction(AttrProd(expr, Sentence({IF, conditional}), [](const std::vector<ElementType>& args) -> ElementType {
		Conditional* conditional_node = static_cast<Conditional*>(std::get<ASTNode*>(args[1]));
		return conditional_node;
	}));
	
	// expr -> member_access_expr
	// Acceso a miembros de objetos
	g.AddProduction(AttrProd(expr, Sentence(member_access_expr), [](const std::vector<ElementType>& args) -> ElementType {
		AccessNode* member_access_expr_node = static_cast<AccessNode*>(std::get<ASTNode*>(args[0]));
		return member_access_expr_node;
	}));
	// expr -> expr := expr
	// Asignación destructiva
	g.AddProduction(AttrProd(expr, Sentence({expr, ASS_DES, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node1 = std::get<ASTNode*>(args[0]);
		ASTNode* expr_node2 = std::get<ASTNode*>(args[2]);
		int minLine = getMinLineFromArgs(args);
		return new BinOpNode(expr_node1, ":=", expr_node2, minLine);
	}));
	
	// expr -> expr = expr
	// Asignación normal
	g.AddProduction(AttrProd(expr, Sentence({expr, EQUAL, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node1 = std::get<ASTNode*>(args[0]);
		ASTNode* expr_node2 = std::get<ASTNode*>(args[2]);
		int minLine = getMinLineFromArgs(args);
		return new BinOpNode(expr_node1, "=", expr_node2, minLine);
	}));
	
	// expr -> expr @ expr
	// Operador de composición simple
	g.AddProduction(AttrProd(expr, Sentence({expr, ARROBA_, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node1 = std::get<ASTNode*>(args[0]);
		ASTNode* expr_node2 = std::get<ASTNode*>(args[2]);
		int minLine = getMinLineFromArgs(args);
		return new BinOpNode(expr_node1, "@", expr_node2, minLine);
	}));
	
	// expr -> expr @@ expr
	// Operador de composición doble
	g.AddProduction(AttrProd(expr, Sentence({expr, D_ARROBA_, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node1 = std::get<ASTNode*>(args[0]);
		ASTNode* expr_node2 = std::get<ASTNode*>(args[2]);
		int minLine = getMinLineFromArgs(args);
		return new BinOpNode(expr_node1, "@@", expr_node2, minLine);
	}));
	
	// // expr -> expr as ID
	// // Conversión explícita de tipo (cast)
	// g.AddProduction(AttrProd(expr, Sentence({expr, AS_, ID_}), [](const std::vector<ElementType>& args) -> ElementType {
	// 	ASTNode* expr_node = std::get<ASTNode*>(args[0]);
	// 	std::string type_name = std::get<std::string>(args[2]);
	// 	int minLine = getMinLineFromArgs(args);
	// 	return new TypeCastNode(expr_node, type_name, minLine);
	// }));

	// ====================== DEFINICIONES DE FUNCIONES ======================
	/* 
	 * Las funciones en HULK pueden definirse con expresiones inline (=>) o con bloques de código.
	 * También pueden tener un tipo de retorno especificado o no.
	 */
	
	// func_def -> function ID ( args_list ) => expr
	// Función con expresión inline sin tipo de retorno
	g.AddProduction(AttrProd(func_def, Sentence({FUNCTION_, ID_, LPARENT, args_list, RPARENT, INLINE, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		// std::string func_name = std::get<std::string>(args[1]);
		Token func_name = std::get<Token>(args[1]);
		ArgsList* args_list_node = static_cast<ArgsList*>(std::get<ASTNode*>(args[3]));
		ASTNode* expr_node = std::get<ASTNode*>(args[6]);
		int minLine = getMinLineFromArgs(args);
		return new AssignFuncNode(new IDNode(func_name.Lexeme(), func_name.Line()), args_list_node, expr_node, minLine);
	}));
	
	// func_def -> function ID ( args_list ) : TYPE_ID => expr
	// Función con expresión inline con tipo de retorno especificado
	g.AddProduction(AttrProd(func_def, Sentence({FUNCTION_, ID_, LPARENT, args_list, RPARENT, TWO_POINTS, TYPE_ID, INLINE, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		Token func_name = std::get<Token>(args[1]);
		ArgsList* args_list_node = static_cast<ArgsList*>(std::get<ASTNode*>(args[3]));
		std::string return_type = std::get<std::string>(args[6]);
		ASTNode* expr_node = std::get<ASTNode*>(args[8]);
		int minLine = getMinLineFromArgs(args);
		return new AssignFuncNode(new IDNode(func_name.Lexeme(), func_name.Line()), args_list_node, expr_node, return_type, minLine);
	}));
	
	// func_full_assign -> function ID ( args_list ) { lines }
	// Función con bloque de código sin tipo de retorno
	g.AddProduction(AttrProd(func_full_assign, Sentence({FUNCTION_, ID_, LPARENT, args_list, RPARENT, LKEY, lines, RKEY}), [](const std::vector<ElementType>& args) -> ElementType {
		Token func_name = std::get<Token>(args[1]);
		ArgsList* args_list_node = static_cast<ArgsList*>(std::get<ASTNode*>(args[3]));
		BlockNode* lines_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[6]));
		int minLine = getMinLineFromArgs(args);
		return new AssignFuncNode(new IDNode(func_name.Lexeme(), func_name.Line()), args_list_node, lines_node, minLine);
	}));
	
	// func_full_assign -> function ID ( args_list ) : TYPE_ID { lines }
	// Función con bloque de código con tipo de retorno especificado
	g.AddProduction(AttrProd(func_full_assign, Sentence({FUNCTION_, ID_, LPARENT, args_list, RPARENT, TWO_POINTS, TYPE_ID, LKEY, lines, RKEY}), [](const std::vector<ElementType>& args) -> ElementType {
		Token func_name = std::get<Token>(args[1]);
		ArgsList* args_list_node = static_cast<ArgsList*>(std::get<ASTNode*>(args[3]));
		Token return_type = std::get<Token>(args[6]);
		BlockNode* lines_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[8]));
		int minLine = getMinLineFromArgs(args);
		return new AssignFuncNode(new IDNode(func_name.Lexeme(), func_name.Line()), args_list_node, lines_node, return_type.Lexeme(), minLine);
	}));

	// ====================== LISTAS DE ARGUMENTOS ======================
	/* 
	 * Las listas de argumentos se utilizan en las declaraciones de funciones y llamadas.
	 * Pueden estar vacías o contener uno o más identificadores separados por comas.
	 */
	
	// args_list -> ε
	// Lista de argumentos vacía
	g.AddProduction(AttrProd(args_list, Sentence(epsilon), [](const std::vector<ElementType>& args) -> ElementType {
		int minLine = getMinLineFromArgs(args);
		return new ArgsList({}, minLine);
	}));
	
	// args_list -> id_expr
	// Lista con un único argumento
	g.AddProduction(AttrProd(args_list, Sentence(id_expr), [](const std::vector<ElementType>& args) -> ElementType {
		IDNode* id_expr_node = static_cast<IDNode*>(std::get<ASTNode*>(args[0]));
		int minLine = getMinLineFromArgs(args);
		return new ArgsList({id_expr_node}, minLine);
	}));
	
	// args_list -> args_list , id_expr
	// Lista con múltiples argumentos (recursión a izquierda)
	g.AddProduction(AttrProd(args_list, Sentence({args_list, COLON, id_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ArgsList* args_list_node = static_cast<ArgsList*>(std::get<ASTNode*>(args[0]));
		IDNode* id_expr_node = static_cast<IDNode*>(std::get<ASTNode*>(args[2]));
		args_list_node->add_child(id_expr_node);
		return args_list_node;
	}));

	// ====================== IDENTIFICADORES ======================
	/*
	 * Los identificadores pueden ser simples o tener un tipo asociado
	 */
	
	// id_expr -> ID : TYPE_ID
	// Identificador con tipo explícito
	g.AddProduction(AttrProd(id_expr, Sentence({ID_, TWO_POINTS, TYPE_ID}), [](const std::vector<ElementType>& args) -> ElementType {
		Token id_token = std::get<Token>(args[0]);
		Token type_token = std::get<Token>(args[2]);
		int minLine = getMinLineFromArgs(args);
		return new IDNode(id_token.Lexeme(), type_token.Lexeme(), minLine);
	}));
	
	// id_expr -> ID
	// Identificador simple
	g.AddProduction(AttrProd(id_expr, Sentence(ID_), [](const std::vector<ElementType>& args) -> ElementType {
		Token id_token = std::get<Token>(args[0]);
		return new IDNode(id_token.Lexeme(), id_token.Line());
	}));

	// ====================== ASIGNACIONES LET-IN ======================
	/*
	 * Las expresiones let-in permiten definir variables locales con su propio ámbito.
	 * La asignación let-in incluye una lista de variables (var_assign_list) seguidas de
	 * una expresión o un bloque de código que puede usar esas variables. Las variables
	 * definidas sólo son válidas dentro del contexto del let-in.
	 */
	
	// let_exp -> LET var_assign_list IN expr
	// Asignación let-in con una expresión como cuerpo
	g.AddProduction(AttrProd(let_exp, Sentence({LET, var_assign_list, IN, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		VarAssignList* var_assign_list_node = static_cast<VarAssignList*>(std::get<ASTNode*>(args[1]));
		ASTNode* expr_node = std::get<ASTNode*>(args[3]);
		return new LetAssign(var_assign_list_node->assigns, expr_node, getMinLineFromArgs(args));
	}));
	// let_exp -> LET var_assign_list IN lines_block
	// Asignación let-in con un bloque de código como cuerpo
	g.AddProduction(AttrProd(let_exp, Sentence({LET, var_assign_list, IN, lines_block}), [](const std::vector<ElementType>& args) -> ElementType {
		VarAssignList* var_assign_list_node = static_cast<VarAssignList*>(std::get<ASTNode*>(args[1]));
		BlockNode* lines_block_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[3]));
		int minLine = getMinLineFromArgs(args);
		return new LetAssign(var_assign_list_node->assigns, lines_block_node, minLine);
	}));

	// --------- Lista de asignaciones de variables ---------
	// var_assign_list -> id_expr = expr
	// Asignación básica de una variable con una expresión
	g.AddProduction(AttrProd(var_assign_list, Sentence({id_expr, EQUAL, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		IDNode* id_expr_node = static_cast<IDNode*>(std::get<ASTNode*>(args[0]));
		ASTNode* expr_node = std::get<ASTNode*>(args[2]);
		int minLine = getMinLineFromArgs(args);
		return new VarAssignList({new VarAssign(id_expr_node, expr_node, minLine)}, minLine);
	}));
	// var_assign_list -> id_expr = expr as TYPE_ID
	// Asignación de una variable con una expresión y especificación de tipo
	g.AddProduction(AttrProd(var_assign_list, Sentence({id_expr, EQUAL, expr, AS_, TYPE_ID}), [](const std::vector<ElementType>& args) -> ElementType {
		IDNode* id_expr_node = static_cast<IDNode*>(std::get<ASTNode*>(args[0]));
		ASTNode* expr_node = std::get<ASTNode*>(args[2]);
		std::string type_name = std::get<std::string>(args[4]);
		int minLine = getMinLineFromArgs(args);
		return new VarAssignList({new VarAssign(id_expr_node, expr_node, type_name, minLine)}, minLine);
	}));
	// var_assign_list -> id_expr = new_expr
	// Asignación de una variable con una expresión de creación de objeto
	g.AddProduction(AttrProd(var_assign_list, Sentence({id_expr, EQUAL, new_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		IDNode* id_expr_node = static_cast<IDNode*>(std::get<ASTNode*>(args[0]));
		NewTypeNode* new_expr_node = static_cast<NewTypeNode*>(std::get<ASTNode*>(args[2]));
		int minLine = getMinLineFromArgs(args);
		return new VarAssignList({new VarAssign(id_expr_node, new_expr_node, minLine)}, minLine);
	}));
	// var_assign_list -> var_assign_list , id_expr = expr
	// Lista de asignaciones de variables con una expresión adicional
	g.AddProduction(AttrProd(var_assign_list, Sentence({var_assign_list, COLON, id_expr, EQUAL, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		VarAssignList* var_assign_list_node = static_cast<VarAssignList*>(std::get<ASTNode*>(args[0]));
		IDNode* id_expr_node = static_cast<IDNode*>(std::get<ASTNode*>(args[2]));
		ASTNode* expr_node = std::get<ASTNode*>(args[4]);
		int minLine = getMinLineFromArgs(args);
		var_assign_list_node->add_child(new VarAssign(id_expr_node, expr_node, minLine));
		return var_assign_list_node;
	}));
	// var_assign_list -> var_assign_list , id_expr = expr as TYPE_ID
	// Lista de asignaciones de variables con una expresión adicional y tipo específico
	g.AddProduction(AttrProd(var_assign_list, Sentence({var_assign_list, COLON, id_expr, EQUAL, expr, AS_, TYPE_ID}), [](const std::vector<ElementType>& args) -> ElementType {
		VarAssignList* var_assign_list_node = static_cast<VarAssignList*>(std::get<ASTNode*>(args[0]));
		IDNode* id_expr_node = static_cast<IDNode*>(std::get<ASTNode*>(args[2]));
		ASTNode* expr_node = std::get<ASTNode*>(args[4]);
		std::string type_name = std::get<std::string>(args[6]);
		int minLine = getMinLineFromArgs(args);
		var_assign_list_node->add_child(new VarAssign(id_expr_node, expr_node, type_name, minLine));
		return var_assign_list_node;
	}));
	// var_assign_list -> var_assign_list , id_expr = new_expr
	// Lista de asignaciones de variables con una instanciación de objeto
	g.AddProduction(AttrProd(var_assign_list, Sentence({var_assign_list, COLON, id_expr, EQUAL, new_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		VarAssignList* var_assign_list_node = static_cast<VarAssignList*>(std::get<ASTNode*>(args[0]));
		IDNode* id_expr_node = static_cast<IDNode*>(std::get<ASTNode*>(args[2]));
		NewTypeNode* new_expr_node = static_cast<NewTypeNode*>(std::get<ASTNode*>(args[4]));
		int minLine = getMinLineFromArgs(args);
		var_assign_list_node->add_child(new VarAssign(id_expr_node, new_expr_node, minLine));
		return var_assign_list_node;
	}));

	// ====================== INSTANCIACIÓN DE OBJETOS ======================
	/*
	 * Las expresiones new_expr permiten crear instancias de tipos definidos por el usuario.
	 * Se pueden pasar argumentos al constructor mediante expr_list.
	 */
	
	// new_expr -> new ID ( expr_list )
	// Creación de un nuevo objeto de un tipo específico con argumentos de constructor opcionales
	g.AddProduction(AttrProd(new_expr, Sentence({NEW, ID_, LPARENT, expr_list, RPARENT}), [](const std::vector<ElementType>& args) -> ElementType {
		Token type_name = std::get<Token>(args[1]);
		ASTNodeVector* expr_list_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[3]));
		int minLine = getMinLineFromArgs(args);
		return new NewTypeNode(type_name.Lexeme(), expr_list_node->children, minLine);
	}));

	// --------- Listas de expresiones ---------
	// expr_list -> ε
	// Lista vacía de expresiones
	g.AddProduction(AttrProd(expr_list, Sentence(epsilon), [](const std::vector<ElementType>& args) -> ElementType {
		int line = getMinLineFromArgs(args);
		return new ASTNodeVector({}, line);
	}));
	// expr_list -> expr
	// Lista con una única expresión
	g.AddProduction(AttrProd(expr_list, Sentence(expr), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node = std::get<ASTNode*>(args[0]);
		int minLine = getMinLineFromArgs(args);
		return new ASTNodeVector({expr_node}, minLine);
	}));
	// expr_list -> new_expr
	// Lista con una única expresión de creación de objeto
	g.AddProduction(AttrProd(expr_list, Sentence(new_expr), [](const std::vector<ElementType>& args) -> ElementType {
		NewTypeNode* new_expr_node = static_cast<NewTypeNode*>(std::get<ASTNode*>(args[0]));
		return new ASTNodeVector({new_expr_node}, new_expr_node->line + 1);
	}));
	// expr_list -> expr_list , expr
	// Lista de expresiones con una expresión adicional
	g.AddProduction(AttrProd(expr_list, Sentence({expr_list, COLON, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNodeVector* expr_list_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[0]));
		ASTNode* expr_node = std::get<ASTNode*>(args[2]);
		expr_list_node->add_child(expr_node);
		return expr_list_node;
	}));
	// expr_list -> expr_list , new_expr
	// Lista de expresiones con una expresión de creación de objeto adicional
	g.AddProduction(AttrProd(expr_list, Sentence({expr_list, COLON, new_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNodeVector* expr_list_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[0]));
		NewTypeNode* new_expr_node = static_cast<NewTypeNode*>(std::get<ASTNode*>(args[2]));
		expr_list_node->add_child(new_expr_node);
		return expr_list_node;
	}));

	// ====================== LLAMADAS A FUNCIONES ======================
	/*
	 * Las llamadas a funciones permiten invocar funciones definidas previamente
	 * pasando opcionalmente argumentos como expresiones.
	 */
	
	// func_call -> ID ( expr_list )
	// Llamada a una función con argumentos opcionales
	g.AddProduction(AttrProd(func_call, Sentence({ID_, LPARENT, expr_list, RPARENT}), [](const std::vector<ElementType>& args) -> ElementType {
		Token func_name = std::get<Token>(args[0]);
		ASTNodeVector* expr_list_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[2]));
		int minLine = getMinLineFromArgs(args);
		return new FunctionCallNode(func_name.Lexeme(), expr_list_node, minLine);
	}));

	// ====================== OPERACIONES ARITMÉTICAS ======================
	/*
	 * Las operaciones aritméticas incluyen números, operaciones binarias (+, -, *, /, ^) y 
	 * operaciones unarias (-). Se respeta la precedencia matemática estándar.
	 */
	// arit_op -> arit_op + term
	// Operación de suma
	g.AddProduction(AttrProd(arit_op, Sentence({arit_op, PLUS, term}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* left = std::get<ASTNode*>(args[0]);
		ASTNode* right = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(left, right);
		return new BinOpNode(left, "+", right, line);
	}));
	// arit_op -> arit_op - term
	g.AddProduction(AttrProd(arit_op, Sentence({arit_op, MINUS, term}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* left = std::get<ASTNode*>(args[0]);
		ASTNode* right = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(left, right);
		return new BinOpNode(left, "-", right, line);
	}));
	// term -> term * factor
	// Operación de multiplicación
	g.AddProduction(AttrProd(term, Sentence({term, TIMES, factor}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* left = std::get<ASTNode*>(args[0]);
		ASTNode* right = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(left, right);
		return new BinOpNode(left, "*", right, line);
	}));
	// term -> term / factor
	g.AddProduction(AttrProd(term, Sentence({term, DIV, factor}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* left = std::get<ASTNode*>(args[0]);
		ASTNode* right = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(left, right);
		return new BinOpNode(left, "/", right, line);
	}));
	// term -> term % factor
	g.AddProduction(AttrProd(term, Sentence({term, MOD, factor}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* left = std::get<ASTNode*>(args[0]);
		ASTNode* right = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(left, right);
		return new BinOpNode(left, "%", right, line);
	}));
	// term -> factor
	g.AddProduction(AttrProd(term, Sentence(factor), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* factor_node = std::get<ASTNode*>(args[0]);
		return factor_node;
	}));
	// factor -> sign ^ factor
	// Operación de potencia
	g.AddProduction(AttrProd(factor, Sentence({sign, POW, factor}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* base = std::get<ASTNode*>(args[0]);
		ASTNode* exponent = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(base, exponent);
		return new BinOpNode(base, "^", exponent, line);
	}));
	// factor -> sign
	g.AddProduction(AttrProd(factor, Sentence(sign), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* sign_node = std::get<ASTNode*>(args[0]);
		return sign_node;
	}));
	
	// // arit_op -> NUMBER
	// // Literal numérico
	// g.AddProduction(AttrProd(arit_op, Sentence(NUMBER), [](const std::vector<ElementType>& args) -> ElementType {
	// 	Token number_token = std::get<Token>(args[0]);
	// 	float number = std::stof(number_token.Lexeme());
	// 	int line = number_token.Line();
	// 	return new FloatNode(number, line);
	// }));
	// // arit_op -> ( expr )
	// // Expresión entre paréntesis para agrupar o cambiar precedencia
	// g.AddProduction(AttrProd(arit_op, Sentence({LPARENT, expr, RPARENT}), [](const std::vector<ElementType>& args) -> ElementType {
	// 	ASTNode* expr_node = std::get<ASTNode*>(args[1]);
	// 	return expr_node;
	// }));
	// sign -> atom
	// Un término atómico (número, identificador, etc.)
	g.AddProduction(AttrProd(sign, Sentence(atom), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* atom_node = std::get<ASTNode*>(args[0]);
		return atom_node;
	}));
	// sign -> - atom
	// Operación unaria de negación
	g.AddProduction(AttrProd(sign, Sentence({MINUS, atom}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* atom_node = std::get<ASTNode*>(args[1]);
		int minLine = getMinLineFromArgs(args);
		return new UnaryOpNode("-", atom_node, minLine);
	}));
	// atom -> NUMBER
	// Literal numérico (entero o flotante)
	g.AddProduction(AttrProd(atom, Sentence(NUMBER), [](const std::vector<ElementType>& args) -> ElementType {
		Token number_token = std::get<Token>(args[0]);
		float number = std::stof(number_token.Lexeme());
		int line = number_token.Line();
		return new FloatNode(number, line);
	}));
	// // atom -> STRING
	// // Literal de cadena de texto
	// g.AddProduction(AttrProd(atom, Sentence(STRING), [](const std::vector<ElementType>& args) -> ElementType {
	// 	Token string_token = std::get<Token>(args[0]);
	// 	std::string value = string_token.Lexeme();
	// 	int line = string_token.Line();
	// 	return new StringNode(value, line);
	// }));
	// atom -> ID
	// Identificador (variable)
	g.AddProduction(AttrProd(atom, Sentence(ID_), [](const std::vector<ElementType>& args) -> ElementType {
		Token id_token = std::get<Token>(args[0]);
		return new IDNode(id_token.Lexeme(), id_token.Line());
	}));
	// atom -> ( expr )
	// Expresión entre paréntesis para agrupar o cambiar precedencia
	g.AddProduction(AttrProd(atom, Sentence({LPARENT, expr, RPARENT}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node = std::get<ASTNode*>(args[1]);
		return expr_node;
	}));
	// atom -> func_call
	// Llamada a función
	g.AddProduction(AttrProd(atom, Sentence(func_call), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* func_call_node = std::get<ASTNode*>(args[0]);
		return func_call_node;
	}));
	// atom -> member_access_expr
	// Acceso a miembros (atributos o métodos)
	g.AddProduction(AttrProd(atom, Sentence(member_access_expr), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* member_access_node = std::get<ASTNode*>(args[0]);
		return member_access_node;
	}));

	// ====================== EXPRESIONES BOOLEANAS ======================
	/*
	 * Las expresiones booleanas incluyen valores booleanos (true/false), operaciones de comparación,
	 * operadores lógicos (AND, OR, NOT) y verificación de tipo (is).
	 */
	
	// bool_expr -> BOOLEAN
	// Literal booleano (true o false)
	g.AddProduction(AttrProd(bool_expr, Sentence(BOOLEAN), [](const std::vector<ElementType>& args) -> ElementType {
		Token boolean_token = std::get<Token>(args[0]);
		bool value = (boolean_token.Lexeme() == "true");
		int line = boolean_token.Line();
		return new BoolExprNode(new BoolNode(value, line), line);
	}));
	// bool_expr -> expr >= expr
	g.AddProduction(AttrProd(bool_expr, Sentence({expr, GR_EQ_, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node1 = std::get<ASTNode*>(args[0]);
		ASTNode* expr_node2 = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(expr_node1, expr_node2);
		return new BoolExprNode(new BinOpNode(expr_node1, ">=", expr_node2, line), line);
	}));
	// bool_expr -> expr > expr
	// Operador de mayor que
	g.AddProduction(AttrProd(bool_expr, Sentence({expr, GR_, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node1 = std::get<ASTNode*>(args[0]);
		ASTNode* expr_node2 = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(expr_node1, expr_node2);
		return new BoolExprNode(new BinOpNode(expr_node1, ">", expr_node2, line), line);
	}));
	// bool_expr -> expr < expr
	g.AddProduction(AttrProd(bool_expr, Sentence({expr, LESS_EQ_, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node1 = std::get<ASTNode*>(args[0]);
		ASTNode* expr_node2 = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(expr_node1, expr_node2);
		return new BoolExprNode(new BinOpNode(expr_node1, "<=", expr_node2, line), line);
	}));
	// bool_expr -> expr < expr
	g.AddProduction(AttrProd(bool_expr, Sentence({expr, LESS_, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node1 = std::get<ASTNode*>(args[0]);
		ASTNode* expr_node2 = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromArgs(args);
		return new BoolExprNode(new BinOpNode(expr_node1, "<", expr_node2, line), line);
	}));
	// bool_expr -> expr == expr
	g.AddProduction(AttrProd(bool_expr, Sentence({expr, EQ_, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node1 = std::get<ASTNode*>(args[0]);
		ASTNode* expr_node2 = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(expr_node1, expr_node2);
		return new BoolExprNode(new BinOpNode(expr_node1, "==", expr_node2, line), line);
	}));
	// bool_expr -> expr != expr
	// Operador de desigualdad
	g.AddProduction(AttrProd(bool_expr, Sentence({expr, DISTINCT_, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node1 = std::get<ASTNode*>(args[0]);
		ASTNode* expr_node2 = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(expr_node1, expr_node2);
		return new BoolExprNode(new BinOpNode(expr_node1, "!=", expr_node2, line), line);
	}));
	// bool_expr -> expr AND expr
	// Operador lógico AND
	g.AddProduction(AttrProd(bool_expr, Sentence({expr, AND_, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node1 = std::get<ASTNode*>(args[0]);
		ASTNode* expr_node2 = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(expr_node1, expr_node2);
		return new BoolExprNode(new BinOpNode(expr_node1, "&", expr_node2, line), line);
	}));
	// bool_expr -> expr OR expr
	// Operador lógico OR
	g.AddProduction(AttrProd(bool_expr, Sentence({expr, OR_, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node1 = std::get<ASTNode*>(args[0]);
		ASTNode* expr_node2 = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(expr_node1, expr_node2);
		return new BoolExprNode(new BinOpNode(expr_node1, "|", expr_node2, line), line);
	}));
	// bool_expr -> NOT expr
	g.AddProduction(AttrProd(bool_expr, Sentence({NOT_, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node = std::get<ASTNode*>(args[1]);
		return new BoolExprNode(new UnaryOpNode("!", expr_node, expr_node->line), expr_node->line);
	}));
	// bool_expr -> expr IS TYPE_ID
	g.AddProduction(AttrProd(bool_expr, Sentence({id_expr, IS_, TYPE_ID}), [](const std::vector<ElementType>& args) -> ElementType {
		IDNode* id_expr_node = static_cast<IDNode*>(std::get<ASTNode*>(args[0]));
		Token type_id_token = std::get<Token>(args[2]);
		int minline = getMinLineFromArgs(args);
		return new BoolExprNode(new BinOpNode(id_expr_node, "is", new IDNode(type_id_token.Lexeme(), type_id_token.Line()), minline), minline);
	}));
	// bool_expr -> func_call IS TYPE_ID
	g.AddProduction(AttrProd(bool_expr, Sentence({func_call, IS_, TYPE_ID}), [](const std::vector<ElementType>& args) -> ElementType {
		FunctionCallNode* func_call_node = static_cast<FunctionCallNode*>(std::get<ASTNode*>(args[0]));
		Token type_id_token = std::get<Token>(args[2]);
		int line = std::min(func_call_node->line, type_id_token.Line());
		return new BoolExprNode(new BinOpNode(func_call_node, "is", new IDNode(type_id_token.Lexeme(), type_id_token.Line()), line), line);
	}));

	// 	return new BoolExprNode(new BinOpNode(func_call_node, "is", new IDNode(type_name, 0), 0), 0);
	// }));

	// ====================== EXPRESIONES CONDICIONALES ======================
	/*
	 * Las expresiones condicionales (if-else/elif) evalúan una condición booleana y
	 * ejecutan uno u otro bloque de código dependiendo del resultado.
	 */
	
	// conditional -> ( bool_expr ) expr else expr
	// Condicional simple con expresiones
	g.AddProduction(AttrProd(conditional, Sentence({LPARENT, bool_expr, RPARENT, expr, ELSE, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		BoolExprNode* bool_expr_node = static_cast<BoolExprNode*>(std::get<ASTNode*>(args[1]));
		ASTNode* expr_node1 = std::get<ASTNode*>(args[3]);
		ASTNode* expr_node2 = std::get<ASTNode*>(args[5]);
		int minLine = getMinLineFromArgs(args);
		return new Conditional(bool_expr_node, expr_node1, expr_node2, minLine);
	}));
	// conditional -> ( bool_expr ) lines_block else expr
	// Condicional con bloque para la parte verdadera y expresión para la parte falsa
	g.AddProduction(AttrProd(conditional, Sentence({LPARENT, bool_expr, RPARENT, lines_block, ELSE, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		BoolExprNode* bool_expr_node = static_cast<BoolExprNode*>(std::get<ASTNode*>(args[1]));
		BlockNode* lines_block_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[3]));
		ASTNode* expr_node = std::get<ASTNode*>(args[5]);
		int minLine = getMinLineFromArgs(args);
		return new Conditional(bool_expr_node, lines_block_node, expr_node, minLine);
	}));
	// conditional -> ( bool_expr ) expr else lines_block
	g.AddProduction(AttrProd(conditional, Sentence({LPARENT, bool_expr, RPARENT, expr, ELSE, lines_block}), [](const std::vector<ElementType>& args) -> ElementType {
		BoolExprNode* bool_expr_node = static_cast<BoolExprNode*>(std::get<ASTNode*>(args[1]));
		ASTNode* expr_node = std::get<ASTNode*>(args[3]);
		BlockNode* lines_block_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[5]));
		return new Conditional(bool_expr_node, expr_node, lines_block_node, 0);
	}));
	// conditional -> ( bool_expr ) lines_block else lines_block
	g.AddProduction(AttrProd(conditional, Sentence({LPARENT, bool_expr, RPARENT, lines_block, ELSE, lines_block}), [](const std::vector<ElementType>& args) -> ElementType {
		BoolExprNode* bool_expr_node = static_cast<BoolExprNode*>(std::get<ASTNode*>(args[1]));
		BlockNode* lines_block_node1 = static_cast<BlockNode*>(std::get<ASTNode*>(args[3]));
		BlockNode* lines_block_node2 = static_cast<BlockNode*>(std::get<ASTNode*>(args[5]));
		return new Conditional(bool_expr_node, lines_block_node1, lines_block_node2, 0);
	}));
	// conditional -> ( bool_expr ) expr else expr
	g.AddProduction(AttrProd(conditional, Sentence({LPARENT, bool_expr, RPARENT, expr, ELIF, conditional}), [](const std::vector<ElementType>& args) -> ElementType {
		BoolExprNode* bool_expr_node = static_cast<BoolExprNode*>(std::get<ASTNode*>(args[1]));
		ASTNode* expr_node = std::get<ASTNode*>(args[3]);
		Conditional* conditional_node = static_cast<Conditional*>(std::get<ASTNode*>(args[5]));
		return new Conditional(bool_expr_node, expr_node, conditional_node, 0);
	}));
	// conditional -> ( bool_expr ) lines_block else lines_block
	g.AddProduction(AttrProd(conditional, Sentence({LPARENT, bool_expr, RPARENT, lines_block, ELIF, conditional}), [](const std::vector<ElementType>& args) -> ElementType {
		BoolExprNode* bool_expr_node = static_cast<BoolExprNode*>(std::get<ASTNode*>(args[1]));
		BlockNode* lines_block_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[3]));
		Conditional* conditional_node = static_cast<Conditional*>(std::get<ASTNode*>(args[5]));
		return new Conditional(bool_expr_node, lines_block_node, conditional_node, 0);
	}));

	// ====================== BUCLES WHILE ======================
	/*
	 * Los bucles while ejecutan repetidamente un bloque de código mientras
	 * una condición booleana sea verdadera.
	 */
	
	// while_expr -> ( bool_expr ) lines_block
	// Bucle while con un bloque de código
	g.AddProduction(AttrProd(while_expr, Sentence({LPARENT, bool_expr, RPARENT, lines_block}), [](const std::vector<ElementType>& args) -> ElementType {
		BoolExprNode* bool_expr_node = static_cast<BoolExprNode*>(std::get<ASTNode*>(args[1]));
		BlockNode* lines_block_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[3]));
		int minLine = getMinLineFromArgs(args);
		return new WhileNode(bool_expr_node, lines_block_node, minLine);
	}));
	// while_expr -> ( bool_expr ) expr
	// Bucle while con una expresión única
	g.AddProduction(AttrProd(while_expr, Sentence({LPARENT, bool_expr, RPARENT, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		BoolExprNode* bool_expr_node = static_cast<BoolExprNode*>(std::get<ASTNode*>(args[1]));
		ASTNode* expr_node = std::get<ASTNode*>(args[3]);
		int minLine = getMinLineFromArgs(args);
		return new WhileNode(bool_expr_node, expr_node, minLine);
	}));
	// ====================== DECLARACIONES DE TIPOS ======================
	/*
	 * Las declaraciones de tipos definen nuevos tipos con atributos y métodos.
	 * Los tipos pueden heredar de otros tipos y tener constructores con parámetros.
	 */
	
	// type_node_decl -> type TYPE_ID { type_body_elements }
	// Declaración simple de tipo
	g.AddProduction(AttrProd(type_node_decl, Sentence({TYPE_, TYPE_ID, LKEY, type_body_elements, RKEY}), [](const std::vector<ElementType>& args) -> ElementType {
		Token type_name = std::get<Token>(args[1]);
		ASTNodeVector* type_body_elements_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[3]));
		int minLine = getMinLineFromArgs(args);
		return new TypeDeclNode(new IDNode(type_name.Lexeme(), type_name.Line()), new ArgsList({}, minLine), type_body_elements_node->children, minLine);
	}));
	// type_node_decl -> type TYPE_ID inherits TYPE_ID { type_body_elements }
	// Declaración de tipo con herencia
	g.AddProduction(AttrProd(type_node_decl, Sentence({TYPE_, TYPE_ID, INHERITS, TYPE_ID, LKEY, type_body_elements, RKEY}), [](const std::vector<ElementType>& args) -> ElementType {
		Token type_name = std::get<Token>(args[1]);
		Token inherits_name = std::get<Token>(args[3]);
		ASTNodeVector* type_body_elements_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[5]));
		int minLine = getMinLineFromArgs(args);
		return new TypeDeclNode(new IDNode(type_name.Lexeme(), type_name.Line()), new ArgsList({}, minLine), type_body_elements_node->children, {inherits_name.Lexeme()}, minLine);
	}));
	// type_node_decl -> type TYPE_ID inherits TYPE_ID ( args_list ) { type_body_elements }
	// Declaración de tipo con herencia y argumentos para el constructor del padre
	g.AddProduction(AttrProd(type_node_decl, Sentence({TYPE_, TYPE_ID, INHERITS, TYPE_ID, LPARENT, args_list, RPARENT, LKEY, type_body_elements, RKEY}), [](const std::vector<ElementType>& args) -> ElementType {
		Token type_name = std::get<Token>(args[1]);
		Token inherits_name = std::get<Token>(args[3]);
		ArgsList* args_list_node = static_cast<ArgsList*>(std::get<ASTNode*>(args[5]));
		ASTNodeVector* type_body_elements_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[8]));
		int minLine = getMinLineFromArgs(args);
		return new TypeDeclNode(new IDNode(type_name.Lexeme(), type_name.Line()), new ArgsList({}, minLine), type_body_elements_node->children, {inherits_name.Lexeme()}, args_list_node, minLine);
	}));
	// type_node_decl -> type TYPE_ID ( args_list ) { type_body_elements }
	// Declaración de tipo con parámetros de constructor
	g.AddProduction(AttrProd(type_node_decl, Sentence({TYPE_, TYPE_ID, LPARENT, args_list, RPARENT, LKEY, type_body_elements, RKEY}), [](const std::vector<ElementType>& args) -> ElementType {
		Token type_name = std::get<Token>(args[1]);
		ArgsList* args_list_node = static_cast<ArgsList*>(std::get<ASTNode*>(args[3]));
		ASTNodeVector* type_body_elements_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[6]));
		int minLine = getMinLineFromArgs(args);
		return new TypeDeclNode(new IDNode(type_name.Lexeme(), type_name.Line()), args_list_node, type_body_elements_node->children, minLine);
	}));
	// type_node_decl -> type TYPE_ID ( args_list ) inherits TYPE_ID { type_body_elements }
	// Declaración de tipo con parámetros y herencia
	g.AddProduction(AttrProd(type_node_decl, Sentence({TYPE_, TYPE_ID, LPARENT, args_list, RPARENT, INHERITS, TYPE_ID, LKEY, type_body_elements, RKEY}), [](const std::vector<ElementType>& args) -> ElementType {
		Token type_name = std::get<Token>(args[1]);
		ArgsList* args_list_node = static_cast<ArgsList*>(std::get<ASTNode*>(args[3]));
		Token inherits_name = std::get<Token>(args[6]);
		ASTNodeVector* type_body_elements_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[8]));
		int minLine = getMinLineFromArgs(args);
		return new TypeDeclNode(new IDNode(type_name.Lexeme(), type_name.Line()), args_list_node, type_body_elements_node->children, {inherits_name.Lexeme()}, minLine);
	}));
	// type_node_decl -> type TYPE_ID ( args_list ) inherits TYPE_ID ( args_list ) { type_body_elements }
	// Declaración de tipo con parámetros y herencia con parámetros para el constructor padre
	g.AddProduction(AttrProd(type_node_decl, Sentence({TYPE_, TYPE_ID, LPARENT, args_list, RPARENT, INHERITS, TYPE_ID, LPARENT, args_list, RPARENT, LKEY, type_body_elements, RKEY}), [](const std::vector<ElementType>& args) -> ElementType {
		Token type_name = std::get<Token>(args[1]);
		ArgsList* args_list_node1 = static_cast<ArgsList*>(std::get<ASTNode*>(args[3]));
		Token inherits_name = std::get<Token>(args[6]);
		ArgsList* args_list_node2 = static_cast<ArgsList*>(std::get<ASTNode*>(args[8]));
		ASTNodeVector* type_body_elements_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[11]));
		int minLine = getMinLineFromArgs(args);
		return new TypeDeclNode(new IDNode(type_name.Lexeme(), type_name.Line()), args_list_node1, type_body_elements_node->children, {inherits_name.Lexeme()}, args_list_node2, minLine);
	}));

	// --------- Elementos del cuerpo de un tipo ---------
	// Volvemos a la versión original pero mejorada para mayor compatibilidad
	
	// type_body_elements -> ε
	// Cuerpo de tipo vacío (sin atributos ni métodos)
	g.AddProduction(AttrProd(type_body_elements, Sentence(epsilon), [](const std::vector<ElementType>& args) -> ElementType {
		int minLine = getMinLineFromArgs(args);
		return new ASTNodeVector({}, minLine);
	}));
	
	// type_body_elements -> type_elements
	// Adición de un atributo a los elementos de un tipo (recursión izquierda)
	g.AddProduction(AttrProd(type_body_elements, Sentence(type_elements), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNodeVector* type_elements_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[0]));
		int minLine = getMinLineFromArgs(args);
		return new ASTNodeVector(type_elements_node->children, minLine);
	}));
	
	// //type_elements -> epsilon
	// g.AddProduction(AttrProd(type_elements, Sentence(epsilon), [](const std::vector<ElementType>& args) -> ElementType {
	// 	int minLine = getMinLineFromArgs(args);
	// 	return new ASTNodeVector({}, minLine);
	// }));
	//type_elements -> attributes
	g.AddProduction(AttrProd(type_elements, Sentence(attributes), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNodeVector* attributes_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[0]));
		int minLine = getMinLineFromArgs(args);
		return new ASTNodeVector(attributes_node->children, minLine);
	}));
	//type_elements -> methods
	g.AddProduction(AttrProd(type_elements, Sentence(methods), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNodeVector* methods_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[0]));
		int minLine = getMinLineFromArgs(args);
		return new ASTNodeVector(methods_node->children, minLine);
	}));
	//type_elements -> attributes methods
	// Combinación de atributos y métodos en el cuerpo del tipo
	g.AddProduction(AttrProd(type_elements, Sentence({attributes, methods}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNodeVector* attributes_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[0]));
		ASTNodeVector* methods_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[1]));
		
		// Combinamos los hijos de ambos nodos
		std::vector<ASTNode*> new_children = attributes_node->children;
		new_children.insert(new_children.end(), methods_node->children.begin(), methods_node->children.end());
		
		int minLine = getMinLineFromArgs(args);
		return new ASTNodeVector(new_children, minLine);
	}));
	//attributes -> epsilon
	g.AddProduction(AttrProd(attributes, Sentence(epsilon), [](const std::vector<ElementType>& args) -> ElementType {
		int minLine = getMinLineFromArgs(args);
		return new ASTNodeVector({}, minLine);
	}));
	// attributes -> attribute
	// Un único atributo
	g.AddProduction(AttrProd(attributes, Sentence(attribute), [](const std::vector<ElementType>& args) -> ElementType {
		VarAssign* attribute_node = static_cast<VarAssign*>(std::get<ASTNode*>(args[0]));
		int minLine = getMinLineFromArgs(args);
		return new ASTNodeVector({attribute_node}, minLine);
	}));
	// attributes -> attributes attribute
	// Adición de un atributo a los elementos de un tipo (recursión izquierda)	
	g.AddProduction(AttrProd(attributes, Sentence({attributes, attribute}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNodeVector* attributes_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[0]));
		VarAssign* attribute_node = static_cast<VarAssign*>(std::get<ASTNode*>(args[1]));
		
		// Insertamos el atributo al inicio para mantener el orden correcto
		std::vector<ASTNode*> new_children = {attribute_node};
		for (auto* child : attributes_node->children) {
			new_children.push_back(child);
		}
		
		int minLine = getMinLineFromArgs(args);
		return new ASTNodeVector(new_children, minLine);
	}));
	// methods -> epsilon
	// Cuerpo de tipo vacío (sin métodos)
	g.AddProduction(AttrProd(methods, Sentence(epsilon), [](const std::vector<ElementType>& args) -> ElementType {
		int minLine = getMinLineFromArgs(args);
		return new ASTNodeVector({}, minLine);
	}));
	// methods -> method
	// Un único método
	g.AddProduction(AttrProd(methods, Sentence(method), [](const std::vector<ElementType>& args) -> ElementType {
		AssignFuncNode* method_node = static_cast<AssignFuncNode*>(std::get<ASTNode*>(args[0]));
		int minLine = getMinLineFromArgs(args);
		return new ASTNodeVector({method_node}, minLine);
	}));
	// methos -> methods method 
	// Adición de un método a los elementos de un tipo (recursión izquierda)
	g.AddProduction(AttrProd(methods, Sentence({methods, method}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNodeVector* methods_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[0]));
		AssignFuncNode* method_node = static_cast<AssignFuncNode*>(std::get<ASTNode*>(args[1]));
		
		// Insertamos el método al inicio para mantener el orden correcto
		std::vector<ASTNode*> new_children = {method_node};
		for (auto* child : methods_node->children) {
			new_children.push_back(child);
		}
		
		int minLine = getMinLineFromArgs(args);
		return new ASTNodeVector(new_children, minLine);
	}));

	// type_body_elements -> method type_body_elements
	// Adición de un método a los elementos de un tipo (recursión izquierda)
	g.AddProduction(AttrProd(type_body_elements, Sentence({method, type_body_elements}), [](const std::vector<ElementType>& args) -> ElementType {
		AssignFuncNode* method_node = static_cast<AssignFuncNode*>(std::get<ASTNode*>(args[0]));
		ASTNodeVector* type_body_elements_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[1]));
		
		// Insertamos el método al inicio para mantener el orden correcto
		std::vector<ASTNode*> new_children = {method_node};
		for (auto* child : type_body_elements_node->children) {
			new_children.push_back(child);
		}
		
		int minLine = getMinLineFromArgs(args);
		return new ASTNodeVector(new_children, minLine);
	}));

	// --------- Atributos de tipos ---------
	// attribute -> id_expr = expr ;
	// Atributo con valor inicial
	g.AddProduction(AttrProd(attribute, Sentence({id_expr, EQUAL, expr, SEMICOLON}), [](const std::vector<ElementType>& args) -> ElementType {
		IDNode* id_expr_node = static_cast<IDNode*>(std::get<ASTNode*>(args[0]));
		ASTNode* expr_node = std::get<ASTNode*>(args[2]);
		int minLine = getMinLineFromArgs(args);
		return new VarAssign(id_expr_node, expr_node, minLine);
	}));
	// attribute -> id_expr = expr as ID ;
	// Atributo con valor inicial y tipo especificado
	g.AddProduction(AttrProd(attribute, Sentence({id_expr, EQUAL, expr, AS_, ID_, SEMICOLON}), [](const std::vector<ElementType>& args) -> ElementType {
		IDNode* id_expr_node = static_cast<IDNode*>(std::get<ASTNode*>(args[0]));
		ASTNode* expr_node = std::get<ASTNode*>(args[2]);
		Token type_name = std::get<Token>(args[4]);
		int minLine = getMinLineFromArgs(args);
		return new VarAssign(id_expr_node, expr_node, type_name.Lexeme(), minLine);
	}));

	// --------- Métodos de tipos ---------
	// method -> ID ( args_list ) => expr ;
	// Método con expresión inline sin tipo de retorno
	g.AddProduction(AttrProd(method, Sentence({ID_, LPARENT, args_list, RPARENT, INLINE, expr, SEMICOLON}), [](const std::vector<ElementType>& args) -> ElementType {
		Token method_name = std::get<Token>(args[0]);
		ArgsList* args_list_node = static_cast<ArgsList*>(std::get<ASTNode*>(args[2]));
		ASTNode* expr_node = std::get<ASTNode*>(args[5]);
		int minLine = getMinLineFromArgs(args);
		return new AssignFuncNode(new IDNode(method_name.Lexeme(), method_name.Line()), args_list_node, expr_node, minLine);
	}));
	// method -> ID ( args_list ) : TYPE_ID => expr ;
	// Método con expresión inline y tipo de retorno especificado
	g.AddProduction(AttrProd(method, Sentence({ID_, LPARENT, args_list, RPARENT, TWO_POINTS, TYPE_ID, INLINE, expr, SEMICOLON}), [](const std::vector<ElementType>& args) -> ElementType {
		Token method_name = std::get<Token>(args[0]);
		ArgsList* args_list_node = static_cast<ArgsList*>(std::get<ASTNode*>(args[2]));
		Token return_type = std::get<Token>(args[5]);
		ASTNode* expr_node = std::get<ASTNode*>(args[7]);
		int minLine = getMinLineFromArgs(args);
		return new AssignFuncNode(new IDNode(method_name.Lexeme(), method_name.Line()), args_list_node, expr_node, return_type.Lexeme(), minLine);
	}));
	// method -> ID ( args_list ) { lines }
	// Método con bloque de código sin tipo de retorno
	g.AddProduction(AttrProd(method, Sentence({ID_, LPARENT, args_list, RPARENT, LKEY, lines, RKEY}), [](const std::vector<ElementType>& args) -> ElementType {
		Token method_name = std::get<Token>(args[0]);
		ArgsList* args_list_node = static_cast<ArgsList*>(std::get<ASTNode*>(args[2]));
		BlockNode* lines_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[5]));
		int minLine = getMinLineFromArgs(args);
		return new AssignFuncNode(new IDNode(method_name.Lexeme(), method_name.Line()), args_list_node, lines_node, minLine);
	}));
	// method -> ID ( args_list ) : TYPE_ID { lines }
	// Método con bloque de código y tipo de retorno especificado
	g.AddProduction(AttrProd(method, Sentence({ID_, LPARENT, args_list, RPARENT, TWO_POINTS, TYPE_ID, LKEY, lines, RKEY}), [](const std::vector<ElementType>& args) -> ElementType {
		Token method_name = std::get<Token>(args[0]);
		ArgsList* args_list_node = static_cast<ArgsList*>(std::get<ASTNode*>(args[2]));
		Token return_type = std::get<Token>(args[5]);
		BlockNode* lines_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[7]));
		int minLine = getMinLineFromArgs(args);
		return new AssignFuncNode(new IDNode(method_name.Lexeme(), method_name.Line()), args_list_node, lines_node, return_type.Lexeme(), minLine);
	}));
	// ====================== ACCESO A MIEMBROS ======================
	/*
	 * El acceso a miembros permite acceder a atributos y métodos de objetos.
	 * Se puede acceder a atributos directamente o llamar a métodos con argumentos.
	 */
	
	// member_access_expr -> ID . ID
	// Acceso a un atributo de un objeto
	g.AddProduction(AttrProd(member_access_expr, Sentence({ID_, ACCESS, ID_}), [](const std::vector<ElementType>& args) -> ElementType {
		Token obj = std::get<Token>(args[0]);
		Token member_name = std::get<Token>(args[2]);
		int minLine = getMinLineFromArgs(args);
		return new AccessNode(obj.Lexeme(), new AttributeMember(member_name.Lexeme(), member_name.Line()), minLine);
	}));
	// member_access_expr -> ID . ID ( expr_list )
	// Llamada a un método de un objeto con argumentos opcionales
	g.AddProduction(AttrProd(member_access_expr, Sentence({ID_, ACCESS, ID_, LPARENT, expr_list, RPARENT}), [](const std::vector<ElementType>& args) -> ElementType {
		Token obj = std::get<Token>(args[0]);
		Token method_name = std::get<Token>(args[2]);
		ASTNodeVector* expr_list_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[4]));
		int minLine = getMinLineFromArgs(args);
		return new AccessNode(obj.Lexeme(), new MethodMember(method_name.Lexeme(), expr_list_node->children, method_name.Line()), minLine);
	}));
	return g;
}




