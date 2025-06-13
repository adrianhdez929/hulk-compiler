#include "../semantic/visitor.h"
#include "../semantic/context.h"
#include <llvm/IR/Value.h>
#include <llvm/IR/DerivedTypes.h>
#include <map>

#ifndef CODEGEN_VISITOR_H
#define CODEGEN_VISITOR_H

class CodegenVisitor : public Visitor {
    public:
    CodegenVisitor() : moduleNode(nullptr), globalContext(nullptr) {}

    void setRootNode(ASTNode* node, Context* context) {
        moduleNode = node;
        globalContext = context;
    }

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
	void visit(NewTypeNode* node, Context* context) override;
	void visit(VarAssignType* node, Context* context) override;
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
	void visit(AccessNode* node, Context* context) override;
	void visit(TypeAssMember* node, Context* context) override;
	void visit(AttributeMember* node, Context* context) override;
	void visit(MethodMember* node, Context* context) override;

    void initialize();
    void optimize();
    void generateCode();

    private:
    ASTNode* moduleNode;
    Context* globalContext;
    llvm::Value* currentValue = nullptr;
    llvm::Value* currentObjectPtr = nullptr;
    std::string currentObjectName;
    
    std::map<std::string, llvm::StructType*> typeStructMap;
    std::map<std::string, std::vector<std::string>> typeAttributeMap;
    std::map<std::string, llvm::Value*> objectInstances;
    std::map<std::string, std::string> objectTypes;
    // Add attribute type map for struct fields
    typedef std::map<std::string, llvm::Type*> AttributeTypeMap;
    std::map<std::string, AttributeTypeMap> typeAttributeTypeMap; 
    
    void createStandardLibraryDeclarations();
    llvm::Function* createMainFunction();
    
    void generateMethodFunction(const std::string& typeName, AssignFuncNode* method, 
                               llvm::StructType* structType, const std::vector<std::string>& attributes);
    void generateForwardingMethod(const std::string& childTypeName, const std::string& parentTypeName,
                                 const std::string& methodName, llvm::StructType* structType);
    
    void handleAssignment(BinOpNode* node, Context* context);
};

#endif