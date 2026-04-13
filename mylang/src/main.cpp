#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include "lexer.hpp"
#include "parser.hpp"
#include "value.hpp"

void printValue(const Value& v, int indent = 0) {
    switch(v.type) {
        case Value::Type::ATOM: std::cout << v.as<std::string>(); break;
        case Value::Type::STRING: std::cout << "\"" << v.as<std::string>() << "\""; break;
        case Value::Type::NUMBER: std::cout << v.as<double>(); break;
        case Value::Type::FUNC: std::cout << "<builtin function>"; break;
        case Value::Type::LIST: {
            std::cout << "(";
            auto items = v.as<std::list<Value>>();
            size_t i = 0;
            for (auto const& item : items) {
                printValue(item, indent + 1);
                if (++i < items.size()) std::cout << " ";
            }
            std::cout << ")";
            break;
        }
        case Value::Type::ARRAY: {
            std::cout << "[";
            auto items = v.as<std::vector<Value>>();
            for (size_t i = 0; i < items.size(); ++i) {
                printValue(items[i], indent + 1);
                if (i < items.size() - 1) std::cout << " ";
            }
            std::cout << "]";
            break;
        }
        case Value::Type::DICT: {
            std::cout << "{";
            auto dict = v.as<std::map<Value, Value>>();
            size_t i = 0;
            for (auto const& [key, val] : dict) {
                printValue(key, indent + 1);
                std::cout << ": ";
                printValue(val, indent + 1);
                if (++i < dict.size()) std::cout << " ";
            }
            std::cout << "}";
            break;
        }
        case Value::Type::SET: {
            std::cout << "{";
            auto set = v.as<std::set<Value>>();
            size_t i = 0;
            for (auto const& item : set) {
                printValue(item, indent + 1);
                if (++i < set.size()) std::cout << " ";
            }
            std::cout << "}";
            break;
        }
        default: std::cout << "?"; break;
    }
}

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    std::string filePath = "tests/parser_suite.mylang";
    if (argc > 1) {
        filePath = argv[1];
    }

    std::cout << "=== Loading File: " << filePath << " ===\n";

    try {
        std::string source = readFile(filePath);
        Lexer lexer(source);
        auto tokenLines = lexer.tokenize();
        Parser parser(tokenLines);
        auto values = parser.parse();

        std::cout << "=== Parser Output (AST) ===\n";
        for (size_t i = 0; i < values.size(); ++i) {
            std::cout << "Line " << i + 1 << ": ";
            printValue(values[i]);
            std::cout << "\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
