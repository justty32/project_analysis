#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include "value.hpp"
#include <map>
#include <string>
#include <stdexcept>

class Environment : public std::enable_shared_from_this<Environment> {
public:
    Environment(std::shared_ptr<Environment> parent = nullptr) : parent_(parent) {}

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
            define(name, val);
        }
    }

private:
    std::map<std::string, Value> variables_;
    std::shared_ptr<Environment> parent_;
};

#endif
