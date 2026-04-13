#include "evaluator.hpp"
#include <stdexcept>
#include <iostream>
#include <fstream>

// Helper to open/append to output.cpp
void appendToOutput(const std::string& code) {
    std::ofstream file("output.cpp", std::ios::app);
    if (file.is_open()) {
        file << code << "\n";
    }
}

Value Evaluator::evaluate(const Value& val, Environment& env) {
    if (val.type == Value::Type::ATOM) {
        std::string name = val.as<std::string>();
        if (name == "nil") return Value::makeNil();
        
        // Try to parse as number
        try {
            size_t pos;
            double d = std::stod(name, &pos);
            if (pos == name.length()) return Value::makeNumber(d);
        } catch (...) {}

        try {
            return env.get(name);
        } catch (...) {
            return val; 
        }
    }
    if (val.type == Value::Type::LIST) {
        auto list = val.as<std::list<Value>>();
        if (list.empty()) return val;
        
        // Check for (quote ...) shortcut
        Value first = list.front();
        if (first.type == Value::Type::ATOM && first.as<std::string>() == "quote") {
            if (list.size() < 2) return Value::makeNil();
            return *std::next(list.begin());
        }

        Value result = evalList(list, env);
        if (result.type == Value::Type::STRING) {
            appendToOutput(result.as<std::string>());
        }
        return result;
    }
    return val;
}

Value Evaluator::evalList(const std::list<Value>& list, Environment& env) {
    if (list.empty()) return Value::makeList({});

    Value first = evaluate(list.front(), env);

    // Native Function Call
    if (first.type == Value::Type::FUNC) {
        std::vector<Value> args;
        auto it = std::next(list.begin());
        while (it != list.end()) {
            args.push_back(evaluate(*it, env));
            it++;
        }
        return first.as<NativeFunc>()(env, args);
    }

    // Native Macro Call (Arguments are not evaluated before call)
    if (first.type == Value::Type::MACRO) {
        std::vector<Value> args;
        auto it = std::next(list.begin());
        while (it != list.end()) {
            args.push_back(*it); // Literal arguments
            it++;
        }
        Value expanded = first.as<NativeFunc>()(env, args);
        return evaluate(expanded, env); // Evaluate the result of macro
    }

    // Default: Return the list with evaluated elements
    std::list<Value> evaluated;
    for (const auto& item : list) {
        evaluated.push_back(evaluate(item, env));
    }
    Value result = Value::makeList(evaluated);
    
    // If a list returns a string, append it to output.cpp
    if (result.type == Value::Type::STRING) {
        appendToOutput(result.as<std::string>());
    }
    
    return result;
}

// Builtin Functions Implementation
Value builtin_def(Environment& env, const std::vector<Value>& args) {
    if (args.size() < 2) throw std::runtime_error("def requires name and value");
    std::string name;
    if (args[0].type == Value::Type::ATOM || args[0].type == Value::Type::STRING) {
        name = args[0].as<std::string>();
    } else {
        throw std::runtime_error("def requires atom or string as name");
    }
    env.define(name, args[1]);
    return args[1];
}

Value builtin_get(Environment& env, const std::vector<Value>& args) {
    if (args.size() == 0) return Value::makeNil();
    
    Value target = args[0];
    if (args.size() == 1) {
        // (get symbol) -> lookup in env
        if (target.type == Value::Type::ATOM) return env.get(target.as<std::string>());
        if (target.type == Value::Type::STRING) return env.get(target.as<std::string>());
        return target;
    }
    
    // (get list/array index)
    Value key = args[1];
    if (target.type == Value::Type::LIST) {
        auto l = target.as<std::list<Value>>();
        int idx = (int)key.as<double>();
        if (idx < 0 || idx >= (int)l.size()) return Value::makeNil();
        auto it = l.begin();
        std::advance(it, idx);
        return *it;
    }
    if (target.type == Value::Type::ARRAY) {
        auto v = target.as<std::vector<Value>>();
        int idx = (int)key.as<double>();
        if (idx < 0 || idx >= (int)v.size()) return Value::makeNil();
        return v[idx];
    }
    if (target.type == Value::Type::DICT) {
        auto m = target.as<std::map<Value, Value>>();
        if (m.count(key)) return m[key];
        return Value::makeNil();
    }
    
    return Value::makeNil();
}

Value builtin_exec(Environment& env, const std::vector<Value>& args) {
    if (args.empty()) return Value::makeNil();
    return Evaluator::evaluate(args[0], env);
}

Value builtin_make(Environment& env, const std::vector<Value>& args) {
    if (args.empty()) return Value::makeNil();
    std::string typeName = args[0].as<std::string>();
    
    if (typeName == "int" || typeName == "int32") {
        if (args.size() < 2) return Value::makeNumber(0);
        return Value::makeNumber(args[1].as<double>());
    }
    if (typeName == "str") {
        if (args.size() < 2) return Value::makeString("");
        return Value::makeString(args[1].as<std::string>());
    }
    // More types can be added here
    return Value::makeNil();
}

Value builtin_print(Environment& env, const std::vector<Value>& args) {
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i].type == Value::Type::STRING) std::cout << args[i].as<std::string>();
        else if (args[i].type == Value::Type::NUMBER) std::cout << args[i].as<double>();
        else if (args[i].type == Value::Type::ATOM) std::cout << args[i].as<std::string>();
        if (i < args.size() - 1) std::cout << " ";
    }
    std::cout << std::endl;
    return Value::makeNil();
}

// Global Environment Initialization
void Evaluator::initGlobalEnv(Environment& env) {
    env.define("def", Value::makeFunc(builtin_def));
    env.define("get", Value::makeFunc(builtin_get));
    env.define("exec", Value::makeFunc(builtin_exec));
    env.define("make", Value::makeFunc(builtin_make));
    env.define("print", Value::makeFunc(builtin_print));
    
    env.define("str", Value::makeFunc([](Environment&, const std::vector<Value>& args) {
        if (args.empty()) return Value::makeString("");
        if (args[0].type == Value::Type::STRING) return args[0];
        if (args[0].type == Value::Type::ATOM) return Value::makeString(args[0].as<std::string>());
        return Value::makeString("");
    }));
    
    // Add math ops
    env.define("+", Value::makeFunc([](Environment&, const std::vector<Value>& args) {
        double res = 0;
        for (const auto& a : args) {
            if (a.type == Value::Type::NUMBER) res += a.as<double>();
            else if (a.type == Value::Type::ATOM) {
                try { res += std::stod(a.as<std::string>()); } catch(...) {}
            }
        }
        return Value::makeNumber(res);
    }));
    env.define("-", Value::makeFunc([](Environment&, const std::vector<Value>& args) {
        if (args.empty()) return Value::makeNumber(0);
        double res = 0;
        if (args[0].type == Value::Type::NUMBER) res = args[0].as<double>();
        else if (args[0].type == Value::Type::ATOM) {
            try { res = std::stod(args[0].as<std::string>()); } catch(...) {}
        }
        for (size_t i = 1; i < args.size(); ++i) {
             if (args[i].type == Value::Type::NUMBER) res -= args[i].as<double>();
             else if (args[i].type == Value::Type::ATOM) {
                 try { res -= std::stod(args[i].as<std::string>()); } catch(...) {}
             }
        }
        return Value::makeNumber(res);
    }));
}
