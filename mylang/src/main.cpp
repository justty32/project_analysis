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
#include "evaluator.hpp"
#include "environment.hpp"

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void printAST(const Value& v, int indent = 0) {
    std::string space(indent * 2, ' ');
    if (v.type == Value::Type::LIST) {
        std::cout << space << "(\n";
        auto list = v.as<std::list<Value>>();
        for (const auto& item : list) {
            printAST(item, indent + 1);
        }
        std::cout << space << ")\n";
    } else if (v.type == Value::Type::ATOM) {
        std::cout << space << v.as<std::string>() << "\n";
    } else if (v.type == Value::Type::STRING) {
        std::cout << space << "\"" << v.as<std::string>() << "\"\n";
    } else if (v.type == Value::Type::NUMBER) {
        std::cout << space << v.as<double>() << "\n";
    }
}

int main(int argc, char* argv[]) {
    std::string filePath = "tests/meta_test.mylang";
    if (argc > 1) {
        filePath = argv[1];
    }

    std::cout << "=== Testing: " << filePath << " ===\n";

    // Clear output.cpp for a fresh run
    std::ofstream ofs("output.cpp", std::ios::trunc);
    ofs.close();

    try {
        std::string source = readFile(filePath);
        Lexer lexer(source);
        auto tokenLines = lexer.tokenize();
        Parser parser(tokenLines);
        auto values = parser.parse();

        std::cout << "--- AST Structure ---\n";
        for (const auto& v : values) {
            printAST(v);
        }

        Environment globalEnv;
        Evaluator::initGlobalEnv(globalEnv);

        std::cout << "\n--- Evaluation Output ---\n";
        for (const auto& v : values) {
            Evaluator::evaluate(v, globalEnv);
        }

        std::cout << "\n=== Generated output.cpp ===\n";
        std::cout << readFile("output.cpp") << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
