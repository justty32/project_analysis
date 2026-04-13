#include "value.hpp"

bool operator<(const Value& lhs, const Value& rhs) {
    if (lhs.type != rhs.type) return lhs.type < rhs.type;
    switch (lhs.type) {
        case Value::Type::NIL: return false;
        case Value::Type::NUMBER: return lhs.as<double>() < rhs.as<double>();
        case Value::Type::STRING: return lhs.as<std::string>() < rhs.as<std::string>();
        case Value::Type::ATOM: return lhs.as<std::string>() < rhs.as<std::string>();
        case Value::Type::LIST: {
            auto l = lhs.as<std::list<Value>>();
            auto r = rhs.as<std::list<Value>>();
            return std::lexicographical_compare(l.begin(), l.end(), r.begin(), r.end());
        }
        case Value::Type::ARRAY: {
            auto l = lhs.as<std::vector<Value>>();
            auto r = rhs.as<std::vector<Value>>();
            return std::lexicographical_compare(l.begin(), l.end(), r.begin(), r.end());
        }
        case Value::Type::DICT: {
            auto l = lhs.as<std::map<Value, Value>>();
            auto r = rhs.as<std::map<Value, Value>>();
            return std::lexicographical_compare(l.begin(), l.end(), r.begin(), r.end());
        }
        case Value::Type::SET: {
            auto l = lhs.as<std::set<Value>>();
            auto r = rhs.as<std::set<Value>>();
            return std::lexicographical_compare(l.begin(), l.end(), r.begin(), r.end());
        }
        default: return false; // Functions/Macros aren't easily comparable
    }
}

bool operator==(const Value& lhs, const Value& rhs) {
    if (lhs.type != rhs.type) return false;
    switch (lhs.type) {
        case Value::Type::NIL: return true;
        case Value::Type::NUMBER: return lhs.as<double>() == rhs.as<double>();
        case Value::Type::STRING: return lhs.as<std::string>() == rhs.as<std::string>();
        case Value::Type::ATOM: return lhs.as<std::string>() == rhs.as<std::string>();
        case Value::Type::LIST: return lhs.as<std::list<Value>>() == rhs.as<std::list<Value>>();
        case Value::Type::ARRAY: return lhs.as<std::vector<Value>>() == rhs.as<std::vector<Value>>();
        case Value::Type::DICT: return lhs.as<std::map<Value, Value>>() == rhs.as<std::map<Value, Value>>();
        case Value::Type::SET: return lhs.as<std::set<Value>>() == rhs.as<std::set<Value>>();
        default: return false;
    }
}
