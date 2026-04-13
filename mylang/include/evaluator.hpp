#ifndef EVALUATOR_HPP
#define EVALUATOR_HPP

#include "value.hpp"
#include "environment.hpp"
#include <list>

class Evaluator {
public:
    static Value evaluate(const Value& val, Environment& env);
    static Value evalList(const std::list<Value>& list, Environment& env);
};

#endif
