#include <string>

#ifndef CONTEXT_H
#define CONTEXT_H

class Context {
public:
    Context() = default;
    ~Context() = default;

    virtual bool isDefined(const std::string& varname);
    virtual bool isDefined(const std::string& funcname, const int argcount);

    virtual bool define(const std::string& varname);
    virtual bool define(const std::string& funcname, const int argcount);

    virtual Context* createChildContext();    
};

#endif