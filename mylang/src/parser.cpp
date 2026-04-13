#include "parser.hpp"
#include <stdexcept>
#include <iostream>

std::vector<Value> Parser::parse() {
    std::vector<Value> results;
    while (linePos_ < lines_.size()) {
        if (lines_[linePos_].empty()) {
            linePos_++; pos_ = 0; continue;
        }
        try {
            results.push_back(parseLine());
        } catch (const std::exception& e) {
            std::cerr << "Parser error at line " << linePos_ + 1 << ": " << e.what() << std::endl;
        }
        linePos_++; pos_ = 0;
    }
    return results;
}

Value Parser::parseLine() {
    std::list<Value> expressions;
    while (!isAtEndOfLine()) {
        expressions.push_back(parseExpression());
    }
    if (expressions.empty()) return Value::makeList({});
    if (expressions.size() == 1) return expressions.front();
    return Value::makeList(expressions);
}

Value Parser::parseExpression() {
    // 1. Case: Forward Nesting or Symmetry Start
    if (!isAtEndOfLine() && peek().type == TokenType::QUOTE) {
        advance(); // consume '
        
        // Handle ''
        if (!isAtEndOfLine() && peek().type == TokenType::QUOTE && !peek().hasSpaceBefore) {
            advance();
            if (isAtEndOfLine() || peek().hasSpaceBefore) return Value::makeList({});
            return Value::makeList({Value::makeAtom("quote"), parseExpression()});
        }
        
        Value first = parsePrimary();
        if (!isAtEndOfLine() && peek().type == TokenType::QUOTE && !peek().hasSpaceBefore) {
            // Middle quote chain: 'A'B...
            std::vector<Value> chain;
            chain.push_back(first);
            bool trailingQuote = false;

            while (!isAtEndOfLine() && peek().type == TokenType::QUOTE && !peek().hasSpaceBefore) {
                advance(); // consume middle '
                if (isAtEndOfLine() || peek().hasSpaceBefore) {
                    trailingQuote = true; break;
                }
                chain.push_back(parsePrimary());
            }

            if (trailingQuote) {
                // Symmetry: 'A'B'C' -> (A B C) (Flat)
                std::list<Value> l;
                for (const auto& v : chain) l.push_back(v);
                return Value::makeList(l);
            } else {
                // Nested: 'A'B'C -> (A (B C)) (Right-associative)
                // We need to unwrap the recursion for 'A'B'C...
                // Simpler: build it from right to left
                Value result = chain.back();
                for (int i = (int)chain.size() - 2; i >= 0; --i) {
                    result = Value::makeList({chain[i], result});
                }
                return result;
            }
        } else {
            // Simple quote: 'A -> (quote A)
            return Value::makeList({Value::makeAtom("quote"), first});
        }
    }

    // 2. Base Primary
    Value base = parsePrimary();

    // 3. Handle Middle or Trailing Quote Chain (Backward/Straight)
    std::vector<Value> chain;
    chain.push_back(base);
    bool trailingQuote = false;

    while (!isAtEndOfLine() && peek().type == TokenType::QUOTE && !peek().hasSpaceBefore) {
        advance(); // consume '
        if (isAtEndOfLine() || peek().hasSpaceBefore) {
            trailingQuote = true;
            break;
        }
        chain.push_back(parsePrimary());
    }

    if (chain.size() == 1) {
        if (trailingQuote) return Value::makeList({chain[0]});
        return chain[0];
    }

    if (trailingQuote) {
        // Backward Nesting: A'B'C' -> ((A B) C) (Left-associative)
        Value result = Value::makeList({chain[0], chain[1]});
        for (size_t i = 2; i < chain.size(); ++i) {
            result = Value::makeList({result, chain[i]});
        }
        return result;
    } else {
        // Straight Connection: A'B'C -> (A B C) (Flat list)
        std::list<Value> l;
        for (const auto& v : chain) l.push_back(v);
        return Value::makeList(l);
    }
}

Value Parser::parsePrimary() {
    if (isAtEndOfLine()) throw std::runtime_error("Unexpected end of line");

    if (match(TokenType::LPAREN)) return parseList();
    if (match(TokenType::LBRACKET)) return parseArray();
    if (match(TokenType::LBRACE)) return parseDictOrSet();

    const Token& t = advance();
    if (t.type == TokenType::STRING) return Value::makeList({Value::makeAtom("str"), Value::makeString(t.value)});
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
        if (peek().type == TokenType::COMMA) { advance(); continue; }
        items.push_back(parseExpression());
    }
    if (!match(TokenType::RBRACKET)) throw std::runtime_error("Unclosed bracket");
    return Value::makeList(items);
}

Value Parser::parseDictOrSet() {
    std::list<Value> rawItems;
    bool hasColon = false;
    while (!isAtEndOfLine() && peek().type != TokenType::RBRACE) {
        if (peek().type == TokenType::COMMA) { advance(); continue; }
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
                result.push_back(Value::makeList({Value::makeAtom("pair"), key, val}));
            } else {
                result.push_back(Value::makeList({Value::makeAtom("pair"), key, Value::makeAtom("nil")}));
                if (it != rawItems.end()) it++;
            }
        }
    } else {
        result.push_back(Value::makeAtom("set"));
        for (const auto& item : rawItems) result.push_back(item);
    }
    return Value::makeList(result);
}

const Token& Parser::peek() const {
    if (linePos_ >= lines_.size() || pos_ >= lines_[linePos_].size()) return eofToken_;
    return lines_[linePos_][pos_];
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
