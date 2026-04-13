#include "lexer.hpp"
#include <cctype>

std::vector<std::vector<Token>> Lexer::tokenize() {
    std::vector<std::vector<Token>> logicalLines;
    std::vector<Token> currentLine;
    int bracketDepth = 0;
    bool hasSpace = false;

    auto pushToken = [&](Token t) {
        if (t.type == TokenType::LPAREN || t.type == TokenType::LBRACKET || t.type == TokenType::LBRACE) {
            bracketDepth++;
        } else if (t.type == TokenType::RPAREN || t.type == TokenType::RBRACKET || t.type == TokenType::RBRACE) {
            bracketDepth--;
        }
        currentLine.push_back(t);
        hasSpace = false;
    };

    while (!isAtEnd()) {
        char c = peek();

        // 1. Whitespace (except newline)
        if (std::isspace(c) && c != '\n') {
            hasSpace = true;
            advance();
            continue;
        }

        // 2. Newline
        if (c == '\n') {
            advance();
            if (bracketDepth == 0 && !currentLine.empty()) {
                logicalLines.push_back(currentLine);
                currentLine.clear();
            }
            line_++;
            hasSpace = true;
            continue;
        }

        // 3. Comments
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

        // 4. Double Quote
        if (c == '"') {
            pushToken(readString(hasSpace));
            continue;
        }

        // 5. Single Quote
        if (c == '\'') {
            pushToken({TokenType::QUOTE, std::string(1, advance()), line_, hasSpace});
            continue;
        }

        // 6. Special Symbols (Removed DOT)
        if (c == '(') { pushToken({TokenType::LPAREN, std::string(1, advance()), line_, hasSpace}); continue; }
        if (c == ')') { pushToken({TokenType::RPAREN, std::string(1, advance()), line_, hasSpace}); continue; }
        if (c == '[') { pushToken({TokenType::LBRACKET, std::string(1, advance()), line_, hasSpace}); continue; }
        if (c == ']') { pushToken({TokenType::RBRACKET, std::string(1, advance()), line_, hasSpace}); continue; }
        if (c == '{') { pushToken({TokenType::LBRACE, std::string(1, advance()), line_, hasSpace}); continue; }
        if (c == '}') { pushToken({TokenType::RBRACE, std::string(1, advance()), line_, hasSpace}); continue; }
        if (c == ':') { pushToken({TokenType::COLON, std::string(1, advance()), line_, hasSpace}); continue; }
        if (c == ',') { pushToken({TokenType::COMMA, std::string(1, advance()), line_, hasSpace}); continue; }

        // 7. Backslash (Highest Priority / Line Continuation)
        if (c == '\\') {
            advance(); // consume \
            if (isAtEnd()) break;
            if (peek() == '\n') {
                advance(); line_++; hasSpace = true; continue; // Continuation with a space
            }
            if (peek() == '\r') {
                advance();
                if (peek() == '\n') { advance(); line_++; hasSpace = true; continue; }
            }
            pos_--; // backtrack to \ and let readAtom handle it as an escaped char
            pushToken(readAtom(hasSpace));
            continue;
        }

        // 8. Atoms (Now handles '.' as normal char)
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
    advance(); // consume "
    std::string value;
    while (!isAtEnd() && peek() != '"' && peek() != '\n') {
        if (peek() == '\\') {
            advance();
            if (!isAtEnd()) {
                char esc = advance();
                if (esc == 'n') value += '\n';
                else if (esc == 't') value += '\t';
                else if (esc == 'r') value += '\r';
                else value += esc;
            }
        } else {
            value += advance();
        }
    }
    if (!isAtEnd() && peek() == '"') advance(); // consume "
    return {TokenType::STRING, value, line_, spaceBefore};
}

Token Lexer::readAtom(bool spaceBefore) {
    std::string value;
    while (!isAtEnd()) {
        char c = peek();
        if (c == '\\') {
            advance(); // consume \
            if (isAtEnd()) break;
            if (peek() == '\n') {
                advance(); line_++; continue; // Continuation
            }
            value += advance();
            continue;
        }
        // Removed '.' from delimiter list
        if (std::isspace(c) || c == '\'' || c == '"' || c == '(' || c == ')' || 
            c == '[' || c == ']' || c == '{' || c == '}' || c == ':' || c == ',') {
            break;
        }
        if (c == '/' && (peekNext() == '/' || peekNext() == '*')) {
            break;
        }
        value += advance();
    }
    return {TokenType::ATOM, value, line_, spaceBefore};
}
