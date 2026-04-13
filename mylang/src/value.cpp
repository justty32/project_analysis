#include "value.hpp"

bool operator<(const Value& lhs, const Value& rhs) {
    if (lhs.type != rhs.type) {
        // Allow cross-type comparison for Atom and String as keys
        if ((lhs.type == Value::Type::ATOM || lhs.type == Value::Type::STRING) &&
            (rhs.type == Value::Type::ATOM || rhs.type == Value::Type::STRING)) {
            return lhs.as<std::string>() < rhs.as<std::string>();
        }
        return lhs.type < rhs.type;
    }
    switch(lhs.type) {
        case Value::Type::STRING: 
        case Value::Type::ATOM: 
            return lhs.as<std::string>() < rhs.as<std::string>();
        case Value::Type::NUMBER: 
            return lhs.as<double>() < rhs.as<double>();
        default: 
            return false;
    }
}
