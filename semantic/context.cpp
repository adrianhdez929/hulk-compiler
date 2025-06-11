#include "context.h"
#include <iostream>
#include <algorithm>

std::shared_ptr<TypeInfo> Context::intType = nullptr;
std::shared_ptr<TypeInfo> Context::stringType = nullptr;
std::shared_ptr<TypeInfo> Context::boolType = nullptr;
std::shared_ptr<TypeInfo> Context::voidType = nullptr;
std::shared_ptr<TypeInfo> Context::objectType = nullptr;

bool TypeInfo::isCompatibleWith(const std::shared_ptr<TypeInfo>& other) const {
    if (name == other->name) return true;
    if (kind == TypeKind::INFERRED || other->kind == TypeKind::INFERRED) return true;
    return isSubtypeOf(other);
}

bool TypeInfo::isSubtypeOf(const std::shared_ptr<TypeInfo>& other) const {
    if (name == other->name) return true;
    if (typeDef && other->typeDef) {
        return typeDef->isSubclassOf(other->typeDef);
    }
    return false;
}

bool TypeDef::hasMethod(const std::string& methodName, const std::vector<std::shared_ptr<TypeInfo>>& paramTypes) const {
    for (const auto& method : methods) {
        if (method.name == methodName && method.paramTypes.size() == paramTypes.size()) {
            bool matches = true;
            for (size_t i = 0; i < paramTypes.size(); i++) {
                if (!paramTypes[i]->isCompatibleWith(method.paramTypes[i])) {
                    matches = false;
                    break;
                }
            }
            if (matches) return true;
        }
    }
    if (parentType && parentType->typeDef) {
        return parentType->typeDef->hasMethod(methodName, paramTypes);
    }
    return false;
}

MethodMemberInfo* TypeDef::findMethod(const std::string& methodName, const std::vector<std::shared_ptr<TypeInfo>>& paramTypes) {
    for (auto& method : methods) {
        if (method.name == methodName && method.paramTypes.size() == paramTypes.size()) {
            bool matches = true;
            for (size_t i = 0; i < paramTypes.size(); i++) {
                if (!paramTypes[i]->isCompatibleWith(method.paramTypes[i])) {
                    matches = false;
                    break;
                }
            }
            if (matches) return &method;
        }
    }
    if (parentType && parentType->typeDef) {
        return parentType->typeDef->findMethod(methodName, paramTypes);
    }
    return nullptr;
}

bool TypeDef::hasProperty(const std::string& propName) const {
    for (const auto& prop : properties) {
        if (prop.name == propName) return true;
    }
    if (parentType && parentType->typeDef) {
        return parentType->typeDef->hasProperty(propName);
    }
    return false;
}

PropertyMemberInfo* TypeDef::findProperty(const std::string& propName) {
    for (auto& prop : properties) {
        if (prop.name == propName) return &prop;
    }
    if (parentType && parentType->typeDef) {
        return parentType->typeDef->findProperty(propName);
    }
    return nullptr;
}

bool TypeDef::isSubclassOf(const std::shared_ptr<TypeDef>& other) const {
    if (name == other->name) return true;
    if (parentType && parentType->typeDef) {
        return parentType->typeDef->isSubclassOf(other);
    }
    return false;
}

Context::Context(Context *parent)
{
    this->parent = parent;
    this->localVars = std::vector<VarInfo>();
    this->localFuncs = std::vector<FuncInfo>();
    this->localTypes = std::unordered_map<std::string, std::shared_ptr<TypeDef>>();

    if (parent != nullptr) {
        this->parentVarIndex = parent->localVars.size();
        this->parentFuncIndex = parent->localFuncs.size();
    } else {
        this->parentVarIndex = 0;
        this->parentFuncIndex = 0;
    }

    this->children = std::list<Context*>();
    
    if (parent == nullptr) {
        initializeBuiltinTypes();
    }
}

void Context::initializeBuiltinTypes() {
    if (intType == nullptr) {
        intType = std::make_shared<TypeInfo>("int", TypeKind::PRIMITIVE);
        stringType = std::make_shared<TypeInfo>("string", TypeKind::PRIMITIVE);
        boolType = std::make_shared<TypeInfo>("bool", TypeKind::PRIMITIVE);
        voidType = std::make_shared<TypeInfo>("void", TypeKind::PRIMITIVE);
        
        auto objectTypeDef = std::make_shared<TypeDef>("Object");
        objectType = std::make_shared<TypeInfo>("Object", TypeKind::CLASS);
        objectType->typeDef = objectTypeDef;
    }
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
    std::cout << "Debug: Searching for function '" << funcname << "' with " << argcount << " params in context " << this << std::endl;
    std::cout << "Debug: Context has " << this->localFuncs.size() << " local functions" << std::endl;
    
    for (int i = 0; i < this->localFuncs.size(); i++) {
        std::cout << "Debug: Found function '" << this->localFuncs[i].name << "' with " << this->localFuncs[i].params << " params" << std::endl;
        if (this->localFuncs[i].name == funcname && this->localFuncs[i].params == argcount) {
            std::cout << "Debug: Match found!" << std::endl;
            return true;
        }
    }
    if (this->parent != nullptr) {
        std::cout << "Debug: Checking parent context" << std::endl;
        return this->parent->isDefined(funcname, argcount);
    }
    std::cout << "Debug: Function not found" << std::endl;
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
    VarInfo var(varname);
    this->localVars.push_back(var);
    return true;
}
bool Context::define(const std::string &funcname, const int argcount)
{
    std::cout << "Debug: define() called for '" << funcname << "' with " << argcount << " params in context " << this << std::endl;
    if (this->isDefined(funcname, argcount)) {
        std::cout << "Debug: Function already exists, returning false" << std::endl;
        return false;
    }
    FuncInfo func(funcname, argcount);
    this->localFuncs.push_back(func);
    std::cout << "Debug: Function added successfully. Context now has " << this->localFuncs.size() << " functions" << std::endl;
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

bool Context::defineType(const std::string& typeName, std::shared_ptr<TypeDef> typeDef) {
    if (isTypeDefined(typeName)) {
        return false;
    }
    localTypes[typeName] = typeDef;
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

std::shared_ptr<TypeDef> Context::getTypeDef(const std::string& typeName) {
    auto it = localTypes.find(typeName);
    if (it != localTypes.end()) {
        return it->second;
    }
    if (parent != nullptr) {
        return parent->getTypeDef(typeName);
    }
    return nullptr;
}

std::shared_ptr<TypeInfo> Context::getType(const std::string& typeName) {
    if (typeName == "int") return intType;
    if (typeName == "string") return stringType;
    if (typeName == "bool") return boolType;
    if (typeName == "void") return voidType;
    if (typeName == "Object") return objectType;
    
    auto typeDef = getTypeDef(typeName);
    if (typeDef) {
        auto typeInfo = std::make_shared<TypeInfo>(typeName, TypeKind::CLASS);
        typeInfo->typeDef = typeDef;
        return typeInfo;
    }
    return nullptr;
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

bool Context::defineFunc(const std::string& funcname, std::shared_ptr<TypeInfo> returnType, 
                        const std::vector<std::shared_ptr<TypeInfo>>& paramTypes) {
    if (isLocal(funcname, paramTypes.size())) {
        return false;
    }
    FuncInfo func(funcname, paramTypes.size(), returnType);
    func.paramTypes = paramTypes;
    localFuncs.push_back(func);
    return true;
}

std::shared_ptr<TypeInfo> Context::getFuncReturnType(const std::string& funcname, 
                                                     const std::vector<std::shared_ptr<TypeInfo>>& paramTypes) {
    for (const auto& func : localFuncs) {
        if (func.name == funcname && func.paramTypes.size() == paramTypes.size()) {
            bool matches = true;
            for (size_t i = 0; i < paramTypes.size(); i++) {
                if (!paramTypes[i]->isCompatibleWith(func.paramTypes[i])) {
                    matches = false;
                    break;
                }
            }
            if (matches) return func.returnType;
        }
    }
    if (parent != nullptr) {
        return parent->getFuncReturnType(funcname, paramTypes);
    }
    return nullptr;
}

std::shared_ptr<TypeInfo> Context::inferType(const std::string& expression) {
    return std::make_shared<TypeInfo>("inferred", TypeKind::INFERRED);
}

std::shared_ptr<TypeInfo> Context::unifyTypes(std::shared_ptr<TypeInfo> type1, std::shared_ptr<TypeInfo> type2) {
    if (type1->name == type2->name) {
        return type1;
    }
    if (type1->kind == TypeKind::INFERRED) {
        return type2;
    }
    if (type2->kind == TypeKind::INFERRED) {
        return type1;
    }
    return findCommonSupertype(type1, type2);
}

std::shared_ptr<TypeInfo> Context::findCommonSupertype(std::shared_ptr<TypeInfo> type1, std::shared_ptr<TypeInfo> type2) {
    if (type1->name == type2->name) {
        return type1;
    }
    if (isSubtype(type1, type2)) {
        return type2;
    }
    if (isSubtype(type2, type1)) {
        return type1;
    }
    return objectType;
}

bool Context::isSubtype(std::shared_ptr<TypeInfo> subtype, std::shared_ptr<TypeInfo> supertype) {
    if (subtype->name == supertype->name) {
        return true;
    }
    
    if (supertype->name == "Object") {
        return true;
    }
    
    if (supertype->name == "Number" || supertype->name == "String" || supertype->name == "Boolean") {
        return subtype->name == supertype->name;
    }
    
    if (subtype->typeDef && supertype->typeDef) {
        return subtype->typeDef->isSubclassOf(supertype->typeDef);
    }
    
    return false;
}

bool Context::canAssign(std::shared_ptr<TypeInfo> fromType, std::shared_ptr<TypeInfo> toType) {
    if (fromType->name == toType->name) return true;
    if (toType->kind == TypeKind::INFERRED) return true;
    return isSubtype(fromType, toType);
}

MethodMemberInfo* Context::resolveMethod(std::shared_ptr<TypeInfo> objType, const std::string& methodName, 
                                    const std::vector<std::shared_ptr<TypeInfo>>& argTypes) {
    if (objType->typeDef) {
        return objType->typeDef->findMethod(methodName, argTypes);
    }
    return nullptr;
}

std::shared_ptr<TypeInfo> Context::instantiateGenericType(std::shared_ptr<TypeInfo> genericType, 
                                                         const std::vector<std::shared_ptr<TypeInfo>>& typeArgs) {
    if (genericType->kind != TypeKind::GENERIC) {
        return genericType;
    }
    
    auto instantiated = std::make_shared<TypeInfo>(genericType->name, TypeKind::CLASS);
    instantiated->typeDef = genericType->typeDef;
    instantiated->genericArgs = typeArgs;
    return instantiated;
}

bool Context::matchesGenericConstraints(std::shared_ptr<TypeInfo> type, const std::string& genericParam) {
    return true;
}