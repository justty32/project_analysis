#ifndef PARSER_HPP
#define PARSER_HPP

#include "lexer.hpp"
#include "value.hpp"
#include <vector>

class Parser {
public:
    Parser(const std::vector<std::vector<Token>>& tokens) : lines_(tokens), linePos_(0), pos_(0) {
        static Token eofToken = {TokenType::ATOM, "EOF", -1, false};
        eofToken_ = eofToken;
    }

    std::vector<Value> parse();

private:
    std::vector<std::vector<Token>> lines_;
    size_t linePos_;
    size_t pos_;
    Token eofToken_;

    Value parseLine();
    Value parseExpression();
    Value parsePrimary();
    Value parseList();
    Value parseArray();
    Value parseDictOrSet();

    const Token& peek() const;
    const Token& advance();
    bool isAtEnd() const;
    bool isAtEndOfLine() const;
    bool match(TokenType type);
};

#endif
