#include "visitor.h"
#include "../Ast/ast.hpp"
#include "context.h"
#include <iostream>

using namespace std;

/**
 * Visita un nodo del árbol de sintaxis abstracto (AST) y verifica su
 * corrección semántica en función del contexto proporcionado.
 *
 * @param node Nodo del AST a visitar
 * @param context Contexto en el que se va a visitar el nodo
 *
 * @throw std::runtime_error Si el nodo es nulo
 */


void SemanticCheckerVisitor::visit(ASTNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    node->accept(this, context);
}

void SemanticCheckerVisitor::visit(FloatNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting Float Node: " << node->value << endl;
}

void SemanticCheckerVisitor::visit(BoolNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    cout << "Visiting Bool Node: " << node->value << endl;
}

void SemanticCheckerVisitor::visit(StringNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }
    
    cout << "Visiting String Node: " << node->value << endl;
}

void SemanticCheckerVisitor::visit(UnaryOpNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting UnaryOp Node: " << node->op << endl;
    
    // First visit the operand
    node->node->accept(this, context);
    
    // Check if the unary operation is valid
    if (node->op == "!" || node->op == "not") {
        // Boolean negation - operand should be boolean-like
        cout << "Semantic check: Boolean negation operation" << endl;
    } else if (node->op == "-" || node->op == "+") {
        // Arithmetic negation/positive - operand should be numeric
        cout << "Semantic check: Arithmetic unary operation" << endl;
    } else {
        throw std::runtime_error("Semantic error: Unknown unary operator: " + node->op);
    }
}

void SemanticCheckerVisitor::visit(BinOpNode* node, Context* context) {
    if (node == nullptr) {
        throw std::runtime_error("Node is null");
    }

    cout << "Visiting BinOp Node: " << node->op << endl;
    
    // Visit both operands first
    node->left->accept(this, context);
    node->right->accept(this, context);
    
    // Check semantic validity of the binary operation
    if (node->op == "+" || node->op == "-" || node->op == "*" || node->op == "/" || node->op == "%" || node->op == "^") {
        // Arithmetic operations - both operands should be numeric
        cout << "Semantic check: Arithmetic binary operation" << endl;
    } else if (node->op == "==" || node->op == "!=" || node->op == "<" || node->op == ">" || node->op == "<=" || node->op == ">=") {
        // Comparison operations - operands should be compatible
        cout << "Semantic check: Comparison operation" << endl;
    } else if (node->op == "&&" || node->op == "||" || node->op == "and" || node->op == "or") {
        // Logical operations - both operands should be boolean-like
        cout << "Semantic check: Logical binary operation" << endl;
    } else {
        throw std::runtime_error("Semantic error: Unknown binary operator: " + node->op);
    }
}

void SemanticCheckerVisitor::visit(FunctionCallNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting FunctionNode: " << node->func_name << endl;

	// Check if function is defined in context
	// First count arguments (for now assume single argument)
	int argCount = 1; // This is a simplification - in a real implementation you'd count the arguments properly
	
	if (!context->isDefined(node->func_name, argCount)) {
		throw std::runtime_error("Semantic error: Function '" + node->func_name + "' with " + to_string(argCount) + " arguments is not defined");
	}
	
	cout << "Semantic check: Function '" << node->func_name << "' is defined" << endl;

	// Visit the argument
	node->argument->accept(this, context);
}

void SemanticCheckerVisitor::visit(IDNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting IDNode: " << node->id_name << endl;
	
	// Check if variable is defined in the current context
	if (!context->isDefined(node->id_name)) {
		throw std::runtime_error("Semantic error: Variable '" + node->id_name + "' is not defined");
	}
	
	cout << "Semantic check: Variable '" << node->id_name << "' is defined" << endl;
}

void SemanticCheckerVisitor::visit(BlockNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting BlockNode" << endl;
	
	// Create a new child context for the block scope
	Context* blockContext = context->createChildContext();
	
	// Visit all children in the block context
	for (auto child : node->children) {
		child->accept(this, blockContext);
	}
	
	cout << "Semantic check: Block processed with " << node->children.size() << " statements" << endl;
}

void SemanticCheckerVisitor::visit(ArgsList* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting ArgsList with " << node->children.size() << " arguments" << endl;

	// Check for duplicate argument names
	for (size_t i = 0; i < node->children.size(); i++) {
		for (size_t j = i + 1; j < node->children.size(); j++) {
			if (node->children[i]->id_name == node->children[j]->id_name) {
				throw std::runtime_error("Semantic error: Duplicate parameter name '" + node->children[i]->id_name + "'");
			}
		}
	}

	// Visit each argument (for semantic analysis of the identifiers)
	for (auto child : node->children) {
		cout << "Processing argument: " << child->id_name << endl;
	}
	
	cout << "Semantic check: All argument names are unique" << endl;
}

void SemanticCheckerVisitor::visit(AssignFuncNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting AssignFuncNode: " << node->func_name << endl;

	// Check if function is already defined in current scope
	int argCount = node->args->children.size();
	if (context->isLocal(node->func_name, argCount)) {
		throw std::runtime_error("Semantic error: Function '" + node->func_name + "' with " + to_string(argCount) + " parameters is already defined in this scope");
	}

	// Define the function in the current context
	if (!context->define(node->func_name, argCount)) {
		throw std::runtime_error("Semantic error: Failed to define function '" + node->func_name + "'");
	}

	// Create a new context for the function body
	Context* functionContext = context->createChildContext();

	// Add function parameters to the function context
	node->args->accept(this, context); // Check arguments for semantic validity first
	for (auto* arg : node->args->children) {
		if (!functionContext->define(arg->id_name)) {
			throw std::runtime_error("Semantic error: Failed to define parameter '" + arg->id_name + "'");
		}
	}

	// Visit the function body in the function context
	node->body->accept(this, functionContext);
	
	cout << "Semantic check: Function '" << node->func_name << "' defined successfully" << endl;
}

void SemanticCheckerVisitor::visit(LetAssign* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting LetAssign with " << node->assigns.size() << " variable assignments" << endl;

	// Create a new context for the let expression
	Context* letContext = context->createChildContext();

	// First pass: define all variables and check for duplicates within this let block
	for (auto* assign : node->assigns) {
		if (letContext->isLocal(assign->var_name)) {
			throw std::runtime_error("Semantic error: Variable '" + assign->var_name + "' is already defined in this let block");
		}
		
		// Define the variable in the let context
		if (!letContext->define(assign->var_name)) {
			throw std::runtime_error("Semantic error: Failed to define variable '" + assign->var_name + "'");
		}
	}

	// Second pass: visit all variable assignments in the let context
	for (auto* assign : node->assigns) {
		assign->accept(this, letContext);
	}

	// Visit the body in the context with the new variables
	node->body->accept(this, letContext);
	
	cout << "Semantic check: Let expression processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(VarAssign* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting VarAssign: " << node->var_name << endl;

	// Visit the value expression first to ensure it's semantically valid
	node->value->accept(this, context);
	
	// Note: The variable definition is handled by the parent context (LetAssign or VarAssignList)
	// Here we just verify the assignment value is valid
	cout << "Semantic check: Variable assignment '" << node->var_name << "' value is valid" << endl;
}

void SemanticCheckerVisitor::visit(VarAssignList* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting VarAssignList with " << node->assigns.size() << " assignments" << endl;

	// Check for duplicate variable names within this assignment list
	for (size_t i = 0; i < node->assigns.size(); i++) {
		for (size_t j = i + 1; j < node->assigns.size(); j++) {
			if (node->assigns[i]->var_name == node->assigns[j]->var_name) {
				throw std::runtime_error("Semantic error: Duplicate variable assignment '" + node->assigns[i]->var_name + "' in the same list");
			}
		}
	}

	// Visit all variable assignments
	for (auto* assign : node->assigns) {
		assign->accept(this, context);
	}
	
	cout << "Semantic check: All variable assignments in list are unique and valid" << endl;
}

void SemanticCheckerVisitor::visit(Conditional* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting Conditional (if-else)" << endl;

	// Visit the boolean expression
	node->bool_expr->accept(this, context);

	// Create separate contexts for if and else branches to handle scoping
	Context* ifContext = context->createChildContext();
	Context* elseContext = context->createChildContext();

	// Visit the if body
	node->if_body->accept(this, ifContext);

	// Visit the else body
	node->else_body->accept(this, elseContext);
	
	cout << "Semantic check: Conditional statement processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(BoolExprNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting BoolExprNode" << endl;

	// Visit the inner expression
	node->expr->accept(this, context);
	
	// The expression should evaluate to a boolean value
	// In a more sophisticated system, we would track types and verify this
	cout << "Semantic check: Boolean expression processed" << endl;
}

void SemanticCheckerVisitor::visit(WhileNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting WhileNode" << endl;

	// Visit the condition expression
	node->bool_expr->accept(this, context);

	// Create a new context for the while body to handle scoping
	Context* whileContext = context->createChildContext();

	// Visit the body in the while context
	node->body->accept(this, whileContext);
	
	cout << "Semantic check: While loop processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(VarDesAssign* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting VarDesAssign: " << node->id->id_name << endl;

	// Check if the variable exists in the context
	if (!context->isDefined(node->id->id_name)) {
		throw std::runtime_error("Variable '" + node->id->id_name + "' is not defined");
	}

	// Visit the expression to assign
	this->visit(node->value, context);
}

void SemanticCheckerVisitor::visit(ForNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting ForNode with iterator: " << node->id->id_name << endl;

	// Visit the group/collection expression
	node->group->accept(this, context);

	// Create a new context for the for loop body
	Context* forContext = context->createChildContext();

	// Define the iterator variable in the for context
	if (!forContext->define(node->id->id_name)) {
		throw std::runtime_error("Semantic error: Failed to define iterator variable '" + node->id->id_name + "'");
	}

	// Visit the body in the for context
	node->body->accept(this, forContext);
	
	cout << "Semantic check: For loop processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(TypeDeclNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting TypeDeclNode: " << node->id->id_name << endl;

	// Check if type is already defined in current scope
	int argCount = node->args->children.size();
	if (context->isLocal(node->id->id_name, argCount)) {
		throw std::runtime_error("Semantic error: Type '" + node->id->id_name + "' with " + to_string(argCount) + " parameters is already defined in this scope");
	}

	// Define the type in the current context (treating it like a constructor function)
	if (!context->define(node->id->id_name, argCount)) {
		throw std::runtime_error("Semantic error: Failed to define type '" + node->id->id_name + "'");
	}

	// Create a new context for the type body
	Context* typeContext = context->createChildContext();

	// Add type parameters to the type context
	node->args->accept(this, context); // Check arguments for semantic validity first
	for (auto* arg : node->args->children) {
		if (!typeContext->define(arg->id_name)) {
			throw std::runtime_error("Semantic error: Failed to define type parameter '" + arg->id_name + "'");
		}
	}

	// Visit the type body elements in the type context
	for (auto* element : node->body) {
		element->accept(this, typeContext);
	}
	
	cout << "Semantic check: Type '" << node->id->id_name << "' defined successfully" << endl;
}

void SemanticCheckerVisitor::visit(ASTNodeVector* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting ASTNodeVector with " << node->children.size() << " elements" << endl;

	// Visit all children in the current context
	for (auto* child : node->children) {
		child->accept(this, context);
	}
	
	cout << "Semantic check: ASTNodeVector processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(ExprsList* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting ExprsList with " << node->children.size() << " expressions" << endl;

	// Visit all expressions in the current context
	for (auto* expr : node->children) {
		expr->accept(this, context);
	}
	
	cout << "Semantic check: ExprsList processed successfully" << endl;
}

void SemanticCheckerVisitor::visit(ProgramNode* node, Context* context) {
	if (node == nullptr) {
		throw std::runtime_error("Node is null");
	}

	cout << "Visiting ProgramNode (root)" << endl;

	// Visit the main program node using the public accessor
	if (node->getNode()) {
		node->getNode()->accept(this, context);
	} else {
		cout << "Warning: ProgramNode has no child node" << endl;
	}
	
	cout << "Semantic check: Program processed successfully" << endl;
}