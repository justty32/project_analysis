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
    Value result;

    if (!isAtEndOfLine() && peek().type == TokenType::QUOTE) {
        advance(); 
        
        if (!isAtEndOfLine() && peek().type == TokenType::QUOTE && !peek().hasSpaceBefore) {
            advance();
            // Check if it's the end of an atom/line or the end of a paren list
            if (isAtEndOfLine() || peek().hasSpaceBefore || peek().type == TokenType::RPAREN) {
                result = Value::makeList({});
            } else {
                result = Value::makeList({Value::makeAtom("quote"), parseExpression()});
            }
        } else {
            Value first = parsePrimary();
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
                    std::list<Value> l;
                    for (const auto& v : chain) l.push_back(v);
                    result = Value::makeList(l);
                } else {
                    Value node = chain.back();
                    for (int i = (int)chain.size() - 2; i >= 0; --i) {
                        node = Value::makeList({chain[i], node});
                    }
                    result = node;
                }
            } else {
                result = Value::makeList({Value::makeAtom("quote"), first});
            }
        }
    } else {
        Value first = parsePrimary();
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
                if (chain.size() == 1) {
                    result = Value::makeList({chain[0]});
                } else {
                    Value node = Value::makeList({chain[0], chain[1]});
                    for (size_t i = 2; i < chain.size(); ++i) {
                        node = Value::makeList({node, chain[i]});
                    }
                    result = node;
                }
            } else {
                std::list<Value> l;
                for (const auto& v : chain) l.push_back(v);
                result = Value::makeList(l);
            }
        } else {
            result = first;
        }
    }

    return result;
}

Value Parser::parsePrimary() {
    if (isAtEndOfLine()) throw std::runtime_error("Unexpected end of line");

    if (match(TokenType::LPAREN)) return parseList();
    if (match(TokenType::LBRACKET)) return parseArray();
    if (match(TokenType::LBRACE)) return parseDictOrSet();

    const Token& t = advance();
    if (t.type == TokenType::STRING) {
        std::list<Value> strList;
        strList.push_back(Value::makeAtom("str"));
        
        std::string val = t.value;
        std::string current;
        for (size_t i = 0; i < val.length(); ++i) {
            if (val[i] == ' ') {
                if (!current.empty()) {
                    strList.push_back(Value::makeAtom(current));
                    current.clear();
                }
                int count = 0;
                while (i < val.length() && val[i] == ' ') {
                    count++;
                    i++;
                }
                i--; // Step back for loop increment
                std::list<Value> spaceList = {Value::makeAtom("ntimes_space"), Value::makeNumber(count)};
                strList.push_back(Value::makeList(spaceList));
            } else {
                current += val[i];
            }
        }
        if (!current.empty()) strList.push_back(Value::makeAtom(current));
        return Value::makeList(strList);
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
                // Generate (quote (key val))
                std::list<Value> pair;
                pair.push_back(key);
                pair.push_back(val);
                result.push_back(Value::makeList({Value::makeAtom("quote"), Value::makeList(pair)}));
            } else {
                std::list<Value> pair;
                pair.push_back(key);
                pair.push_back(Value::makeAtom("nil"));
                result.push_back(Value::makeList({Value::makeAtom("quote"), Value::makeList(pair)}));
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
