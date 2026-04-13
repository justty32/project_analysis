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
    if (!file.is_open()) throw std::runtime_error("Could not open file: " + path);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void runScript(const std::string& filePath, std::shared_ptr<Environment>& env) {
    try {
        std::string source = readFile(filePath);
        Lexer lexer(source);
        auto tokenLines = lexer.tokenize();
        Parser parser(tokenLines);
        auto values = parser.parse();

        for (const auto& v : values) {
            Evaluator::evaluate(v, env);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error running script: " << e.what() << std::endl;
    }
}

void repl() {
    auto globalEnv = std::make_shared<Environment>();
    Evaluator::initGlobalEnv(globalEnv);

    std::cout << "mylang REPL v0.1\n";
    std::cout << "Type 'exit' or Ctrl-C to quit.\n";

    std::string line;
    std::string multilineBuffer;
    int bracketDepth = 0;

    while (true) {
        if (multilineBuffer.empty()) std::cout << ">> ";
        else std::cout << ".. ";

        if (!std::getline(std::cin, line)) break;
        if (line == "exit") break;
        if (line.empty() && multilineBuffer.empty()) continue;

        multilineBuffer += line + "\n";

        // Simple bracket/parenthesis check to support multi-line entry
        for (char c : line) {
            if (c == '(' || c == '[' || c == '{') bracketDepth++;
            else if (c == ')' || c == ']' || c == '}') bracketDepth--;
        }

        if (bracketDepth <= 0) {
            try {
                Lexer lexer(multilineBuffer);
                auto tokenLines = lexer.tokenize();
                if (!tokenLines.empty()) {
                    Parser parser(tokenLines);
                    auto values = parser.parse();
                    for (const auto& v : values) {
                        Value res = Evaluator::evaluate(v, globalEnv);
                        if (res.type != Value::Type::NIL) {
                            std::cout << "=> ";
                            print_value(res);
                            std::cout << std::endl;
                        }
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Error: " << e.what() << std::endl;
            }
            multilineBuffer.clear();
            bracketDepth = 0;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        auto globalEnv = std::make_shared<Environment>();
        Evaluator::initGlobalEnv(globalEnv);
        runScript(argv[1], globalEnv);
    } else {
        repl();
    }
    return 0;
}
