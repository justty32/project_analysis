#ifndef EVALUATOR_HPP
#define EVALUATOR_HPP

#include "value.hpp"
#include "environment.hpp"
#include <list>

class Evaluator {
public:
    static Value evaluate(const Value& val, std::shared_ptr<Environment>& env);
    static Value evalList(const std::list<Value>& list, std::shared_ptr<Environment>& env);
    static void initGlobalEnv(std::shared_ptr<Environment>& env);
};

void print_value(const Value& v);

#endif
