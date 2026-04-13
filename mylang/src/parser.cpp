#include "parser.hpp"
#include <stdexcept>
#include <iostream>

std::vector<Value> Parser::parse() {
    return parseWithIndent(0);
}

std::vector<Value> Parser::parseWithIndent(int targetIndent) {
    std::vector<Value> results;
    while (linePos_ < lines_.size()) {
        const auto& lineTokens = lines_[linePos_];
        if (lineTokens.empty()) {
            linePos_++; continue;
        }

        int currentIndent = lineTokens[0].indent;

        if (currentIndent < targetIndent) {
            break;
        }

        if (currentIndent > targetIndent) {
            // Jump case: indentation deeper than expected without a parent line
            std::vector<Value> subResults = parseWithIndent(currentIndent);
            results.insert(results.end(), subResults.begin(), subResults.end());
            continue;
        }

        // currentIndent == targetIndent
        pos_ = 0;
        Value currentLineValue = parseLineContent();
        linePos_++;

        // Attach all subsequent lines that are deeper as children
        if (linePos_ < lines_.size() && !lines_[linePos_].empty() && lines_[linePos_][0].indent > targetIndent) {
            std::vector<Value> children = parseWithIndent(targetIndent + 1);
            
            std::list<Value> combined;
            if (currentLineValue.type == Value::Type::LIST) {
                auto items = currentLineValue.as<std::list<Value>>();
                combined.assign(items.begin(), items.end());
            } else {
                combined.push_back(currentLineValue);
            }
            
            for (auto const& child : children) {
                combined.push_back(child);
            }
            results.push_back(Value::makeList(combined));
        } else {
            results.push_back(currentLineValue);
        }
    }
    return results;
}

Value Parser::parseLineContent() {
    std::list<Value> expressions;
    while (!isAtEndOfLine()) {
        expressions.push_back(parseExpression());
    }
    
    // If a line is a single list, return it directly to avoid double wrapping.
    // Example: (a b c) on a line should be (a b c), not ((a b c)).
    if (expressions.size() == 1) {
        const Value& first = expressions.front();
        if (first.type == Value::Type::LIST) {
            return first;
        }
    }
    
    return Value::makeList(expressions);
}

Value Parser::parseExpression() {
    if (isAtEndOfLine()) throw std::runtime_error("Unexpected end of line");

    // Handle unary prefixes: ', `, ,, ,@
    if (peek().type == TokenType::QUOTE || 
        peek().type == TokenType::BACKQUOTE || 
        peek().type == TokenType::UNQUOTE || 
        peek().type == TokenType::UNQUOTE_SPLICING) {
        
        Token t = advance();
        std::string op;
        if (t.type == TokenType::QUOTE) op = "quote";
        else if (t.type == TokenType::BACKQUOTE) op = "backquote";
        else if (t.type == TokenType::UNQUOTE) op = "unquote";
        else if (t.type == TokenType::UNQUOTE_SPLICING) op = "unquote_splicing";

        // Special case for '': handle ''A as 'A
        if (t.type == TokenType::QUOTE && !isAtEndOfLine() && peek().type == TokenType::QUOTE && !peek().hasSpaceBefore) {
            advance();
            if (isAtEndOfLine() || peek().hasSpaceBefore || peek().type == TokenType::RPAREN) {
                return Value::makeList({});
            }
            return Value::makeList({Value::makeAtom("quote"), parseExpression()});
        }

        Value expr = parseExpression();
        return Value::makeList({Value::makeAtom(op), expr});
    }

    Value first = parsePrimary();
    
    // Handle quote-based list shorthand: A'B -> (A B)
    if (!isAtEndOfLine() && peek().type == TokenType::QUOTE && !peek().hasSpaceBefore) {
        std::vector<Value> chain;
        chain.push_back(first);
        bool trailingQuote = false;

        while (!isAtEndOfLine() && peek().type == TokenType::QUOTE && !peek().hasSpaceBefore) {
            advance();
            if (isAtEndOfLine() || peek().hasSpaceBefore || peek().type == TokenType::RPAREN) {
                trailingQuote = true; break;
            }
            chain.push_back(parsePrimary());
        }

        if (trailingQuote) {
            if (chain.size() == 1) return Value::makeList({chain[0]});
            Value node = Value::makeList({chain[0], chain[1]});
            for (size_t i = 2; i < chain.size(); ++i) node = Value::makeList({node, chain[i]});
            return node;
        } else {
            std::list<Value> l;
            for (const auto& v : chain) l.push_back(v);
            return Value::makeList(l);
        }
    }

    return first;
}

Value Parser::parsePrimary() {
    if (isAtEndOfLine()) throw std::runtime_error("Unexpected end of line");

    if (match(TokenType::LPAREN)) return parseList();
    if (match(TokenType::LBRACKET)) return parseArray();
    if (match(TokenType::LBRACE)) return parseDictOrSet();

    const Token& t = advance();
    if (t.type == TokenType::STRING) {
        return Value::makeList({Value::makeAtom("str"), Value::makeString(t.value)});
    }
    if (t.type == TokenType::ATOM) return Value::makeAtom(t.value);
    
    throw std::runtime_error("Unexpected token in parsePrimary: " + t.value);
}

Value Parser::parseList() {
    std::list<Value> items;
    while (!isAtEndOfLine() && peek().type != TokenType::RPAREN) items.push_back(parseExpression());
    if (!match(TokenType::RPAREN)) throw std::runtime_error("Unclosed parenthesis");
    return Value::makeList(items);
}

Value Parser::parseArray() {
    std::list<Value> items;
    items.push_back(Value::makeAtom("arr"));
    while (!isAtEndOfLine() && peek().type != TokenType::RBRACKET) {
        // Commas are no longer treated as separators by the Lexer (they are UNQUOTE now)
        // If a comma appears here without being part of a backquote, it will be parsed as (unquote ...)
        items.push_back(parseExpression());
    }
    if (!match(TokenType::RBRACKET)) throw std::runtime_error("Unclosed bracket");
    return Value::makeList(items);
}

Value Parser::parseDictOrSet() {
    std::list<Value> rawItems;
    bool hasColon = false;
    while (!isAtEndOfLine() && peek().type != TokenType::RBRACE) {
        if (peek().type == TokenType::COLON) { hasColon = true; rawItems.push_back(Value::makeAtom(":")); advance(); continue; }
        rawItems.push_back(parseExpression());
    }
    if (!match(TokenType::RBRACE)) throw std::runtime_error("Unclosed brace");

    std::list<Value> result;
    if (hasColon) {
        result.push_back(Value::makeAtom("dict"));
        auto it = rawItems.begin();
        while (it != rawItems.end()) {
            Value key = *it; it++;
            if (it != rawItems.end() && it->type == Value::Type::ATOM && it->as<std::string>() == ":") {
                it++;
                Value val = (it != rawItems.end()) ? *it : Value::makeAtom("nil");
                if (it != rawItems.end()) it++;
                std::list<Value> pair = {key, val};
                result.push_back(Value::makeList({Value::makeAtom("quote"), Value::makeList(pair)}));
            } else {
                std::list<Value> pair = {key, Value::makeAtom("nil")};
                result.push_back(Value::makeList({Value::makeAtom("quote"), Value::makeList(pair)}));
            }
        }
    } else {
        result.push_back(Value::makeAtom("set"));
        for (const auto& item : rawItems) result.push_back(item);
    }
    return Value::makeList(result);
}

const Token& Parser::peek() const {
    if (linePos_ >= lines_.size()) return eofToken_;
    const auto& line = lines_[linePos_];
    if (pos_ >= line.size()) return eofToken_;
    return line[pos_];
}

const Token& Parser::advance() {
    const Token& t = peek();
    if (linePos_ < lines_.size() && pos_ < lines_[linePos_].size()) pos_++;
    return t;
}

bool Parser::isAtEnd() const { return linePos_ >= lines_.size(); }
bool Parser::isAtEndOfLine() const { return linePos_ >= lines_.size() || pos_ >= lines_[linePos_].size(); }
bool Parser::match(TokenType type) {
    if (isAtEndOfLine()) return false;
    if (peek().type == type) { advance(); return true; }
    return false;
}
