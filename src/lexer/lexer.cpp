#include "lexer.hpp"
#include <unordered_map>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <iostream>

static const std::unordered_map<std::string, TokenType> keywords = {
    {"func", TokenType::FUNC},
    {"print", TokenType::PRINT},
    {"println", TokenType::PRINTLN},
    {"loop", TokenType::LOOP},
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"return", TokenType::RETURN},
    {"var", TokenType::VAR},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE},
    {"null", TokenType::NULL_TOKEN},
    {"break", TokenType::BREAK}
};

Lexer::Lexer(const std::string& source)
    : source_(source), current_(0), start_(0), line_(1), column_(1) {}

Token Lexer::nextToken() {
    skipWhitespace();

    start_ = current_;

    if (isAtEnd()) {
        return Token(TokenType::EOF_TOKEN, "EOF", line_, column_);
    }

    if (!tokenBuffer_.empty()) {
        Token token = tokenBuffer_.front();
        tokenBuffer_.pop();
        return token;
    }

    return scanToken();
}

bool Lexer::isEOF() const {
    return isAtEnd();
}

char Lexer::advance() {
    char c = source_[current_++];
    if (c == '\n') {
        line_++;
        column_ = 1;
    } else {
        column_++;
    }
    return c;
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source_[current_] != expected) return false;
    current_++;
    column_++;
    return true;
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source_[current_];
}

char Lexer::peekNext() const {
    if (current_ + 1 >= source_.length()) return '\0';
    return source_[current_ + 1];
}

bool Lexer::isAtEnd() const {
    return current_ >= source_.length();
}

void Lexer::skipWhitespace() {
    while (true) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                advance();
                break;
            case '/':
                if (peekNext() == '/') {
                    skipComment();
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

void Lexer::skipComment() {
    while (peek() != '\n' && !isAtEnd()) advance();
}

Token Lexer::scanToken() {
    char c = advance();

    if (std::isalpha(c) || c == '_') {
        return scanIdentifier();
    }

    if (std::isdigit(c)) {
        return scanNumber();
    }

    switch (c) {
        case '(': return Token(TokenType::LPAREN, "(", line_, column_ - 1);
        case ')': return Token(TokenType::RPAREN, ")", line_, column_ - 1);
        case '{': return Token(TokenType::LBRACE, "{", line_, column_ - 1);
        case '}': return Token(TokenType::RBRACE, "}", line_, column_ - 1);
        case '[': return Token(TokenType::LBRACKET, "[", line_, column_ - 1);
        case ']': return Token(TokenType::RBRACKET, "]", line_, column_ - 1);
        case ';': return Token(TokenType::SEMICOLON, ";", line_, column_ - 1);
        case ',': return Token(TokenType::COMMA, ",", line_, column_ - 1);
        case '.': return Token(TokenType::DOT, ".", line_, column_ - 1);
        case '-': return Token(TokenType::MINUS, "-", line_, column_ - 1);
        case '+': return Token(TokenType::PLUS, "+", line_, column_ - 1);
        case '/': return Token(TokenType::DIVIDE, "/", line_, column_ - 1);
        case '*': return Token(TokenType::MULTIPLY, "*", line_, column_ - 1);
        case '%': return Token(TokenType::MODULO, "%", line_, column_ - 1);
        case '!':
            if (match('=')) {
                return Token(TokenType::NOT_EQUALS, "!=", line_, column_ - 2);
            } else {
                return Token(TokenType::BANG, "!", line_, column_ - 1);
            }
        case '=':
            if (match('=')) {
                return Token(TokenType::EQUALS, "==", line_, column_ - 2);
            } else {
                return Token(TokenType::ASSIGN, "=", line_, column_ - 1);
            }
        case '<':
            if (match('=')) {
                return Token(TokenType::LESS_EQ, "<=", line_, column_ - 2);
            } else {
                return Token(TokenType::LESS, "<", line_, column_ - 1);
            }
        case '>':
            if (match('=')) {
                return Token(TokenType::GREATER_EQ, ">=", line_, column_ - 2);
            } else {
                return Token(TokenType::GREATER, ">", line_, column_ - 1);
            }
        case '&':
            if (match('&')) {
                return Token(TokenType::AND, "&&", line_, column_ - 2);
            } else {
                return Token(TokenType::ERROR, "Expected '&' after '&'", line_, column_ - 1);
            }
        case '|':
            if (match('|')) {
                return Token(TokenType::OR, "||", line_, column_ - 2);
            } else {
                return Token(TokenType::ERROR, "Expected '|' after '|'", line_, column_ - 1);
            }
        case '"': return scanString();
        default:
            return Token(TokenType::ERROR, "Unexpected character", line_, column_ - 1);
    }
}

Token Lexer::scanIdentifier() {
    while (std::isalnum(peek()) || peek() == '_') advance();

    std::string text = source_.substr(start_, current_ - start_);
    TokenType type = identifierType();

    if (text == "loop" && peek() == '(') {
        Token loopToken = Token(TokenType::LOOP, text, line_, column_ - text.length());

        advance();

        tokenBuffer_.push(Token(TokenType::LPAREN, "(", line_, column_ - 1));

        start_ = current_;

        return loopToken;
    }

    if (type == TokenType::IDENTIFIER) {
        return Token(type, TokenValue(text), text, line_, column_ - text.length());
    } else if (type == TokenType::TRUE) {
        return Token(type, TokenValue(true), text, line_, column_ - text.length());
    } else if (type == TokenType::FALSE) {
        return Token(type, TokenValue(false), text, line_, column_ - text.length());
    } else {
        return Token(type, text, line_, column_ - text.length());
    }
}

TokenType Lexer::identifierType() {
    std::string text = source_.substr(start_, current_ - start_);
    auto it = keywords.find(text);
    return it != keywords.end() ? it->second : TokenType::IDENTIFIER;
}

Token Lexer::scanNumber() {
    while (std::isdigit(peek())) advance();

    if (peek() == '.' && std::isdigit(peekNext())) {
        advance();

        while (std::isdigit(peek())) advance();
    }

    std::string numberStr = source_.substr(start_, current_ - start_);
    double value = std::stod(numberStr);

    return Token(TokenType::NUMBER, TokenValue(value), numberStr, line_, column_ - numberStr.length());
}

Token Lexer::scanString() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n') {
            line_++;
            column_ = 1;
        }
        advance();
    }

    if (isAtEnd()) {
        return Token(TokenType::ERROR, "Unterminated string", line_, column_);
    }

    advance();

    std::string value = source_.substr(start_ + 1, current_ - start_ - 2);
    return Token(TokenType::STRING, TokenValue(value), value, line_, column_ - value.length() - 2);
}

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::EOF_TOKEN: return "EOF";
        case TokenType::ERROR: return "ERROR";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::STRING: return "STRING";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::MULTIPLY: return "MULTIPLY";
        case TokenType::DIVIDE: return "DIVIDE";
        case TokenType::MODULO: return "MODULO";
        case TokenType::ASSIGN: return "ASSIGN";
        case TokenType::EQUALS: return "EQUALS";
        case TokenType::NOT_EQUALS: return "NOT_EQUALS";
        case TokenType::LESS: return "LESS";
        case TokenType::GREATER: return "GREATER";
        case TokenType::LESS_EQ: return "LESS_EQ";
        case TokenType::GREATER_EQ: return "GREATER_EQ";
        case TokenType::BANG: return "BANG";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::COMMA: return "COMMA";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::DOT: return "DOT";
        case TokenType::FUNC: return "FUNC";
        case TokenType::PRINT: return "PRINT";
        case TokenType::PRINTLN: return "PRINTLN";
        case TokenType::LOOP: return "LOOP";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::RETURN: return "RETURN";
        case TokenType::VAR: return "VAR";
        case TokenType::TRUE: return "TRUE";
        case TokenType::FALSE: return "FALSE";
        case TokenType::NULL_TOKEN: return "NULL";
        case TokenType::BREAK: return "BREAK";
        default: return "UNKNOWN";
    }
}

std::string tokenValueToString(const TokenValue& value) {
    if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    } else if (std::holds_alternative<double>(value)) {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << std::get<double>(value);
        return ss.str();
    } else if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? "true" : "false";
    }
    return "";
}