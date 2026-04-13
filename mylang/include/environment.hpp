#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include "value.hpp"
#include <map>
#include <string>
#include <stdexcept>

class Environment {
public:
    Environment(Environment* parent = nullptr) : parent_(parent) {}

    void define(const std::string& name, Value val) {
        variables_[name] = val;
    }

    bool exists(const std::string& name) const {
        if (variables_.count(name)) return true;
        if (parent_) return parent_->exists(name);
        return false;
    }

    Value get(const std::string& name) {
        if (variables_.count(name)) return variables_[name];
        if (parent_) return parent_->get(name);
        throw std::runtime_error("Undefined atom: " + name);
    }

    void set(const std::string& name, Value val) {
        if (variables_.count(name)) {
            variables_[name] = val;
        } else if (parent_) {
            parent_->set(name, val);
        } else {
            define(name, val); // Default to local define if not found in chain
        }
    }

private:
    std::map<std::string, Value> variables_;
    Environment* parent_;
};

#endif
