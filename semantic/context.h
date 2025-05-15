#include <string>
#include <list>
#include <vector>

#ifndef CONTEXT_H
#define CONTEXT_H

struct VarInfo {
    std::string name;
};

struct FuncInfo {
    std::string name;
    int params;
};

class Context {
public:
    Context* parent;
    std::list<Context*> children;

    std::vector<VarInfo> localVars;
    std::vector<FuncInfo> localFuncs;

    int parentVarIndex;
    int parentFuncIndex;

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

    virtual Context* createChildContext();    
};

#endif