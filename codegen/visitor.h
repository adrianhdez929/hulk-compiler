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
	void visit(TypeCastNode* node, Context* context) override;

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
    
    // VTable infrastructure for polymorphism
    std::map<std::string, std::vector<std::string>> typeMethodMap; // type -> method names in order
    std::map<std::string, llvm::GlobalVariable*> typeVTableMap; // type -> vtable global
    std::map<std::string, std::map<std::string, int>> methodIndexMap; // type -> method -> index
    std::map<std::string, llvm::Function*> methodFunctionMap; // "type_method" -> function
    
    // Add attribute type map for struct fields
    typedef std::map<std::string, llvm::Type*> AttributeTypeMap;
    std::map<std::string, AttributeTypeMap> typeAttributeTypeMap; 
    // Add inheritance map to track type hierarchies
    std::map<std::string, std::string> typeInheritanceMap; // child -> parent 
    // Map to store default values for each attribute in each type
    // Store raw values instead of LLVM Values to avoid context issues
    struct DefaultValue {
        enum Type { DOUBLE, STRING_ } type;
        double doubleVal;
        std::string stringVal;
        DefaultValue() : type(DOUBLE), doubleVal(0.0) {} // Default constructor
        DefaultValue(double val) : type(DOUBLE), doubleVal(val) {}
        DefaultValue(const std::string& val) : type(STRING_), stringVal(val) {}
    };
    std::map<std::string, std::map<std::string, DefaultValue>> typeDefaultValuesMap;
    
    void createStandardLibraryDeclarations();
    llvm::Function* createMainFunction();
    
    // VTable management
    void createVTable(const std::string& typeName);
    void addMethodToVTable(const std::string& typeName, const std::string& methodName, llvm::Function* methodFunc);
    llvm::Value* callVirtualMethod(llvm::Value* objectPtr, const std::string& methodName, 
                                  const std::vector<llvm::Value*>& args);
    
    // Function to perform proper vtable-based dynamic dispatch
    llvm::Value* callVirtualMethodWithVTableLookup(llvm::Value* objectPtr, const std::string& methodName, 
                                                   const std::vector<llvm::Value*>& args);
    
    // Function to determine runtime type from vtable pointer
    std::string getRuntimeType(llvm::Value* objectPtr);
    
    void generateMethodFunction(const std::string& typeName, AssignFuncNode* method, 
                               llvm::StructType* structType, const std::vector<std::string>& attributes);
    void generateForwardingMethod(const std::string& childTypeName, const std::string& parentTypeName,
                                 const std::string& methodName, llvm::StructType* structType);
    
    void handleAssignment(BinOpNode* node, Context* context);
    void handleStringConcatenation(llvm::Value* leftValue, llvm::Value* rightValue, BinOpNode* node, Context* context, bool space);
    
    // Helper method to check inheritance relationships
    bool isSubtypeOf(const std::string& childType, const std::string& parentType);
};

#endif