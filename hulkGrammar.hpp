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
	auto ASSIGN = g.SetTerminal("=");
	auto ASS_DES = g.SetTerminal(":=");
	auto IF = g.SetTerminal("if");
	auto ELSE = g.SetTerminal("else");
	auto ELIF = g.SetTerminal("elif");
	auto WHILE = g.SetTerminal("while");
	auto LET = g.SetTerminal("let");
	auto IN = g.SetTerminal("in");
	auto TYPE = g.SetTerminal("type");
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
	auto add_expr = g.SetNonTerminal("add_expr");
	auto mult_expr = g.SetNonTerminal("mult_expr");
	auto power_expr = g.SetNonTerminal("power_expr");
	// auto assign_expr = g.SetNonTerminal("assign_expr");
	auto assign_term = g.SetNonTerminal("assign_term");
	// auto concat_expr = g.SetNonTerminal("concat_expr");
	auto concat_term = g.SetNonTerminal("concat_term");
	// auto as_expr = g.SetNonTerminal("as_expr");
	auto expr_item = g.SetNonTerminal("expr_item");
	auto line_item = g.SetNonTerminal("line_item");
	auto unary_expr = g.SetNonTerminal("unary_expr");
	auto primary_expr = g.SetNonTerminal("primary_expr");
	auto id_expr = g.SetNonTerminal("id_expr");
	// Jerarquía para expresiones booleanas
	auto bool_expr = g.SetNonTerminal("bool_expr");
	auto or_expr = g.SetNonTerminal("or_expr");
	auto and_expr = g.SetNonTerminal("and_expr");
	auto not_expr = g.SetNonTerminal("not_expr");
	auto comp_expr = g.SetNonTerminal("comp_expr");
	auto eq_expr = g.SetNonTerminal("eq_expr");       // ==, != (igualdad)
	auto rel_expr = g.SetNonTerminal("rel_expr");     // <, <=, >, >= (relacional)
	auto bool_primary = g.SetNonTerminal("bool_primary"); // primarios booleanos (literales, expresiones entre paréntesis, etc)
	
	// Jerarquía para otras operaciones binarias
	auto assign_expr = g.SetNonTerminal("assign_expr");      // Asignación (=)
	auto concat_expr = g.SetNonTerminal("concat_expr");      // Concatenación (@, @@)
	auto as_expr = g.SetNonTerminal("as_expr");              // Conversión de tipo (as)

	auto args_list = g.SetNonTerminal("args_list");
	auto let_assign = g.SetNonTerminal("let_assign");
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
	// //declarations -> epsilon
	// g.AddProduction(AttrProd(declarations, Sentence(epsilon), [](const std::vector<ElementType>& args) -> ElementType {
	// 	int minLine = getMinLineFromArgs(args);
	// 	return new ASTNodeVector({}, minLine);
	// }));

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
	// lines_block -> epsilon
	g.AddProduction(AttrProd(lines_block, Sentence(epsilon), [](const std::vector<ElementType>& args) -> ElementType {
		int minLine = getMinLineFromArgs(args);
		return new BlockNode({}, minLine);
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

	// ----- NUEVA IMPLEMENTACIÓN DE SECUENCIAS DE LÍNEAS (SLR1 COMPATIBLE) -----
	
	// non_empty_lines -> line_item
	g.AddProduction(AttrProd(non_empty_lines, Sentence(line_item), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// line_item -> line
	// Línea individual
	g.AddProduction(AttrProd(line_item, Sentence(line), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* node = std::get<ASTNode*>(args[0]);
		int minLine = getMinLineFromArgs(args);
		return new BlockNode({node}, minLine);
	}));
	
	// non_empty_lines -> line line_item
	// Una secuencia de líneas (sin recursión a izquierda directa)
	g.AddProduction(AttrProd(non_empty_lines, Sentence({line, line_item}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* line_node = std::get<ASTNode*>(args[0]);
		BlockNode* line_item_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[1]));
		
		// Insertamos la nueva línea al principio
		line_item_node->children.insert(line_item_node->children.begin(), line_node);
		return line_item_node;
	}));

	// --------- Líneas individuales de código ---------
	// line -> expr ;
	// Una expresión que termina con punto y coma
	g.AddProduction(AttrProd(line, Sentence({expr, SEMICOLON}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node = std::get<ASTNode*>(args[0]);
		return expr_node;
	}));
	
	// line -> func_assign ;
	// Una declaración de función que termina con punto y coma
	g.AddProduction(AttrProd(line, Sentence({func_assign, SEMICOLON}), [](const std::vector<ElementType>& args) -> ElementType {
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
	
	// expr -> assign_expr
	// Expresión principal - conectamos con la nueva jerarquía de asignación/concatenación/cast
	g.AddProduction(AttrProd(expr, Sentence( assign_expr ), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node = std::get<ASTNode*>(args[0]);
		return expr_node;
	}));
	
	// Mantenemos la original para compatibilidad
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
	
	// expr -> let_assign
	// Expresión let-in para declaraciones locales
	g.AddProduction(AttrProd(expr, Sentence(let_assign), [](const std::vector<ElementType>& args) -> ElementType {
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
	// expr -> id := assign_expr
	// Asignación destructiva (ajustada a la nueva jerarquía de asignación)
	g.AddProduction(AttrProd(expr, Sentence({ID_, ASS_DES, assign_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* var_id_node = std::get<ASTNode*>(args[0]);
		ASTNode* assign_expr_node = std::get<ASTNode*>(args[2]);
		int minLine = getMinLineFromArgs(args);
		return new BinOpNode(var_id_node, ":=", assign_expr_node, minLine);
	}));
	
	// expr -> assign_expr
	// Expresiones de asignación
	g.AddProduction(AttrProd(expr, Sentence(assign_expr), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// ----- NUEVA IMPLEMENTACIÓN DE OPERADORES DE ASIGNACIÓN (SLR1 COMPATIBLE) -----
	
	// assign_expr -> assign_term
	g.AddProduction(AttrProd(assign_expr, Sentence(assign_term), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// assign_term -> concat_expr
	g.AddProduction(AttrProd(assign_term, Sentence(concat_expr), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// assign_expr -> assign_expr = assign_term
	// Asignación normal con recursión izquierda (pero controlada en jerarquía)
	g.AddProduction(AttrProd(assign_expr, Sentence({assign_expr, ASSIGN, assign_term}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* assign_expr_node = std::get<ASTNode*>(args[0]);
		ASTNode* assign_term_node = std::get<ASTNode*>(args[2]);
		int minLine = getMinLineFromArgs(args);
		return new BinOpNode(assign_expr_node, "=", assign_term_node, minLine);
	}));
	
	// ----- NUEVA IMPLEMENTACIÓN DE OPERADORES DE CONCATENACIÓN (SLR1 COMPATIBLE) -----
	
	// concat_expr -> concat_term
	g.AddProduction(AttrProd(concat_expr, Sentence(concat_term), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// concat_term -> as_expr
	g.AddProduction(AttrProd(concat_term, Sentence(as_expr), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// concat_expr -> concat_expr @ concat_term
	// Operador de composición simple con recursión izquierda (pero controlada en jerarquía)
	g.AddProduction(AttrProd(concat_expr, Sentence({concat_expr, ARROBA_, concat_term}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* concat_expr_node = std::get<ASTNode*>(args[0]);
		ASTNode* concat_term_node = std::get<ASTNode*>(args[2]);
		int minLine = getMinLineFromArgs(args);
		return new BinOpNode(concat_expr_node, "@", concat_term_node, minLine);
	}));
	
	// concat_expr -> concat_expr @@ concat_term
	// Operador de composición doble con recursión izquierda (pero controlada en jerarquía)
	g.AddProduction(AttrProd(concat_expr, Sentence({concat_expr, D_ARROBA_, concat_term}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* concat_expr_node = std::get<ASTNode*>(args[0]);
		ASTNode* concat_term_node = std::get<ASTNode*>(args[2]);
		int minLine = getMinLineFromArgs(args);
		return new BinOpNode(concat_expr_node, "@@", concat_term_node, minLine);
	}));
	
	// ----- NUEVA IMPLEMENTACIÓN DE TYPE CAST (SLR1 COMPATIBLE) -----
	// as_expr -> arit_op
	g.AddProduction(AttrProd(as_expr, Sentence(arit_op), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// as_expr -> arit_op as ID
	// Conversión explícita de tipo (cast) sin recursión izquierda
	g.AddProduction(AttrProd(as_expr, Sentence({arit_op, AS_, ID_}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* arit_op_node = std::get<ASTNode*>(args[0]);
		Token type_name_token = std::get<Token>(args[2]);
		int minLine = getMinLineFromArgs(args);
		return new TypeCastNode(arit_op_node, type_name_token.Lexeme(), minLine);
	}));

	// ====================== DEFINICIONES DE FUNCIONES ======================
	/* 
	 * Las funciones en HULK pueden definirse con expresiones inline (=>) o con bloques de código.
	 * También pueden tener un tipo de retorno especificado o no.
	 */
	
	// func_assign -> function ID ( args_list ) => expr
	// Función con expresión inline sin tipo de retorno
	g.AddProduction(AttrProd(func_assign, Sentence({FUNCTION_, ID_, LPARENT, args_list, RPARENT, INLINE, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		// std::string func_name = std::get<std::string>(args[1]);
		Token func_name = std::get<Token>(args[1]);
		ArgsList* args_list_node = static_cast<ArgsList*>(std::get<ASTNode*>(args[3]));
		ASTNode* expr_node = std::get<ASTNode*>(args[6]);
		int minLine = getMinLineFromArgs(args);
		return new AssignFuncNode(new IDNode(func_name.Lexeme(), func_name.Line()), args_list_node, expr_node, minLine);
	}));
	
	// func_assign -> function ID ( args_list ) : TYPE_ID => expr
	// Función con expresión inline con tipo de retorno especificado
	g.AddProduction(AttrProd(func_assign, Sentence({FUNCTION_, ID_, LPARENT, args_list, RPARENT, TWO_POINTS, TYPE_ID, INLINE, expr}), [](const std::vector<ElementType>& args) -> ElementType {
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
	
	// let_assign -> LET var_assign_list IN expr
	// Asignación let-in con una expresión como cuerpo
	g.AddProduction(AttrProd(let_assign, Sentence({LET, var_assign_list, IN, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		VarAssignList* var_assign_list_node = static_cast<VarAssignList*>(std::get<ASTNode*>(args[1]));
		ASTNode* expr_node = std::get<ASTNode*>(args[3]);
		return new LetAssign(var_assign_list_node->assigns, expr_node, getMinLineFromArgs(args));
	}));
	// let_assign -> LET var_assign_list IN lines_block
	// Asignación let-in con un bloque de código como cuerpo
	g.AddProduction(AttrProd(let_assign, Sentence({LET, var_assign_list, IN, lines_block}), [](const std::vector<ElementType>& args) -> ElementType {
		VarAssignList* var_assign_list_node = static_cast<VarAssignList*>(std::get<ASTNode*>(args[1]));
		BlockNode* lines_block_node = static_cast<BlockNode*>(std::get<ASTNode*>(args[3]));
		int minLine = getMinLineFromArgs(args);
		return new LetAssign(var_assign_list_node->assigns, lines_block_node, minLine);
	}));

	// --------- Lista de asignaciones de variables ---------
	// var_assign_list -> id_expr = expr
	// Asignación básica de una variable con una expresión
	g.AddProduction(AttrProd(var_assign_list, Sentence({id_expr, ASSIGN, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		IDNode* id_expr_node = static_cast<IDNode*>(std::get<ASTNode*>(args[0]));
		ASTNode* expr_node = std::get<ASTNode*>(args[2]);
		int minLine = getMinLineFromArgs(args);
		return new VarAssignList({new VarAssign(id_expr_node, expr_node, minLine)}, minLine);
	}));
	// var_assign_list -> id_expr = expr as ID
	// Asignación de una variable con una expresión y especificación de tipo
	g.AddProduction(AttrProd(var_assign_list, Sentence({id_expr, ASSIGN, expr, AS_, ID_}), [](const std::vector<ElementType>& args) -> ElementType {
		IDNode* id_expr_node = static_cast<IDNode*>(std::get<ASTNode*>(args[0]));
		ASTNode* expr_node = std::get<ASTNode*>(args[2]);
		std::string type_name = std::get<std::string>(args[4]);
		int minLine = getMinLineFromArgs(args);
		return new VarAssignList({new VarAssign(id_expr_node, expr_node, type_name, minLine)}, minLine);
	}));
	// var_assign_list -> id_expr = new_expr
	// Asignación de una variable con una expresión de creación de objeto
	g.AddProduction(AttrProd(var_assign_list, Sentence({id_expr, ASSIGN, new_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		IDNode* id_expr_node = static_cast<IDNode*>(std::get<ASTNode*>(args[0]));
		NewTypeNode* new_expr_node = static_cast<NewTypeNode*>(std::get<ASTNode*>(args[2]));
		int minLine = getMinLineFromArgs(args);
		return new VarAssignList({new VarAssign(id_expr_node, new_expr_node, minLine)}, minLine);
	}));
	// var_assign_list -> var_assign_list , id_expr = expr
	// Lista de asignaciones de variables con una expresión adicional
	g.AddProduction(AttrProd(var_assign_list, Sentence({var_assign_list, COLON, id_expr, ASSIGN, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		VarAssignList* var_assign_list_node = static_cast<VarAssignList*>(std::get<ASTNode*>(args[0]));
		IDNode* id_expr_node = static_cast<IDNode*>(std::get<ASTNode*>(args[2]));
		ASTNode* expr_node = std::get<ASTNode*>(args[4]);
		int minLine = getMinLineFromArgs(args);
		var_assign_list_node->add_child(new VarAssign(id_expr_node, expr_node, minLine));
		return var_assign_list_node;
	}));
	// var_assign_list -> var_assign_list , id_expr = expr as ID
	// Lista de asignaciones de variables con una expresión adicional y tipo específico
	g.AddProduction(AttrProd(var_assign_list, Sentence({var_assign_list, COLON, id_expr, ASSIGN, expr, AS_, ID_}), [](const std::vector<ElementType>& args) -> ElementType {
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
	g.AddProduction(AttrProd(var_assign_list, Sentence({var_assign_list, COLON, id_expr, ASSIGN, new_expr}), [](const std::vector<ElementType>& args) -> ElementType {
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
	// ----- NUEVA IMPLEMENTACIÓN DE LISTAS DE EXPRESIONES (SLR1 COMPATIBLE) -----
	
	// expr_list -> expr_item
	g.AddProduction(AttrProd(expr_list, Sentence(expr_item), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// expr_item -> expr
	// Item individual de expresión normal
	g.AddProduction(AttrProd(expr_item, Sentence(expr), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node = std::get<ASTNode*>(args[0]);
		int minLine = getMinLineFromArgs(args);
		return new ASTNodeVector({expr_node}, minLine);
	}));
	
	// expr_item -> new_expr
	// Item individual de expresión de creación de objeto
	g.AddProduction(AttrProd(expr_item, Sentence(new_expr), [](const std::vector<ElementType>& args) -> ElementType {
		NewTypeNode* new_expr_node = static_cast<NewTypeNode*>(std::get<ASTNode*>(args[0]));
		return new ASTNodeVector({new_expr_node}, new_expr_node->line + 1);
	}));
	
	// expr_list -> expr_list , expr_item
	// Lista extendida con un nuevo ítem (puede ser expr o new_expr)
	g.AddProduction(AttrProd(expr_list, Sentence({expr_list, COLON, expr_item}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNodeVector* expr_list_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[0]));
		ASTNodeVector* expr_item_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[2]));
		
		// Agregamos cada hijo del item a la lista principal
		for (auto child : expr_item_node->children) {
			expr_list_node->add_child(child);
		}
		
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
	
	// ----- NUEVA IMPLEMENTACIÓN DE OPERACIONES ARITMÉTICAS (SLR1 COMPATIBLE) -----
	
	// arit_op -> add_expr
	// Entrada al nivel más bajo de precedencia (suma/resta)
	g.AddProduction(AttrProd(arit_op, Sentence(add_expr), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// ----- SUMAS Y RESTAS (precedencia baja) -----
	
	// add_expr -> add_expr + mult_expr
	// Suma (asociatividad izquierda)
	g.AddProduction(AttrProd(add_expr, Sentence({add_expr, PLUS, mult_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* left = std::get<ASTNode*>(args[0]);
		ASTNode* right = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(left, right);
		return new BinOpNode(left, "+", right, line);
	}));
	
	// add_expr -> add_expr - mult_expr
	// Resta (asociatividad izquierda)
	g.AddProduction(AttrProd(add_expr, Sentence({add_expr, MINUS, mult_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* left = std::get<ASTNode*>(args[0]);
		ASTNode* right = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(left, right);
		return new BinOpNode(left, "-", right, line);
	}));
	
	// add_expr -> mult_expr
	// Paso al siguiente nivel de precedencia
	g.AddProduction(AttrProd(add_expr, Sentence(mult_expr), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// ----- MULTIPLICACIONES Y DIVISIONES (precedencia media) -----
	
	// mult_expr -> mult_expr * power_expr
	// Multiplicación (asociatividad izquierda)
	g.AddProduction(AttrProd(mult_expr, Sentence({mult_expr, TIMES, power_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* left = std::get<ASTNode*>(args[0]);
		ASTNode* right = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(left, right);
		return new BinOpNode(left, "*", right, line);
	}));
	
	// mult_expr -> mult_expr / power_expr
	// División (asociatividad izquierda)
	g.AddProduction(AttrProd(mult_expr, Sentence({mult_expr, DIV, power_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* left = std::get<ASTNode*>(args[0]);
		ASTNode* right = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(left, right);
		return new BinOpNode(left, "/", right, line);
	}));
	
	// mult_expr -> mult_expr % power_expr
	// Módulo (asociatividad izquierda)
	g.AddProduction(AttrProd(mult_expr, Sentence({mult_expr, MOD, power_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* left = std::get<ASTNode*>(args[0]);
		ASTNode* right = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(left, right);
		return new BinOpNode(left, "%", right, line);
	}));
	
	// mult_expr -> power_expr
	// Paso al siguiente nivel de precedencia
	g.AddProduction(AttrProd(mult_expr, Sentence(power_expr), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// ----- POTENCIA (precedencia alta) -----
	
	// power_expr -> unary_expr ^ power_expr
	// Potencia (asociatividad derecha)
	g.AddProduction(AttrProd(power_expr, Sentence({unary_expr, POW, power_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* left = std::get<ASTNode*>(args[0]);
		ASTNode* right = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(left, right);
		return new BinOpNode(left, "^", right, line);
	}));
	
	// power_expr -> unary_expr
	// Paso al siguiente nivel de precedencia
	g.AddProduction(AttrProd(power_expr, Sentence(unary_expr), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	// ----- OPERADORES UNARIOS -----
	
	// unary_expr -> - primary_expr
	// Negación unaria
	g.AddProduction(AttrProd(unary_expr, Sentence({MINUS, primary_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr = std::get<ASTNode*>(args[1]);
		int line = getMinLineFromArgs(args);
		return new UnaryOpNode("-", expr, line);
	}));
	
	// unary_expr -> primary_expr
	// Paso al siguiente nivel de precedencia
	g.AddProduction(AttrProd(unary_expr, Sentence(primary_expr), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// ----- TÉRMINOS PRIMARIOS -----
	
	// primary_expr -> NUMBER
	// Literal numérico
	g.AddProduction(AttrProd(primary_expr, Sentence(NUMBER), [](const std::vector<ElementType>& args) -> ElementType {
		Token number_token = std::get<Token>(args[0]);
		float number = std::stof(number_token.Lexeme());
		int line = number_token.Line();
		return new FloatNode(number, line);
	}));
	
	// primary_expr -> ( expr )
	// Expresión entre paréntesis para agrupar o cambiar precedencia
	g.AddProduction(AttrProd(primary_expr, Sentence({LPARENT, expr, RPARENT}), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[1]);
	}));
	
	// Mantener la implementación original para el resto de operadores unarios
	// arit_op -> - expr (ANTIGUA IMPLEMENTACIÓN - MANTENIDA PARA COMPATIBILIDAD)
	g.AddProduction(AttrProd(arit_op, Sentence({MINUS, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node = std::get<ASTNode*>(args[1]);
		int minLine = getMinLineFromArgs(args);
		return new UnaryOpNode("-", expr_node, minLine);
	}));

	// ====================== EXPRESIONES BOOLEANAS ======================
	/*
	 * Las expresiones booleanas incluyen valores booleanos (true/false), operaciones de comparación,
	 * operadores lógicos (AND, OR, NOT) y verificación de tipo (is).
	 * 
	 * La jerarquía de precedencia de operadores de menor a mayor es:
	 * 1. OR (|)
	 * 2. AND (&)
	 * 3. NOT (!)
	 * 4. Comparaciones (>, >=, <, <=, ==, !=)
	 * 5. Valores booleanos, expresiones entre paréntesis, comprobación de tipos (is)
	 */
	
	// === NUEVA IMPLEMENTACIÓN SLR(1) COMPATIBLE ===
	// La jerarquía de precedencia booleana sigue este esquema:
	// bool_expr -> or_expr
	// or_expr -> and_expr | or_expr OR and_expr
	// and_expr -> not_expr | and_expr AND not_expr
	// not_expr -> comp_expr | NOT comp_expr
	// comp_expr -> expr comparison_op expr | BOOLEAN | (bool_expr) | IS_expr
	
	// bool_expr -> or_expr
	g.AddProduction(AttrProd(bool_expr, Sentence(or_expr), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// or_expr -> and_expr
	g.AddProduction(AttrProd(or_expr, Sentence(and_expr), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// or_expr -> or_expr OR and_expr (left recursion)
	g.AddProduction(AttrProd(or_expr, Sentence({or_expr, OR_, and_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* or_expr_node = std::get<ASTNode*>(args[0]);
		ASTNode* and_expr_node = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(or_expr_node, and_expr_node);
		return new BoolExprNode(new BinOpNode(or_expr_node, "|", and_expr_node, line), line);
	}));
	
	// and_expr -> not_expr
	g.AddProduction(AttrProd(and_expr, Sentence(not_expr), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// and_expr -> and_expr AND not_expr (left recursion)
	g.AddProduction(AttrProd(and_expr, Sentence({and_expr, AND_, not_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* and_expr_node = std::get<ASTNode*>(args[0]);
		ASTNode* not_expr_node = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(and_expr_node, not_expr_node);
		return new BoolExprNode(new BinOpNode(and_expr_node, "&", not_expr_node, line), line);
	}));
	
	// not_expr -> comp_expr
	g.AddProduction(AttrProd(not_expr, Sentence(comp_expr), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// not_expr -> NOT not_expr
	g.AddProduction(AttrProd(not_expr, Sentence({NOT_, not_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* expr_node = std::get<ASTNode*>(args[1]);
		return new BoolExprNode(new UnaryOpNode("!", expr_node, expr_node->line), expr_node->line);
	}));
	
	// === NUEVA ESTRUCTURA DE COMPARACIÓN ===
	// La jerarquía de operadores de comparación es:
	// comp_expr -> eq_expr
	// eq_expr -> rel_expr | eq_expr == rel_expr | eq_expr != rel_expr
	// rel_expr -> bool_primary | rel_expr < bool_primary | rel_expr <= bool_primary | rel_expr > bool_primary | rel_expr >= bool_primary
	// bool_primary -> BOOLEAN | (bool_expr) | id_expr IS TYPE_ID | func_call IS ID
	
	// comp_expr -> eq_expr
	g.AddProduction(AttrProd(comp_expr, Sentence(eq_expr), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// eq_expr -> rel_expr
	g.AddProduction(AttrProd(eq_expr, Sentence(rel_expr), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// eq_expr -> eq_expr == rel_expr (recursión izquierda)
	g.AddProduction(AttrProd(eq_expr, Sentence({eq_expr, EQ_, rel_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* eq_expr_node = std::get<ASTNode*>(args[0]);
		ASTNode* rel_expr_node = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(eq_expr_node, rel_expr_node);
		return new BoolExprNode(new BinOpNode(eq_expr_node, "==", rel_expr_node, line), line);
	}));
	
	// eq_expr -> eq_expr != rel_expr (recursión izquierda)
	g.AddProduction(AttrProd(eq_expr, Sentence({eq_expr, DISTINCT_, rel_expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* eq_expr_node = std::get<ASTNode*>(args[0]);
		ASTNode* rel_expr_node = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(eq_expr_node, rel_expr_node);
		return new BoolExprNode(new BinOpNode(eq_expr_node, "!=", rel_expr_node, line), line);
	}));
	
	// rel_expr -> bool_primary
	g.AddProduction(AttrProd(rel_expr, Sentence(bool_primary), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// rel_expr -> rel_expr < bool_primary (recursión izquierda)
	g.AddProduction(AttrProd(rel_expr, Sentence({rel_expr, LESS_, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* rel_expr_node = std::get<ASTNode*>(args[0]);
		ASTNode* primary_node = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(rel_expr_node, primary_node);
		return new BoolExprNode(new BinOpNode(rel_expr_node, "<", primary_node, line), line);
	}));
	
	// rel_expr -> rel_expr <= bool_primary (recursión izquierda)
	g.AddProduction(AttrProd(rel_expr, Sentence({rel_expr, LESS_EQ_, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* rel_expr_node = std::get<ASTNode*>(args[0]);
		ASTNode* primary_node = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(rel_expr_node, primary_node);
		return new BoolExprNode(new BinOpNode(rel_expr_node, "<=", primary_node, line), line);
	}));
	
	// rel_expr -> rel_expr > bool_primary (recursión izquierda)
	g.AddProduction(AttrProd(rel_expr, Sentence({rel_expr, GR_, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* rel_expr_node = std::get<ASTNode*>(args[0]);
		ASTNode* primary_node = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(rel_expr_node, primary_node);
		return new BoolExprNode(new BinOpNode(rel_expr_node, ">", primary_node, line), line);
	}));
	
	// rel_expr -> rel_expr >= bool_primary (recursión izquierda)
	g.AddProduction(AttrProd(rel_expr, Sentence({rel_expr, GR_EQ_, expr}), [](const std::vector<ElementType>& args) -> ElementType {
		ASTNode* rel_expr_node = std::get<ASTNode*>(args[0]);
		ASTNode* primary_node = std::get<ASTNode*>(args[2]);
		int line = getMinLineFromNodes(rel_expr_node, primary_node);
		return new BoolExprNode(new BinOpNode(rel_expr_node, ">=", primary_node, line), line);
	}));
	
	// bool_primary -> expr
	g.AddProduction(AttrProd(bool_primary, Sentence(expr), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[0]);
	}));
	
	// bool_primary -> BOOLEAN
	g.AddProduction(AttrProd(bool_primary, Sentence(BOOLEAN), [](const std::vector<ElementType>& args) -> ElementType {
		Token bool_token = std::get<Token>(args[0]);
		return new BoolExprNode(new BoolNode(bool_token.Lexeme() == "true", bool_token.Line()), bool_token.Line());	
	}));
	
	// bool_primary -> ( bool_expr )
	g.AddProduction(AttrProd(bool_primary, Sentence({LPARENT, bool_expr, RPARENT}), [](const std::vector<ElementType>& args) -> ElementType {
		return std::get<ASTNode*>(args[1]); // Simplemente devuelve la expresión booleana entre paréntesis
	}));
	
	// bool_primary -> id_expr IS TYPE_ID
	g.AddProduction(AttrProd(bool_primary, Sentence({id_expr, IS_, TYPE_ID}), [](const std::vector<ElementType>& args) -> ElementType {
		IDNode* id_expr_node = static_cast<IDNode*>(std::get<ASTNode*>(args[0]));
		Token type_id_token = std::get<Token>(args[2]);
		int minline = getMinLineFromArgs(args);
		return new BoolExprNode(new BinOpNode(id_expr_node, "is", new IDNode(type_id_token.Lexeme(), type_id_token.Line()), minline), minline);
	}));
	
	// bool_primary -> func_call IS ID
	g.AddProduction(AttrProd(bool_primary, Sentence({func_call, IS_, ID_}), [](const std::vector<ElementType>& args) -> ElementType {
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
	g.AddProduction(AttrProd(type_node_decl, Sentence({TYPE, TYPE_ID, LKEY, type_body_elements, RKEY}), [](const std::vector<ElementType>& args) -> ElementType {
		Token type_name = std::get<Token>(args[1]);
		ASTNodeVector* type_body_elements_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[3]));
		int minLine = getMinLineFromArgs(args);
		return new TypeDeclNode(new IDNode(type_name.Lexeme(), type_name.Line()), new ArgsList({}, minLine), type_body_elements_node->children, minLine);
	}));
	// type_node_decl -> type TYPE_ID inherits TYPE_ID { type_body_elements }
	// Declaración de tipo con herencia
	g.AddProduction(AttrProd(type_node_decl, Sentence({TYPE, TYPE_ID, INHERITS, TYPE_ID, LKEY, type_body_elements, RKEY}), [](const std::vector<ElementType>& args) -> ElementType {
		Token type_name = std::get<Token>(args[1]);
		Token inherits_name = std::get<Token>(args[3]);
		ASTNodeVector* type_body_elements_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[5]));
		int minLine = getMinLineFromArgs(args);
		return new TypeDeclNode(new IDNode(type_name.Lexeme(), type_name.Line()), new ArgsList({}, minLine), type_body_elements_node->children, {inherits_name.Lexeme()}, minLine);
	}));
	// type_node_decl -> type TYPE_ID inherits TYPE_ID ( args_list ) { type_body_elements }
	// Declaración de tipo con herencia y argumentos para el constructor del padre
	g.AddProduction(AttrProd(type_node_decl, Sentence({TYPE, TYPE_ID, INHERITS, TYPE_ID, LPARENT, args_list, RPARENT, LKEY, type_body_elements, RKEY}), [](const std::vector<ElementType>& args) -> ElementType {
		Token type_name = std::get<Token>(args[1]);
		Token inherits_name = std::get<Token>(args[3]);
		ArgsList* args_list_node = static_cast<ArgsList*>(std::get<ASTNode*>(args[5]));
		ASTNodeVector* type_body_elements_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[8]));
		int minLine = getMinLineFromArgs(args);
		return new TypeDeclNode(new IDNode(type_name.Lexeme(), type_name.Line()), new ArgsList({}, minLine), type_body_elements_node->children, {inherits_name.Lexeme()}, args_list_node, minLine);
	}));
	// type_node_decl -> type TYPE_ID ( args_list ) { type_body_elements }
	// Declaración de tipo con parámetros de constructor
	g.AddProduction(AttrProd(type_node_decl, Sentence({TYPE, TYPE_ID, LPARENT, args_list, RPARENT, LKEY, type_body_elements, RKEY}), [](const std::vector<ElementType>& args) -> ElementType {
		Token type_name = std::get<Token>(args[1]);
		ArgsList* args_list_node = static_cast<ArgsList*>(std::get<ASTNode*>(args[3]));
		ASTNodeVector* type_body_elements_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[6]));
		int minLine = getMinLineFromArgs(args);
		return new TypeDeclNode(new IDNode(type_name.Lexeme(), type_name.Line()), args_list_node, type_body_elements_node->children, minLine);
	}));
	// type_node_decl -> type TYPE_ID ( args_list ) inherits TYPE_ID { type_body_elements }
	// Declaración de tipo con parámetros y herencia
	g.AddProduction(AttrProd(type_node_decl, Sentence({TYPE, TYPE_ID, LPARENT, args_list, RPARENT, INHERITS, TYPE_ID, LKEY, type_body_elements, RKEY}), [](const std::vector<ElementType>& args) -> ElementType {
		Token type_name = std::get<Token>(args[1]);
		ArgsList* args_list_node = static_cast<ArgsList*>(std::get<ASTNode*>(args[3]));
		Token inherits_name = std::get<Token>(args[6]);
		ASTNodeVector* type_body_elements_node = static_cast<ASTNodeVector*>(std::get<ASTNode*>(args[8]));
		int minLine = getMinLineFromArgs(args);
		return new TypeDeclNode(new IDNode(type_name.Lexeme(), type_name.Line()), args_list_node, type_body_elements_node->children, {inherits_name.Lexeme()}, minLine);
	}));
	// type_node_decl -> type TYPE_ID ( args_list ) inherits TYPE_ID ( args_list ) { type_body_elements }
	// Declaración de tipo con parámetros y herencia con parámetros para el constructor padre
	g.AddProduction(AttrProd(type_node_decl, Sentence({TYPE, TYPE_ID, LPARENT, args_list, RPARENT, INHERITS, TYPE_ID, LPARENT, args_list, RPARENT, LKEY, type_body_elements, RKEY}), [](const std::vector<ElementType>& args) -> ElementType {
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
	g.AddProduction(AttrProd(attribute, Sentence({id_expr, ASSIGN, expr, SEMICOLON}), [](const std::vector<ElementType>& args) -> ElementType {
		IDNode* id_expr_node = static_cast<IDNode*>(std::get<ASTNode*>(args[0]));
		ASTNode* expr_node = std::get<ASTNode*>(args[2]);
		int minLine = getMinLineFromArgs(args);
		return new VarAssign(id_expr_node, expr_node, minLine);
	}));
	// attribute -> id_expr = expr as ID ;
	// Atributo con valor inicial y tipo especificado
	g.AddProduction(AttrProd(attribute, Sentence({id_expr, ASSIGN, expr, AS_, ID_, SEMICOLON}), [](const std::vector<ElementType>& args) -> ElementType {
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




