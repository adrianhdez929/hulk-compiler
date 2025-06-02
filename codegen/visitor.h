#include "../semantic/visitor.h"
#include "../semantic/context.h"
#include <llvm/IR/Value.h>

#ifndef CODEGEN_VISITOR_H
#define CODEGEN_VISITOR_H

class CodegenVisitor : public Visitor {
    public:
    CodegenVisitor() : moduleNode(nullptr), globalContext(nullptr) {}

    // Set the root node and global context
    void setRootNode(ASTNode* node, Context* context) {
        moduleNode = node;
        globalContext = context;
    }

    // Visit methods for AST nodes
    void visit(ASTNode* node, Context* context) override;
    void visit(FloatNode* node, Context* context) override;
    void visit(BoolNode* node, Context* context) override;
    void visit(StringNode* node, Context* context) override;
    void visit(UnaryOpNode* node, Context* context) override;
    void visit(BinOpNode* node, Context* context) override;
	void visit(FunctionCallNode* node, Context* context) override;
	void visit(IDNode* node, Context* context) override;
	void visit(BlockNode* node, Context* context) override;
	void visit(ArgsList* node, Context* context) override;
	void visit(AssignFuncNode* node, Context* context) override;
	void visit(LetAssign* node, Context* context) override;
	void visit(VarAssign* node, Context* context) override;
	void visit(VarAssignList* node, Context* context) override;
	void visit(Conditional* node, Context* context) override;
	void visit(BoolExprNode* node, Context* context) override;
	void visit(WhileNode* node, Context* context) override;
	void visit(VarDesAssign* node, Context* context) override;
	void visit(ForNode* node, Context* context) override;
	void visit(TypeDeclNode* node, Context* context) override;
	void visit(ASTNodeVector* node, Context* context) override;
	void visit(ExprsList* node, Context* context) override;
	void visit(ProgramNode* node, Context* context) override;

    // JIT methods
    void initialize();
    void optimize();
    void generateCode();

    private:
    ASTNode* moduleNode;
    Context* globalContext;
    llvm::Value* currentValue = nullptr;
    
    // Helper methods for standard library
    void createStandardLibraryDeclarations();
    llvm::Function* createMainFunction();
};

#endif