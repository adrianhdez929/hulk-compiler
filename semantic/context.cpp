#include "context.h"
#include <iostream>
#include <algorithm>

std::shared_ptr<TypeInfo> Context::numberType = nullptr;
std::shared_ptr<TypeInfo> Context::stringType = nullptr;
std::shared_ptr<TypeInfo> Context::boolType = nullptr;
std::shared_ptr<TypeInfo> Context::voidType = nullptr;
std::shared_ptr<TypeInfo> Context::objectType = nullptr;

void Context::initializeBuiltinTypes() {
    if (numberType == nullptr) {
        numberType = std::make_shared<TypeInfo>("Number");
        stringType = std::make_shared<TypeInfo>("String");  
        boolType = std::make_shared<TypeInfo>("Boolean");
        voidType = std::make_shared<TypeInfo>("Expression");
        objectType = std::make_shared<TypeInfo>("Object");
    }
}


TypeInfo::TypeInfo(std::string name) : name(std::move(name)) {
    this->attributes = std::vector<TypeAttribute>();
    this->methods = std::vector<TypeMethod>();
    this->parents = std::vector<std::shared_ptr<TypeInfo>>();
}

TypeAttribute TypeInfo::getAttribute(const std::string& name) {
    for (const auto& attr : this->attributes) {
        if (attr.name == name) {
            return attr; // Return the found attribute
        }
    }
    for (const auto& parent : this->parents) {
        if (parent) {
            TypeAttribute parentAttr = parent->getAttribute(name);
            if (!parentAttr.name.empty()) {
                return parentAttr; // Return attribute from parent type if found
            }
        }
    }
    return TypeAttribute{"", nullptr}; // Return an empty attribute if not found
}

bool TypeInfo::hasAttribute(const std::string& name) const {
    bool exists = std::any_of(attributes.begin(), attributes.end(), 
                       [&name](const TypeAttribute& attr) { return attr.name == name; });
    if (!exists) {
        for (const auto& parent : parents) {
            if (parent && parent->hasAttribute(name)) {
                return true; // Check in parent types
            }
        }
    }
    return exists; // Return true if attribute exists in this type or its parents
}

TypeMethod TypeInfo::getMethod(const std::string& name) {
    for (const auto& method : this->methods) {
        if (method.name == name) {
            return method; // Return the found method
        }
    }
    for (const auto& parent : this->parents) {
        if (parent) {
            TypeMethod parentMethod = parent->getMethod(name);
            if (!parentMethod.name.empty()) {
                return parentMethod; // Return method from parent type if found
            }
        }
    }
    return TypeMethod{"", nullptr, {}}; // Return an empty method if not found
}

bool TypeInfo::hasMethod(const std::string& name) const {
    bool exists = std::any_of(methods.begin(), methods.end(), 
                       [&name](const TypeMethod& method) { return method.name == name; });
    if (!exists) {
        for (const auto& parent : parents) {
            if (parent && parent->hasMethod(name)) {
                return true; // Check in parent types
            }
        }
    }
    return exists; // Return true if method exists in this type or its parents
}

bool TypeInfo::isCompatibleWith(const std::string& name) const {
    if (this->name == name) return true;

    for (std::shared_ptr<TypeInfo> parent : this->parents) {
        if (parent && parent->name == name) return true;
        if (parent && parent->isCompatibleWith(name)) return true;
    }

    return false;
}

bool TypeInfo::defineAttribute(const std::string& name, std::shared_ptr<TypeInfo> type) {
    for (const auto& attr : this->attributes) {
        if (attr.name == name) {
            std::cerr << "Attribute '" << name << "' already defined in type '" << this->name << "'" << std::endl;
            return false; // Attribute already exists
        }
    }
    
    TypeAttribute newAttr{name, type};
    this->attributes.push_back(newAttr);
    return true;
}

bool TypeInfo::defineParent(const std::shared_ptr<TypeInfo>& parent) {
    if (parent == nullptr) {
        std::cout << "Cannot define null parent for type '" << this->name << "'" << std::endl;
        return false; // Cannot define null parent
    }

    // Check if parent is already added
    for (const auto& existingParent : this->parents) {
        if (existingParent->name == parent->name) {
            std::cout << "Parent '" << parent->name << "' already exists for type '" << this->name << "'" << std::endl;
            return false; // Parent already exists
        }
    }

    // Check for circular inheritance - but exclude self-reference check
    if (this->name != parent->name && parent->isCompatibleWith(this->name)) {
        std::cout << "Circular inheritance detected when trying to add parent '" 
                  << parent->name << "' to type '" << this->name << "'" << std::endl;
        return false; // Circular inheritance
    }

    this->parents.push_back(parent);
    std::cout << "Debug: Added parent '" << parent->name << "' to type '" << this->name 
              << "'. Parents count: " << this->parents.size() << std::endl;
    return true;
}

bool TypeInfo::defineMethod(const std::string& name, std::shared_ptr<TypeInfo> returnType, 
    const std::vector<TypeAttribute>& arguments) {
    for (const auto& method : this->methods) {
        if (method.name == name) {
            std::cerr << "Method '" << name << "' already defined in type '" << this->name << "'" << std::endl;
            return false; // Method already exists
        }
    }

    TypeMethod newMethod{name, returnType, arguments};
    this->methods.push_back(newMethod);
    return true;
}

Context::Context(Context *parent)
{
    // Initialize builtin types first
    initializeBuiltinTypes();
    
    this->parent = parent;
    this->localVars = std::vector<VarInfo>();
    this->localFuncs = std::vector<TypeMethod>();
    this->localTypes = std::unordered_map<std::string, std::shared_ptr<TypeInfo>>();

    if (parent != nullptr) {
        this->parentVarIndex = parent->localVars.size();
        this->parentFuncIndex = parent->localFuncs.size();
    } else {
        this->parentVarIndex = 0;
        this->parentFuncIndex = 0;
    }

    this->children = std::list<Context*>();
    
    if (parent == nullptr) {
        initializeBuiltinFunctions();
    }
}

void Context::initializeBuiltinFunctions() {
    // Ensure builtin types are initialized before using them
    if (numberType == nullptr) {
        initializeBuiltinTypes();
    }
    
    // Mathematical functions with one parameter
    std::vector<std::string> singleParamFuncs = {"sqrt", "sin", "cos", "exp"};
    for (const auto& funcName : singleParamFuncs) {
        defineFunc(funcName, numberType, {{"x", numberType}});
    }
    
    // Log function with two parameters (base, value)
    defineFunc("log", numberType, {{"x", numberType}, {"y", numberType}});
    
    // Random function with no parameters
    defineFunc("rand", numberType, {});
}

Context* Context::createChildContext()
{
    Context* child = new Context(this);
    child->currentType = this->currentType; // Inherit current type context
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
    // Simple search without excessive debugging
    for (int i = 0; i < this->localFuncs.size(); i++) {
        if (!this->localFuncs[i].name.empty() && 
            this->localFuncs[i].name == funcname && 
            this->localFuncs[i].arguments.size() == argcount) {
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
// bool Context::isLocal(const std::string &funcname, const int argcount)
// {
//     for (int i = 0; i < this->localFuncs.size(); i++) {
//         if (this->localFuncs[i].name == funcname && this->localFuncs[i].params == argcount) {
//             return true;
//         }
//     }
//     return false;
// }

// bool Context::define(const std::string &varname)
// {
//     if (this->isDefined(varname)) {
//         return false;
//     }
//     VarInfo var(varname);
//     this->localVars.push_back(var);
//     return true;
// }
// bool Context::define(const std::string &funcname, const int argcount)
// {
//     std::cout << "Debug: define() called for '" << funcname << "' with " << argcount << " params in context " << this << std::endl;
//     if (this->isDefined(funcname, argcount)) {
//         std::cout << "Debug: Function already exists, returning false" << std::endl;
//         return false;
//     }
//     TypeMethod func(funcname, argcount);
//     this->localFuncs.push_back(func);
//     std::cout << "Debug: Function added successfully. Context now has " << this->localFuncs.size() << " functions" << std::endl;
//     return true;
// }

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

// FuncInfo Context::getLocal(const std::string &funcname, const int argcount, FuncInfo &func)
// {
//     for (int i = 0; i < this->localFuncs.size(); i++) {
//         if (this->localFuncs[i].name == funcname && this->localFuncs[i].arguments.size() == argcount) {
//             func = this->localFuncs[i];
//             return func;
//         }
//     }
//     if (this->parent != nullptr) {
//         return this->parent->getLocal(funcname, argcount, func);
//     }
//     return func;
// }

bool Context::defineType(const std::string& typeName) {
    if (isTypeDefined(typeName)) {
        return false;
    }
    localTypes[typeName] = std::make_shared<TypeInfo>(typeName);
    return true;
}

bool Context::isTypeDefined(const std::string& typeName) {
    if (localTypes.find(typeName) != localTypes.end()) {
        return true;
    }
    if (parent != nullptr) {
        return parent->isTypeDefined(typeName);
    }
    return false;
}

std::shared_ptr<TypeInfo> Context::getType(const std::string& typeName) {    
    // std::cout << "Debug: getType() called for '" << typeName << "' in context " << this << std::endl;
    if (typeName.empty()) {
        std::cout << "Warning: Empty typeName in getType()" << std::endl;
        return objectType;
    }
        
    if (typeName == "Number") return numberType;
    if (typeName == "String") return stringType;
    if (typeName == "Boolean") return boolType;
    if (typeName == "Expression") return voidType;
    if (typeName == "Object") return objectType;
    
    auto localType = localTypes.find(typeName);
    if (localType != localTypes.end()) {
        return localType->second;
    }
    if (parent != nullptr) {
        return parent->getType(typeName);
    }

    return objectType;
}

bool Context::defineVar(const std::string& varname, std::shared_ptr<TypeInfo> type) {
    if (isLocal(varname)) {
        return false;
    }
    VarInfo var(varname, type);
    localVars.push_back(var);
    return true;
}

std::shared_ptr<TypeInfo> Context::getVarType(const std::string& varname) {
    for (const auto& var : localVars) {
        if (var.name == varname) {
            return var.type;
        }
    }
    if (parent != nullptr) {
        return parent->getVarType(varname);
    }
    return nullptr;
}

TypeMethod Context::getFunc(const std::string& funcname, 
                                              const std::vector<std::shared_ptr<TypeInfo>>& paramTypes) {
    for (const auto& func : localFuncs) {
        if (func.name == funcname && func.arguments.size() == paramTypes.size()) {
            bool matches = true;
            for (size_t i = 0; i < paramTypes.size(); i++) {
                if (!paramTypes[i]->isCompatibleWith(func.arguments[i].name)) {
                    matches = false;
                    break;
                }
            }
            if (matches) {
                return {func.name, func.returnType, func.arguments};
            }
        }
    }
    if (parent != nullptr) {
        return parent->getFunc(funcname, paramTypes);
    }
    return {"", nullptr, {}};
}

bool Context::defineFunc(const std::string& funcname, std::shared_ptr<TypeInfo> returnType, 
                        const std::vector<TypeAttribute> paramTypes) {
    if (funcname.empty() || returnType == nullptr) {
        std::cerr << "Function name or return type cannot be empty or null." << std::endl;
        return false; // Invalid function definition
    }
    std::cout << "defining function " << funcname << std::endl;
    if (isDefined(funcname, paramTypes.size())) {
        std::cerr << "Function '" << funcname << "' with " << paramTypes.size() 
                  << " parameters already defined in this context." << std::endl;
        return false; // Function already exists
    }
    TypeMethod func = {funcname, returnType, paramTypes};
    localFuncs.push_back(func);
    return true;
}

std::shared_ptr<TypeInfo> Context::getFuncReturnType(const std::string& funcname, 
                                                     const std::vector<std::shared_ptr<TypeInfo>>& paramTypes) {
    // Robust safety checks to prevent crashes from corrupted data
    try {
        // Check if funcname is valid
        if (funcname.empty() || funcname.size() > 1000) {
            return nullptr;
        }
        
        // Check for printable characters
        for (char c : funcname) {
            if (c < 32 || c > 126) {
                return nullptr;
            }
        }
    } catch (...) {
        return nullptr;
    }
    
    // Ensure builtin types are initialized
    if (numberType == nullptr) {
        initializeBuiltinTypes();
    }
    
    // Safely iterate through functions
    try {
        for (const auto& func : localFuncs) {
            if (func.name.empty() || func.returnType == nullptr) {
                continue;
            }
            
            if (func.name == funcname && func.arguments.size() == paramTypes.size()) {
                bool matches = true;
                for (size_t i = 0; i < paramTypes.size(); i++) {
                    if (paramTypes[i] == nullptr || func.arguments[i].type == nullptr) {
                        matches = false;
                        break;
                    }
                    if (!paramTypes[i]->isCompatibleWith(func.arguments[i].type->name)) {
                        matches = false;
                        break;
                    }
                }
                if (matches) {
                    return func.returnType;
                }
            }
        }
    } catch (...) {
        return nullptr;
    }
    
    if (parent != nullptr) {
        return parent->getFuncReturnType(funcname, paramTypes);
    }
    
    return nullptr;
}
