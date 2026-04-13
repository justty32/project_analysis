#include "evaluator.hpp"
#include <stdexcept>
#include <iostream>

Value Evaluator::evaluate(const Value& val, Environment& env) {
    if (val.type == Value::Type::ATOM) {
        std::string name = val.as<std::string>();
        if (name == "nil") return val;
        return env.resolve(name);
    }
    if (val.type == Value::Type::LIST) {
        auto list = val.as<std::list<Value>>();
        if (list.empty()) return val;
        return evalList(list, env);
    }
    return val;
}

Value Evaluator::evalList(const std::list<Value>& list, Environment& env) {
    if (list.empty()) return Value::makeList({});

    Value first = evaluate(list.front(), env);

    if (first.type == Value::Type::FUNC) {
        std::vector<Value> args;
        auto it = std::next(list.begin());
        while (it != list.end()) {
            args.push_back(evaluate(*it, env));
            it++;
        }
        return first.as<Value::BuiltinFunc>()(env, args);
    }

    if (first.type == Value::Type::MACRO) {
        std::vector<Value> args;
        auto it = std::next(list.begin());
        while (it != list.end()) {
            args.push_back(*it);
            it++;
        }
        Value expanded = first.as<Value::BuiltinFunc>()(env, args);
        return evaluate(expanded, env);
    }

    // Default: return the list with evaluated elements
    std::list<Value> evaluated;
    for (const auto& item : list) {
        evaluated.push_back(evaluate(item, env));
    }
    return Value::makeList(evaluated);
}
