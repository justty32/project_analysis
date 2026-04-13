#ifndef VALUE_HPP
#define VALUE_HPP

#include <any>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <iostream>
#include <functional>
#include <memory>

struct Value;
class Environment;

using NativeFunc = std::function<Value(Environment&, const std::vector<Value>&)>;

struct Value {
    enum class Type {
        LIST,
        ARRAY,
        DICT,
        SET,
        STRING,
        NUMBER,
        ATOM,
        FUNC,
        MACRO
    };

    std::any data;
    Type type;

    template<typename T>
    T as() const { return std::any_cast<T>(data); }

    static Value makeNumber(double d) { return {d, Type::NUMBER}; }
    static Value makeString(const std::string& s) { return {s, Type::STRING}; }
    static Value makeAtom(const std::string& s) { return {s, Type::ATOM}; }
    static Value makeFunc(NativeFunc f) { return {f, Type::FUNC}; }
    static Value makeMacro(NativeFunc f) { return {f, Type::MACRO}; }
    
    static Value makeList(const std::list<Value>& v) { return {v, Type::LIST}; }
    static Value makeArray(const std::vector<Value>& v) { return {v, Type::ARRAY}; }
    static Value makeDict(const std::map<Value, Value>& m) { return {m, Type::DICT}; }
    static Value makeSet(const std::set<Value>& s) { return {s, Type::SET}; }
};

bool operator<(const Value& lhs, const Value& rhs);

#endif
