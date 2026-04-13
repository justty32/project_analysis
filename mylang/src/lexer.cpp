#include "lexer.hpp"
#include <cctype>

int Lexer::countLeadingTabs() {
    int count = 0;
    while (!isAtEnd() && peek() == '\t') {
        count++;
        advance();
    }
    return count;
}

std::vector<std::vector<Token>> Lexer::tokenize() {
    std::vector<std::vector<Token>> logicalLines;
    std::vector<Token> currentLine;
    int bracketDepth = 0;
    bool hasSpace = false;
    bool atLineStart = true;

    auto pushToken = [&](Token t) {
        if (t.type == TokenType::LPAREN || t.type == TokenType::LBRACKET || t.type == TokenType::LBRACE) {
            bracketDepth++;
        } else if (t.type == TokenType::RPAREN || t.type == TokenType::RBRACKET || t.type == TokenType::RBRACE) {
            bracketDepth--;
        }
        currentLine.push_back(t);
        hasSpace = false;
        atLineStart = false;
    };

    while (!isAtEnd()) {
        if (atLineStart && bracketDepth == 0) {
            indent_ = countLeadingTabs();
            atLineStart = false;
            if (isAtEnd()) break;
        }

        char c = peek();

        if (std::isspace(c) && c != '\n') {
            hasSpace = true;
            advance();
            continue;
        }

        if (c == '\n') {
            advance();
            if (bracketDepth == 0 && !currentLine.empty()) {
                logicalLines.push_back(currentLine);
                currentLine.clear();
            }
            line_++;
            hasSpace = true;
            atLineStart = true;
            continue;
        }

        if (c == '/' && (peekNext() == '/' || peekNext() == '*')) {
            hasSpace = true;
            if (peekNext() == '/') {
                advance(); advance();
                while (!isAtEnd() && peek() != '\n') advance();
            } else {
                advance(); advance();
                while (!isAtEnd()) {
                    if (peek() == '*' && peekNext() == '/') {
                        advance(); advance();
                        break;
                    }
                    if (peek() == '\n') line_++;
                    advance();
                }
            }
            continue;
        }

        if (c == '"') {
            pushToken(readString(hasSpace));
            continue;
        }

        if (c == '\'') {
            pushToken({TokenType::QUOTE, std::string(1, advance()), line_, indent_, hasSpace});
            continue;
        }

        if (c == '(') { pushToken({TokenType::LPAREN, std::string(1, advance()), line_, indent_, hasSpace}); continue; }
        if (c == ')') { pushToken({TokenType::RPAREN, std::string(1, advance()), line_, indent_, hasSpace}); continue; }
        if (c == '[') { pushToken({TokenType::LBRACKET, std::string(1, advance()), line_, indent_, hasSpace}); continue; }
        if (c == ']') { pushToken({TokenType::RBRACKET, std::string(1, advance()), line_, indent_, hasSpace}); continue; }
        if (c == '{') { pushToken({TokenType::LBRACE, std::string(1, advance()), line_, indent_, hasSpace}); continue; }
        if (c == '}') { pushToken({TokenType::RBRACE, std::string(1, advance()), line_, indent_, hasSpace}); continue; }
        if (c == ':') { pushToken({TokenType::COLON, std::string(1, advance()), line_, indent_, hasSpace}); continue; }
        if (c == ',') { pushToken({TokenType::COMMA, std::string(1, advance()), line_, indent_, hasSpace}); continue; }

        if (c == '\\') {
            if (peekNext() == '\n') {
                advance(); advance(); line_++; hasSpace = true; continue; 
            }
            if (peekNext() == '\r') {
                advance(); advance();
                if (peek() == '\n') advance();
                line_++; hasSpace = true; continue;
            }
            pushToken(readAtom(hasSpace));
            continue;
        }

        if (!isAtEnd() && !std::isspace(c)) {
            pushToken(readAtom(hasSpace));
        } else if (!isAtEnd()) {
            advance();
        }
    }

    if (!currentLine.empty()) {
        logicalLines.push_back(currentLine);
    }

    return logicalLines;
}

Token Lexer::readString(bool spaceBefore) {
    advance(); // Skip opening quote
    std::string value;
    while (!isAtEnd()) {
        char c = peek();
        if (c == '"') {
            advance();
            break;
        }
        if (c == '\\') {
            advance();
            if (isAtEnd()) break;
            char next = peek();
            if (next == '\n') {
                // Line continuation: skip backslash and newline
                advance();
                line_++;
                continue;
            }
            if (next == '\r') {
                advance();
                if (!isAtEnd() && peek() == '\n') advance();
                line_++;
                continue;
            }
            // Normal escape
            char esc = advance();
            if (esc == 'n') value += '\n';
            else if (esc == 't') value += '\t';
            else if (esc == 'r') value += '\r';
            else value += esc;
        } else {
            if (c == '\n') line_++;
            value += advance();
        }
    }
    return {TokenType::STRING, value, line_, indent_, spaceBefore};
}

Token Lexer::readAtom(bool spaceBefore) {
    std::string value;
    while (!isAtEnd()) {
        char c = peek();
        if (c == '\\') {
            advance();
            if (isAtEnd()) break;
            if (peek() == '\n') {
                advance(); line_++; 
                value += ' ';
                continue; 
            }
            if (peek() == '\r') {
                advance();
                if (peek() == '\n') advance();
                line_++; value += ' '; continue;
            }
            value += advance();
            continue;
        }
        if (std::isspace(c) || c == '\'' || c == '"' || c == '(' || c == ')' || 
            c == '[' || c == ']' || c == '{' || c == '}' || c == ':' || c == ',') {
            break;
        }
        if (c == '/' && (peekNext() == '/' || peekNext() == '*')) {
            break;
        }
        value += advance();
    }
    return {TokenType::ATOM, value, line_, indent_, spaceBefore};
}
