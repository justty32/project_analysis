#include "evaluator.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

double evaluate_to_double(const Value& v) {
    if (v.type == Value::Type::NUMBER) return v.as<double>();
    if (v.type == Value::Type::ATOM || v.type == Value::Type::STRING) {
        try { return std::stod(v.as<std::string>()); } catch (...) {}
    }
    return 0;
}

void print_value(const Value& v) {
    if (v.type == Value::Type::STRING) std::cout << v.as<std::string>();
    else if (v.type == Value::Type::NUMBER) {
        double d = v.as<double>();
        if (d == (long long)d) std::cout << (long long)d; else std::cout << d;
    }
    else if (v.type == Value::Type::ATOM) std::cout << v.as<std::string>();
    else if (v.type == Value::Type::LIST) {
        std::cout << "(";
        auto l = v.as<std::list<Value>>();
        for (auto it = l.begin(); it != l.end(); ++it) {
            print_value(*it);
            if (std::next(it) != l.end()) std::cout << " ";
        }
        std::cout << ")";
    } else if (v.type == Value::Type::ARRAY) {
        std::cout << "[";
        auto a = v.as<std::vector<Value>>();
        for (size_t i = 0; i < a.size(); ++i) {
            print_value(a[i]);
            if (i < a.size() - 1) std::cout << ", ";
        }
        std::cout << "]";
    } else if (v.type == Value::Type::DICT) {
        std::cout << "{";
        auto m = v.as<std::map<Value, Value>>();
        for (auto it = m.begin(); it != m.end(); ++it) {
            print_value(it->first); std::cout << ": "; print_value(it->second);
            if (std::next(it) != m.end()) std::cout << ", ";
        }
        std::cout << "}";
    } else if (v.type == Value::Type::SET) {
        std::cout << "{";
        auto s = v.as<std::set<Value>>();
        for (auto it = s.begin(); it != s.end(); ++it) {
            print_value(*it);
            if (std::next(it) != s.end()) std::cout << ", ";
        }
        std::cout << "}";
    } else if (v.type == Value::Type::NIL) std::cout << "nil";
}

Value Evaluator::evaluate(const Value& val, std::shared_ptr<Environment>& env) {
    if (val.type == Value::Type::ATOM) {
        std::string name = val.as<std::string>();
        if (name == "nil") return Value::makeNil();
        if (name == "true") return Value::makeAtom("true");
        if (name == "false") return Value::makeAtom("false");
        try {
            size_t pos;
            double d = std::stod(name, &pos);
            if (pos == name.length()) return Value::makeNumber(d);
        } catch (...) {}
        return env->get(name);
    }
    if (val.type == Value::Type::LIST) {
        auto list = val.as<std::list<Value>>();
        if (list.empty()) return val;
        Value first = list.front();
        if (first.type == Value::Type::ATOM && first.as<std::string>() == "quote") {
            if (list.size() < 2) return Value::makeNil();
            return *std::next(list.begin());
        }
        return evalList(list, env);
    }
    return val;
}

Value Evaluator::evalList(const std::list<Value>& list, std::shared_ptr<Environment>& env) {
    if (list.empty()) return Value::makeList({});
    Value head = evaluate(list.front(), env);

    if (head.type == Value::Type::FUNC) {
        std::vector<Value> args;
        auto it = std::next(list.begin());
        while (it != list.end()) {
            args.push_back(evaluate(*it, env));
            it++;
        }
        return head.as<NativeFunc>()(env, args);
    }

    if (head.type == Value::Type::MACRO) {
        std::vector<Value> args;
        auto it = std::next(list.begin());
        while (it != list.end()) { args.push_back(*it); it++; }
        Value expanded = head.as<NativeFunc>()(env, args);
        if (expanded.type == Value::Type::LIST) return evaluate(expanded, env);
        return expanded;
    }

    if (head.type == Value::Type::USER_FUNC) {
        auto proc = head.as<std::shared_ptr<UserProcedure>>();
        auto localEnv = std::make_shared<Environment>(proc->closure);
        auto it = std::next(list.begin());
        for (const auto& param : proc->params) {
            if (it != list.end()) { localEnv->define(param, evaluate(*it, env)); it++; }
            else { localEnv->define(param, Value::makeNil()); }
        }
        return evaluate(proc->body, localEnv);
    }

    if (head.type == Value::Type::USER_MACRO) {
        auto proc = head.as<std::shared_ptr<UserProcedure>>();
        auto localEnv = std::make_shared<Environment>(proc->closure);
        auto it = std::next(list.begin());
        for (const auto& param : proc->params) {
            if (it != list.end()) { localEnv->define(param, *it); it++; }
            else { localEnv->define(param, Value::makeNil()); }
        }
        Value expanded = evaluate(proc->body, localEnv);
        if (expanded.type == Value::Type::LIST) return evaluate(expanded, env);
        return expanded;
    }

    std::list<Value> res;
    for (const auto& item : list) res.push_back(evaluate(item, env));
    return Value::makeList(res);
}

Value builtin_def(std::shared_ptr<Environment>& env, const std::vector<Value>& args) {
    if (args.size() < 2) throw std::runtime_error("def requires name and value");
    if (args[0].type != Value::Type::ATOM) throw std::runtime_error("def name must be atom");
    Value val = Evaluator::evaluate(args[1], env);
    env->define(args[0].as<std::string>(), val);
    return val;
}

Value builtin_lambda(std::shared_ptr<Environment>& env, const std::vector<Value>& args) {
    if (args.size() < 2) throw std::runtime_error("lambda requires params and body");
    auto proc = std::make_shared<UserProcedure>();
    if (args[0].type == Value::Type::LIST) {
        for (const auto& v : args[0].as<std::list<Value>>()) 
            if (v.type == Value::Type::ATOM) proc->params.push_back(v.as<std::string>());
    } else if (args[0].type == Value::Type::ATOM) {
        proc->params.push_back(args[0].as<std::string>());
    }
    if (args.size() > 2) {
        std::list<Value> body_list = {Value::makeAtom("begin")};
        for (size_t i = 1; i < args.size(); ++i) body_list.push_back(args[i]);
        proc->body = Value::makeList(body_list);
    } else { proc->body = args[1]; }
    proc->closure = env;
    return Value::makeUserFunc(proc);
}

Value builtin_def_macro(std::shared_ptr<Environment>& env, const std::vector<Value>& args) {
    if (args.size() < 3) throw std::runtime_error("def_macro requires name, params, body");
    if (args[0].type != Value::Type::ATOM) throw std::runtime_error("def_macro name must be atom");
    auto proc = std::make_shared<UserProcedure>();
    if (args[1].type == Value::Type::LIST) {
        for (const auto& v : args[1].as<std::list<Value>>()) 
            if (v.type == Value::Type::ATOM) proc->params.push_back(v.as<std::string>());
    } else if (args[1].type == Value::Type::ATOM) {
        proc->params.push_back(args[1].as<std::string>());
    }
    if (args.size() > 3) {
        std::list<Value> body_list = {Value::makeAtom("begin")};
        for (size_t i = 2; i < args.size(); ++i) body_list.push_back(args[i]);
        proc->body = Value::makeList(body_list);
    } else { proc->body = args[2]; }
    proc->closure = env;
    Value macro = Value::makeUserMacro(proc);
    env->define(args[0].as<std::string>(), macro);
    return macro;
}

Value builtin_if(std::shared_ptr<Environment>& env, const std::vector<Value>& args) {
    if (args.size() < 2) throw std::runtime_error("if requires cond and then");
    Value cond = Evaluator::evaluate(args[0], env);
    bool isTrue = true;
    if (cond.type == Value::Type::NIL) isTrue = false;
    else if (cond.type == Value::Type::NUMBER && cond.as<double>() == 0) isTrue = false;
    else if (cond.type == Value::Type::ATOM && (cond.as<std::string>() == "false" || cond.as<std::string>() == "nil")) isTrue = false;
    if (isTrue) return Evaluator::evaluate(args[1], env);
    return (args.size() >= 3) ? Evaluator::evaluate(args[2], env) : Value::makeNil();
}

void Evaluator::initGlobalEnv(std::shared_ptr<Environment>& env) {
    env->define("begin", Value::makeMacro([](std::shared_ptr<Environment>& env, const std::vector<Value>& args) {
        Value last = Value::makeNil();
        for (const auto& a : args) last = Evaluator::evaluate(a, env);
        return last;
    }));
    env->define("def", Value::makeMacro(builtin_def));
    env->define("lambda", Value::makeMacro(builtin_lambda));
    env->define("def_macro", Value::makeMacro(builtin_def_macro));
    env->define("if", Value::makeMacro(builtin_if));
    env->define("list", Value::makeMacro([](std::shared_ptr<Environment>& env, const std::vector<Value>& args) {
        std::list<Value> res; for (const auto& a : args) res.push_back(Evaluator::evaluate(a, env)); return Value::makeList(res);
    }));
    env->define("make", Value::makeFunc([](std::shared_ptr<Environment>& env, const std::vector<Value>& args) {
        if (args.empty() || args[0].type != Value::Type::ATOM) return Value::makeNil();
        std::string type = args[0].as<std::string>();
        if (type == "LIST" || type == "list") {
            std::list<Value> res; for (size_t i = 1; i < args.size(); ++i) res.push_back(args[i]); return Value::makeList(res);
        }
        if (type == "ARRAY" || type == "arr") {
            std::vector<Value> res; for (size_t i = 1; i < args.size(); ++i) res.push_back(args[i]); return Value::makeArray(res);
        }
        return Value::makeNil();
    }));
    env->define("exec", Value::makeMacro([](std::shared_ptr<Environment>& env, const std::vector<Value>& args) {
        if (args.empty()) return Value::makeNil();
        Value val = Evaluator::evaluate(args[0], env); return Evaluator::evaluate(val, env);
    }));
    env->define("arr", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) { return Value::makeArray(args); }));
    env->define("dict", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) {
        std::map<Value, Value> m;
        for (const auto& a : args) {
            if (a.type == Value::Type::LIST) {
                auto l = a.as<std::list<Value>>();
                if (l.size() >= 2) { m[l.front()] = *std::next(l.begin()); }
            }
        }
        return Value::makeDict(m);
    }));
    env->define("set", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) {
        std::set<Value> s; for (const auto& a : args) s.insert(a); return Value::makeSet(s);
    }));
    env->define("get", Value::makeFunc([](std::shared_ptr<Environment>& env, const std::vector<Value>& args) {
        if (args.empty() || args[0].type != Value::Type::ATOM) return Value::makeNil(); return env->get(args[0].as<std::string>());
    }));
    env->define("print", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) {
        for (size_t i = 0; i < args.size(); ++i) { print_value(args[i]); if (i < args.size() - 1) std::cout << " "; }
        std::cout << std::endl; return Value::makeNil();
    }));
    env->define("str", Value::makeMacro([](std::shared_ptr<Environment>& env, const std::vector<Value>& args) {
        std::stringstream ss;
        for (const auto& a : args) {
            Value v; try { v = Evaluator::evaluate(a, env); } catch (...) { v = a; }
            if (v.type == Value::Type::STRING || v.type == Value::Type::ATOM) ss << v.as<std::string>();
            else if (v.type == Value::Type::NUMBER) {
                double d = v.as<double>(); if (d == (long long)d) ss << (long long)d; else ss << d;
            } else if (v.type == Value::Type::NIL) ss << "nil";
        }
        return Value::makeString(ss.str());
    }));
    env->define("set!", Value::makeMacro([](std::shared_ptr<Environment>& env, const std::vector<Value>& args) {
        if (args.size() < 2 || args[0].type != Value::Type::ATOM) throw std::runtime_error("set! requires atom name");
        Value val = Evaluator::evaluate(args[1], env); env->set(args[0].as<std::string>(), val); return val;
    }));
    env->define("while", Value::makeMacro([](std::shared_ptr<Environment>& env, const std::vector<Value>& args) {
        if (args.size() < 2) throw std::runtime_error("while requires condition and body");
        Value last = Value::makeNil();
        while (true) {
            Value cond = Evaluator::evaluate(args[0], env);
            bool isTrue = true;
            if (cond.type == Value::Type::NIL) isTrue = false;
            else if (cond.type == Value::Type::NUMBER && cond.as<double>() == 0) isTrue = false;
            else if (cond.type == Value::Type::ATOM && (cond.as<std::string>() == "false" || cond.as<std::string>() == "nil")) isTrue = false;
            if (!isTrue) break;
            for (size_t i = 1; i < args.size(); ++i) last = Evaluator::evaluate(args[i], env);
        }
        return last;
    }));
    env->define("len", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) {
        if (args.empty()) return Value::makeNumber(0); const auto& a = args[0];
        if (a.type == Value::Type::LIST) return Value::makeNumber(a.as<std::list<Value>>().size());
        if (a.type == Value::Type::ARRAY) return Value::makeNumber(a.as<std::vector<Value>>().size());
        if (a.type == Value::Type::DICT) return Value::makeNumber(a.as<std::map<Value, Value>>().size());
        if (a.type == Value::Type::SET) return Value::makeNumber(a.as<std::set<Value>>().size());
        if (a.type == Value::Type::STRING || a.type == Value::Type::ATOM) return Value::makeNumber(a.as<std::string>().size());
        return Value::makeNumber(0);
    }));
    env->define("at", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) {
        if (args.size() < 2 || args[1].type != Value::Type::NUMBER) return Value::makeNil();
        int idx = (int)args[1].as<double>(); const auto& a = args[0];
        if (a.type == Value::Type::LIST) {
            auto l = a.as<std::list<Value>>(); if (idx < 0 || idx >= (int)l.size()) return Value::makeNil();
            auto it = l.begin(); std::advance(it, idx); return *it;
        }
        if (a.type == Value::Type::ARRAY) {
            auto v = a.as<std::vector<Value>>(); if (idx < 0 || idx >= (int)v.size()) return Value::makeNil(); return v[idx];
        }
        if (a.type == Value::Type::STRING || a.type == Value::Type::ATOM) {
            std::string s = a.as<std::string>(); if (idx < 0 || idx >= (int)s.size()) return Value::makeNil(); return Value::makeString(std::string(1, s[idx]));
        }
        return Value::makeNil();
    }));
    env->define("push", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) {
        if (args.size() < 2 || args[0].type != Value::Type::ARRAY) return Value::makeNil();
        auto v = args[0].as<std::vector<Value>>(); v.push_back(args[1]); return Value::makeArray(v);
    }));
    env->define("put", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) {
        if (args.size() < 3 || args[0].type != Value::Type::DICT) return Value::makeNil();
        auto m = args[0].as<std::map<Value, Value>>(); m[args[1]] = args[2]; return Value::makeDict(m);
    }));
    env->define("has?", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) {
        if (args.size() < 2) return Value::makeAtom("false"); const auto& c = args[0];
        if (c.type == Value::Type::DICT) return c.as<std::map<Value, Value>>().count(args[1]) ? Value::makeAtom("true") : Value::makeAtom("false");
        if (c.type == Value::Type::SET) return c.as<std::set<Value>>().count(args[1]) ? Value::makeAtom("true") : Value::makeAtom("false");
        return Value::makeAtom("false");
    }));
    env->define("nil?", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) { return (args.size() > 0 && args[0].type == Value::Type::NIL) ? Value::makeAtom("true") : Value::makeAtom("false"); }));
    env->define("atom?", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) { return (args.size() > 0 && args[0].type == Value::Type::ATOM) ? Value::makeAtom("true") : Value::makeAtom("false"); }));
    env->define("list?", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) { return (args.size() > 0 && args[0].type == Value::Type::LIST) ? Value::makeAtom("true") : Value::makeAtom("false"); }));
    env->define("num?", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) { return (args.size() > 0 && args[0].type == Value::Type::NUMBER) ? Value::makeAtom("true") : Value::makeAtom("false"); }));
    env->define("str?", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) { return (args.size() > 0 && args[0].type == Value::Type::STRING) ? Value::makeAtom("true") : Value::makeAtom("false"); }));
    env->define("io_read", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) {
        if (args.empty() || (args[0].type != Value::Type::STRING && args[0].type != Value::Type::ATOM)) return Value::makeNil();
        std::ifstream f(args[0].as<std::string>()); if (!f.is_open()) return Value::makeNil();
        std::stringstream ss; ss << f.rdbuf(); return Value::makeString(ss.str());
    }));
    env->define("io_write", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) {
        if (args.size() < 2 || (args[0].type != Value::Type::STRING && args[0].type != Value::Type::ATOM)) return Value::makeNil();
        std::ofstream f(args[0].as<std::string>()); if (!f.is_open()) return Value::makeNil();
        Value v = args[1]; if (v.type == Value::Type::STRING || v.type == Value::Type::ATOM) f << v.as<std::string>();
        else if (v.type == Value::Type::NUMBER) { double d = v.as<double>(); if (d == (long long)d) f << (long long)d; else f << d; }
        else f << "<obj>"; return Value::makeAtom("true");
    }));
    env->define("io_exists?", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) {
        if (args.empty() || (args[0].type != Value::Type::STRING && args[0].type != Value::Type::ATOM)) return Value::makeAtom("false");
        return fs::exists(args[0].as<std::string>()) ? Value::makeAtom("true") : Value::makeAtom("false");
    }));
    env->define("+", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) { double res = 0; for (const auto& a : args) res += evaluate_to_double(a); return Value::makeNumber(res); }));
    env->define("-", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) {
        if (args.empty()) return Value::makeNumber(0);
        double res = evaluate_to_double(args[0]); for (size_t i = 1; i < args.size(); ++i) res -= evaluate_to_double(args[i]); return Value::makeNumber(res);
    }));
    env->define("*", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) { double res = 1; for (const auto& a : args) res *= evaluate_to_double(a); return Value::makeNumber(res); }));
    env->define("/", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) {
        if (args.empty()) return Value::makeNumber(0);
        double res = evaluate_to_double(args[0]); for (size_t i = 1; i < args.size(); ++i) {
            double v = evaluate_to_double(args[i]); if (v == 0) throw std::runtime_error("Division by zero"); res /= v;
        } return Value::makeNumber(res);
    }));
    env->define("mod", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) {
        if (args.size() < 2) return Value::makeNumber(0); return Value::makeNumber((int)evaluate_to_double(args[0]) % (int)evaluate_to_double(args[1]));
    }));
    auto eq_func = [](std::shared_ptr<Environment>&, const std::vector<Value>& args) { if (args.size() < 2) return Value::makeAtom("true"); return (args[0] == args[1]) ? Value::makeAtom("true") : Value::makeAtom("false"); };
    env->define("eq", Value::makeFunc(eq_func)); env->define("==", Value::makeFunc(eq_func));
    auto ne_func = [](std::shared_ptr<Environment>&, const std::vector<Value>& args) { if (args.size() < 2) return Value::makeAtom("false"); return (!(args[0] == args[1])) ? Value::makeAtom("true") : Value::makeAtom("false"); };
    env->define("ne", Value::makeFunc(ne_func)); env->define("!=", Value::makeFunc(ne_func));
    auto gt_func = [](std::shared_ptr<Environment>&, const std::vector<Value>& args) { if (args.size() < 2) return Value::makeAtom("false"); return (evaluate_to_double(args[0]) > evaluate_to_double(args[1])) ? Value::makeAtom("true") : Value::makeAtom("false"); };
    env->define("gt", Value::makeFunc(gt_func)); env->define(">", Value::makeFunc(gt_func));
    auto lt_func = [](std::shared_ptr<Environment>&, const std::vector<Value>& args) { if (args.size() < 2) return Value::makeAtom("false"); return (evaluate_to_double(args[0]) < evaluate_to_double(args[1])) ? Value::makeAtom("true") : Value::makeAtom("false"); };
    env->define("lt", Value::makeFunc(lt_func)); env->define("<", Value::makeFunc(lt_func));
    auto ge_func = [](std::shared_ptr<Environment>&, const std::vector<Value>& args) { if (args.size() < 2) return Value::makeAtom("false"); return (evaluate_to_double(args[0]) >= evaluate_to_double(args[1])) ? Value::makeAtom("true") : Value::makeAtom("false"); };
    env->define("ge", Value::makeFunc(ge_func)); env->define(">=", Value::makeFunc(ge_func));
    auto le_func = [](std::shared_ptr<Environment>&, const std::vector<Value>& args) { if (args.size() < 2) return Value::makeAtom("false"); return (evaluate_to_double(args[0]) <= evaluate_to_double(args[1])) ? Value::makeAtom("true") : Value::makeAtom("false"); };
    env->define("le", Value::makeFunc(le_func)); env->define("<=", Value::makeFunc(le_func));
    env->define("not", Value::makeFunc([](std::shared_ptr<Environment>&, const std::vector<Value>& args) {
        if (args.empty()) return Value::makeAtom("true"); bool isTrue = true;
        if (args[0].type == Value::Type::NIL) isTrue = false;
        else if (args[0].type == Value::Type::NUMBER && args[0].as<double>() == 0) isTrue = false;
        else if (args[0].type == Value::Type::ATOM && (args[0].as<std::string>() == "false" || args[0].as<std::string>() == "nil")) isTrue = false;
        return isTrue ? Value::makeAtom("false") : Value::makeAtom("true");
    }));
    env->define("and", Value::makeMacro([](std::shared_ptr<Environment>& env, const std::vector<Value>& args) {
        Value last = Value::makeAtom("true");
        for (const auto& a : args) {
            last = Evaluator::evaluate(a, env); bool isTrue = true;
            if (last.type == Value::Type::NIL) isTrue = false;
            else if (last.type == Value::Type::NUMBER && last.as<double>() == 0) isTrue = false;
            else if (last.type == Value::Type::ATOM && (last.as<std::string>() == "false" || last.as<std::string>() == "nil")) isTrue = false;
            if (!isTrue) return Value::makeAtom("false");
        } return last;
    }));
    env->define("or", Value::makeMacro([](std::shared_ptr<Environment>& env, const std::vector<Value>& args) {
        for (const auto& a : args) {
            Value val = Evaluator::evaluate(a, env); bool isTrue = true;
            if (val.type == Value::Type::NIL) isTrue = false;
            else if (val.type == Value::Type::NUMBER && val.as<double>() == 0) isTrue = false;
            else if (val.type == Value::Type::ATOM && (val.as<std::string>() == "false" || val.as<std::string>() == "nil")) isTrue = false;
            if (isTrue) return val;
        } return Value::makeAtom("false");
    }));
    env->define("def_func", Value::makeMacro([](std::shared_ptr<Environment>&, const std::vector<Value>& args) {
        if (args.size() < 3) throw std::runtime_error("def_func requires name, params, body");
        std::list<Value> lambda_args = {Value::makeAtom("lambda"), args[1]};
        for (size_t i = 2; i < args.size(); ++i) lambda_args.push_back(args[i]);
        return Value::makeList({Value::makeAtom("def"), args[0], Value::makeList(lambda_args)});
    }));
}
