#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>

enum class TokenType {
    ATOM,
    STRING,
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,
    QUOTE,
    COLON, // For dict k:v
    COMMA  // For separators
};

struct Token {
    TokenType type;
    std::string value;
    int line;
    bool hasSpaceBefore;
};

class Lexer {
public:
    Lexer(const std::string& source) : source_(source), pos_(0), line_(1), lastPos_(0) {}

    std::vector<std::vector<Token>> tokenize();

private:
    std::string source_;
    size_t pos_;
    int line_;
    size_t lastPos_;

    bool isAtEnd() const { return pos_ >= source_.length(); }
    char peek() const { return isAtEnd() ? '\0' : source_[pos_]; }
    char peekNext() const { return pos_ + 1 >= source_.length() ? '\0' : source_[pos_ + 1]; }
    char advance() { return source_[pos_++]; }

    Token readString(bool spaceBefore);
    Token readAtom(bool spaceBefore);
};

#endif
