#ifndef CONTEXT_H
#define CONTEXT_H

#include <string>
#include <list>
#include <vector>
#include <unordered_map>
#include <memory>

struct TypeInfo;
struct TypeDef;

enum class TypeKind {
    PRIMITIVE,    
    CLASS,        
    FUNCTION,     
    GENERIC,      
    INFERRED      
};

struct TypeInfo {
    std::string name;
    TypeKind kind;
    std::shared_ptr<TypeDef> typeDef;
    std::vector<std::shared_ptr<TypeInfo>> genericArgs;
    
    TypeInfo(const std::string& n, TypeKind k = TypeKind::PRIMITIVE) 
        : name(n), kind(k) {}
    
    bool isCompatibleWith(const std::shared_ptr<TypeInfo>& other) const;
    bool isSubtypeOf(const std::shared_ptr<TypeInfo>& other) const;
};

struct MethodMemberInfo {
    std::string name;
    std::shared_ptr<TypeInfo> returnType;
    std::vector<std::shared_ptr<TypeInfo>> paramTypes;
    bool isVirtual;
    
    MethodMemberInfo(const std::string& n, std::shared_ptr<TypeInfo> ret, 
                 std::vector<std::shared_ptr<TypeInfo>> params, bool virt = false)
        : name(n), returnType(ret), paramTypes(params), isVirtual(virt) {}
};

struct PropertyMemberInfo {
    std::string name;
    std::shared_ptr<TypeInfo> type;
    
    PropertyMemberInfo(const std::string& n, std::shared_ptr<TypeInfo> t)
        : name(n), type(t) {}
};

struct TypeDef {
    std::string name;
    std::shared_ptr<TypeInfo> parentType; 
    std::vector<MethodMemberInfo> methods;
    std::vector<PropertyMemberInfo> properties;
    std::vector<std::string> genericParams;
    
    TypeDef(const std::string& n) : name(n) {}
    
    bool hasMethod(const std::string& methodName, const std::vector<std::shared_ptr<TypeInfo>>& paramTypes) const;
    MethodMemberInfo* findMethod(const std::string& methodName, const std::vector<std::shared_ptr<TypeInfo>>& paramTypes);
    bool hasProperty(const std::string& propName) const;
    PropertyMemberInfo* findProperty(const std::string& propName);
    bool isSubclassOf(const std::shared_ptr<TypeDef>& other) const;
};

struct VarInfo {
    std::string name;
    std::shared_ptr<TypeInfo> type;
    
    VarInfo(const std::string& n = "", std::shared_ptr<TypeInfo> t = nullptr) 
        : name(n), type(t) {}
};

struct FuncInfo {
    std::string name;
    int params;
    std::shared_ptr<TypeInfo> returnType;
    std::vector<std::shared_ptr<TypeInfo>> paramTypes;
    
    FuncInfo(const std::string& n = "", int p = 0, std::shared_ptr<TypeInfo> ret = nullptr) 
        : name(n), params(p), returnType(ret) {}
};

struct MethodInfo {
        std::string name;
        std::shared_ptr<TypeInfo> returnType;
        std::vector<std::shared_ptr<TypeInfo>> paramTypes;
        std::vector<std::string> paramNames;
        std::string ownerType;
        bool isInherited;
        int line;
        
        MethodInfo(const std::string& n, std::shared_ptr<TypeInfo> ret, 
                  const std::vector<std::shared_ptr<TypeInfo>>& params,
                  const std::vector<std::string>& paramNames,
                  const std::string& owner, bool inherited = false, int l = 0)
            : name(n), returnType(ret), paramTypes(params), paramNames(paramNames),
              ownerType(owner), isInherited(inherited), line(l) {}
    };

    struct AttributeInfo {
        std::string name;
        std::shared_ptr<TypeInfo> type;
        std::string ownerType;
        bool isInherited;
        int line;
        
        AttributeInfo(const std::string& n, std::shared_ptr<TypeInfo> t, 
                     const std::string& owner, bool inherited = false, int l = 0)
            : name(n), type(t), ownerType(owner), isInherited(inherited), line(l) {}
    };

    struct VariableInfo {
        std::string name;
        std::shared_ptr<TypeInfo> type;
        std::string scope;
        bool isParameter;
        int line;
        
        VariableInfo(const std::string& n, std::shared_ptr<TypeInfo> t, 
                    const std::string& s, bool param = false, int l = 0)
            : name(n), type(t), scope(s), isParameter(param), line(l) {}
    };

class Context {
public:
    Context* parent;
    std::list<Context*> children;
    std::shared_ptr<TypeInfo> currentType;

    std::vector<VarInfo> localVars;
    std::vector<FuncInfo> localFuncs;
    std::unordered_map<std::string, std::shared_ptr<TypeDef>> localTypes;

    std::unordered_map<std::string, std::vector<MethodInfo>> methodsByType;
    std::unordered_map<std::string, std::vector<AttributeInfo>> attributesByType;
    std::vector<VariableInfo> globalVariables;

    int parentVarIndex;
    int parentFuncIndex;

    static std::shared_ptr<TypeInfo> numberType;
    static std::shared_ptr<TypeInfo> stringType;
    static std::shared_ptr<TypeInfo> boolType;
    static std::shared_ptr<TypeInfo> voidType;
    static std::shared_ptr<TypeInfo> objectType;

    Context(Context* parent);
    ~Context() = default;

    virtual bool isDefined(const std::string& varname);
    virtual bool isDefined(const std::string& funcname, const int argcount);
    virtual bool isLocal(const std::string& varname);
    virtual bool isLocal(const std::string& funcname, const int argcount);

    virtual bool define(const std::string& varname);
    virtual bool define(const std::string& funcname, const int argcount);
    virtual VarInfo getLocal(const std::string& varname, VarInfo& var);
    virtual FuncInfo getLocal(const std::string& funcname, const int argcount, FuncInfo& func);

    virtual bool defineType(const std::string& typeName, std::shared_ptr<TypeDef> typeDef);
    virtual bool isTypeDefined(const std::string& typeName);
    virtual std::shared_ptr<TypeDef> getTypeDef(const std::string& typeName);
    virtual std::shared_ptr<TypeInfo> getType(const std::string& typeName);
    
    virtual bool defineVar(const std::string& varname, std::shared_ptr<TypeInfo> type);
    virtual std::shared_ptr<TypeInfo> getVarType(const std::string& varname);
    
    virtual bool defineFunc(const std::string& funcname, std::shared_ptr<TypeInfo> returnType, 
                           const std::vector<std::shared_ptr<TypeInfo>>& paramTypes);
    virtual std::shared_ptr<TypeInfo> getFuncReturnType(const std::string& funcname, 
                                                        const std::vector<std::shared_ptr<TypeInfo>>& paramTypes);
    
    virtual std::shared_ptr<TypeInfo> inferType(const std::string& expression);
    virtual std::shared_ptr<TypeInfo> unifyTypes(std::shared_ptr<TypeInfo> type1, std::shared_ptr<TypeInfo> type2);
    virtual std::shared_ptr<TypeInfo> findCommonSupertype(std::shared_ptr<TypeInfo> type1, std::shared_ptr<TypeInfo> type2);
    
    virtual bool isSubtype(std::shared_ptr<TypeInfo> subtype, std::shared_ptr<TypeInfo> supertype);
    virtual bool canAssign(std::shared_ptr<TypeInfo> fromType, std::shared_ptr<TypeInfo> toType);
    virtual MethodMemberInfo* resolveMethod(std::shared_ptr<TypeInfo> objType, const std::string& methodName, 
                                       const std::vector<std::shared_ptr<TypeInfo>>& argTypes);
    
    virtual std::shared_ptr<TypeInfo> instantiateGenericType(std::shared_ptr<TypeInfo> genericType, 
                                                             const std::vector<std::shared_ptr<TypeInfo>>& typeArgs);
    virtual bool matchesGenericConstraints(std::shared_ptr<TypeInfo> type, const std::string& genericParam);

    virtual Context* createChildContext();
    
    // Symbol collector integration - store collected symbol information
    struct CollectedMethodInfo {
        std::string name;
        std::shared_ptr<TypeInfo> returnType;
        std::vector<std::shared_ptr<TypeInfo>> paramTypes;
        std::vector<std::string> paramNames;
        std::string ownerType;
        bool isInherited;
        int line;
        
        CollectedMethodInfo(const std::string& n, std::shared_ptr<TypeInfo> ret, 
                           const std::vector<std::shared_ptr<TypeInfo>>& params,
                           const std::vector<std::string>& paramNames,
                           const std::string& owner, bool inherited = false, int l = 0)
            : name(n), returnType(ret), paramTypes(params), paramNames(paramNames),
              ownerType(owner), isInherited(inherited), line(l) {}
    };

    struct CollectedAttributeInfo {
        std::string name;
        std::shared_ptr<TypeInfo> type;
        std::string ownerType;
        bool isInherited;
        int line;
        
        CollectedAttributeInfo(const std::string& n, std::shared_ptr<TypeInfo> t, 
                              const std::string& owner, bool inherited = false, int l = 0)
            : name(n), type(t), ownerType(owner), isInherited(inherited), line(l) {}
    };

    struct CollectedVariableInfo {
        std::string name;
        std::shared_ptr<TypeInfo> type;
        std::string scope;
        bool isParameter;
        int line;
        
        CollectedVariableInfo(const std::string& n, std::shared_ptr<TypeInfo> t, 
                             const std::string& s, bool param = false, int l = 0)
            : name(n), type(t), scope(s), isParameter(param), line(l) {}
    };

public:
    
    // Methods to query stored symbol collector data
    virtual std::vector<MethodInfo> getMethodsForType(const std::string& typeName, bool includeInherited = true) const;
    virtual std::vector<AttributeInfo> getAttributesForType(const std::string& typeName, bool includeInherited = true) const;
    virtual MethodInfo* findMethod(const std::string& typeName, const std::string& methodName, 
                                                     const std::vector<std::shared_ptr<TypeInfo>>& paramTypes) const;
    virtual AttributeInfo* findAttribute(const std::string& typeName, const std::string& attrName) const;
    virtual const std::vector<VariableInfo>& getGlobalVariables() const;
    
    static void initializeBuiltinTypes();
    void initializeBuiltinFunctions();
};

#endif