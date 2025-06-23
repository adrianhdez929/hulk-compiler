#ifndef CONTEXT_H
#define CONTEXT_H

#include <string>
#include <list>
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>

struct TypeAttribute;
struct TypeMethod;

class TypeInfo {
    public:
    TypeInfo(std::string name);

    std::string name;
    std::vector<TypeAttribute> attributes;
    std::vector<TypeMethod> methods;
    std::vector<std::shared_ptr<TypeInfo>> parents;

    TypeAttribute getAttribute(const std::string& name);
    TypeMethod getMethod(const std::string& name);
    bool hasAttribute(const std::string& name) const;
    bool hasMethod(const std::string& name) const;
    bool defineParent(const std::shared_ptr<TypeInfo>& parent);
    bool defineAttribute(const std::string& name, std::shared_ptr<TypeInfo> type);
    bool defineMethod(const std::string& name, std::shared_ptr<TypeInfo> returnType, 
                      const std::vector<TypeAttribute>& arguments);
    bool isCompatibleWith(const std::string& name) const;
};

struct TypeAttribute {
    std::string name;
    std::shared_ptr<TypeInfo> type;
};

struct TypeMethod {
    std::string name;
    std::shared_ptr<TypeInfo> returnType;
    std::vector<TypeAttribute> arguments;
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
    std::vector<TypeAttribute> paramTypes;
    
    FuncInfo(const std::string& n = "", int p = 0, std::shared_ptr<TypeInfo> ret = nullptr) 
        : name(n), params(p), returnType(ret) {}
};

class Context {
public:
    Context* parent;
    std::list<Context*> children;
    std::shared_ptr<TypeInfo> currentType;

    std::vector<VarInfo> localVars;
    std::vector<TypeMethod> localFuncs;
    std::unordered_map<std::string, std::shared_ptr<TypeInfo>> localTypes;

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
    // virtual bool isLocal(const std::string& funcname, const int argcount);

    // virtual bool define(const std::string& varname);
    // virtual bool define(const std::string& funcname, const int argcount);
    virtual VarInfo getLocal(const std::string& varname, VarInfo& var);
    // virtual FuncInfo getLocal(const std::string& funcname, const int argcount, FuncInfo& func);

    virtual bool defineType(const std::string& typeName);
    virtual bool isTypeDefined(const std::string& typeName);
    virtual std::shared_ptr<TypeInfo> getType(const std::string& typeName);
    
    virtual bool defineVar(const std::string& varname, std::shared_ptr<TypeInfo> type);
    virtual std::shared_ptr<TypeInfo> getVarType(const std::string& varname);
    
    virtual TypeMethod getFunc(const std::string& funcname, 
                                                const std::vector<std::shared_ptr<TypeInfo>>& paramTypes);
    virtual bool defineFunc(const std::string& funcname, std::shared_ptr<TypeInfo> returnType, 
                           const std::vector<TypeAttribute> paramTypes);
    virtual std::shared_ptr<TypeInfo> getFuncReturnType(const std::string& funcname, 
                                                        const std::vector<std::shared_ptr<TypeInfo>>& paramTypes);
        
    // virtual MethodMemberInfo* resolveMethod(std::shared_ptr<TypeInfo> objType, const std::string& methodName, 
    //                                    const std::vector<std::shared_ptr<TypeInfo>>& argTypes);
    
    virtual Context* createChildContext();
public:
    static void initializeBuiltinTypes();
    void initializeBuiltinFunctions();
};

#endif