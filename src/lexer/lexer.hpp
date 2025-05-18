#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <memory>
#include <vector>
#include <variant>
#include <optional>
#include <queue>

// Типы токенов
enum class TokenType {
    EOF_TOKEN,
    ERROR,
    IDENTIFIER,
    NUMBER,
    STRING,

    PLUS,        // +
    MINUS,       // -
    MULTIPLY,    // *
    DIVIDE,      // /
    MODULO,      // %
    ASSIGN,      // =
    EQUALS,      // ==
    NOT_EQUALS,  // !=
    LESS,        // <
    GREATER,     // >
    LESS_EQ,     // <=
    GREATER_EQ,  // >=
    BANG,        // !
    AND,         // &&
    OR,          // ||

    LPAREN,      // (
    RPAREN,      // )
    LBRACE,      // {
    RBRACE,      // }
    LBRACKET,    // [
    RBRACKET,    // ]
    COMMA,       // ,
    SEMICOLON,   // ;
    DOT,         // .

    FUNC,        // func
    PRINT,       // print
    PRINTLN,     // println
    LOOP,        // loop
    IF,          // if
    ELSE,        // else
    RETURN,      // return
    VAR,         // var
    TRUE,        // true
    FALSE,       // false
    NULL_TOKEN,  // null
    BREAK        // break
};

using TokenValue = std::variant<std::string, double, bool>;

struct Token {
    TokenType type;
    TokenValue value;
    size_t line;
    size_t column;
    std::string lexeme;

    Token(TokenType type, const std::string& lexeme, size_t line, size_t column)
        : type(type), value(std::string()), line(line), column(column), lexeme(lexeme) {}

    Token(TokenType type, const TokenValue& value, const std::string& lexeme, size_t line, size_t column)
        : type(type), value(value), line(line), column(column), lexeme(lexeme) {}

    Token() = default;
};

class Lexer {
public:
    explicit Lexer(const std::string& source);

    Token nextToken();

    bool isEOF() const;

    size_t getCurrentLine() const { return line_; }
    size_t getCurrentColumn() const { return column_; }

private:
    std::string source_;
    size_t current_;
    size_t start_;
    size_t line_;
    size_t column_;
    std::queue<Token> tokenBuffer_;

    char advance();
    bool match(char expected);
    char peek() const;
    char peekNext() const;
    bool isAtEnd() const;

    Token scanToken();
    Token scanIdentifier();
    Token scanNumber();
    Token scanString();
    void skipWhitespace();
    void skipComment();

    TokenType checkKeyword(size_t start, size_t length, const std::string& rest, TokenType type);
    TokenType identifierType();
public:

    Lexer() = default;
};

std::string tokenTypeToString(TokenType type);
std::string tokenValueToString(const TokenValue& value);

#endif // LEXER_HPP