#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include "value.hpp"
#include <map>
#include <string>

class Environment {
public:
    Environment(Environment* parent = nullptr) : parent_(parent) {}

    void define(const std::string& name, Value val) {
        variables_[name] = val;
    }

    Value get(const std::string& name) {
        if (variables_.count(name)) return variables_[name];
        if (parent_) return parent_->get(name);
        throw std::runtime_error("Undefined atom: " + name);
    }

private:
    std::map<std::string, Value> variables_;
    Environment* parent_;
};

#endif
