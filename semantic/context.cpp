#include "context.h"

Context::Context(Context *parent)
{
    this->parent = parent;
    this->localVars = std::vector<VarInfo>();
    this->localFuncs = std::vector<FuncInfo>();

    if (parent != nullptr) {
        this->parentVarIndex = parent->localVars.size();
        this->parentFuncIndex = parent->localFuncs.size();
    } else {
        this->parentVarIndex = 0;
        this->parentFuncIndex = 0;
    }

    this->children = std::list<Context*>();
}

Context* Context::createChildContext()
{
    Context* child = new Context(this);
    this->children.push_back(child);
    return child;
}
bool Context::isDefined(const std::string &varname)
{
    for (int i = 0; i < this->localVars.size(); i++) {
        if (this->localVars[i].name == varname) {
            return true;
        }
    }
    if (this->parent != nullptr) {
        return this->parent->isDefined(varname);
    }
    return false;
}
bool Context::isDefined(const std::string &funcname, const int argcount)
{
    for (int i = 0; i < this->localFuncs.size(); i++) {
        if (this->localFuncs[i].name == funcname && this->localFuncs[i].params == argcount) {
            return true;
        }
    }
    if (this->parent != nullptr) {
        return this->parent->isDefined(funcname, argcount);
    }
    return false;
}

bool Context::isLocal(const std::string &varname)
{
    for (int i = 0; i < this->localVars.size(); i++) {
        if (this->localVars[i].name == varname) {
            return true;
        }
    }
    return false;
}
bool Context::isLocal(const std::string &funcname, const int argcount)
{
    for (int i = 0; i < this->localFuncs.size(); i++) {
        if (this->localFuncs[i].name == funcname && this->localFuncs[i].params == argcount) {
            return true;
        }
    }
    return false;
}

bool Context::define(const std::string &varname)
{
    if (this->isDefined(varname)) {
        return false;
    }
    VarInfo var;
    var.name = varname;
    this->localVars.push_back(var);
    return true;
}
bool Context::define(const std::string &funcname, const int argcount)
{
    if (this->isDefined(funcname, argcount)) {
        return false;
    }
    FuncInfo func;
    func.name = funcname;
    func.params = argcount;
    this->localFuncs.push_back(func);
    return true;
}

VarInfo Context::getLocal(const std::string &varname, VarInfo &var)
{
    for (int i = 0; i < this->localVars.size(); i++) {
        if (this->localVars[i].name == varname) {
            var = this->localVars[i];
            return var;
        }
    }
    if (this->parent != nullptr) {
        return this->parent->getLocal(varname, var);
    }
    return var;
}

FuncInfo Context::getLocal(const std::string &funcname, const int argcount, FuncInfo &func)
{
    for (int i = 0; i < this->localFuncs.size(); i++) {
        if (this->localFuncs[i].name == funcname && this->localFuncs[i].params == argcount) {
            func = this->localFuncs[i];
            return func;
        }
    }
    if (this->parent != nullptr) {
        return this->parent->getLocal(funcname, argcount, func);
    }
    return func;
}