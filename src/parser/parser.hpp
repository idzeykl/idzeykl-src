#ifndef PARSER_HPP
#define PARSER_HPP

#include "../lexer/lexer.hpp"
#include <memory>
#include <vector>
#include <stdexcept>
#include <string>

class ASTNode {
public:
    enum class Type {
        BLOCK,
        VARIABLE_DECLARATION,
        FUNCTION_DECLARATION,
        LOOP,
        IF,
        PRINT,
        RETURN,
        BREAK,
        EXPRESSION,
        BINARY,
        UNARY,
        IDENTIFIER,
        LITERAL,
        CALL,
        ARRAY,
        ARRAY_ACCESS,
        PROPERTY_ACCESS
    };

    virtual ~ASTNode() = default;
    virtual Type getType() const = 0;
    virtual ASTNode* clone() const = 0;
};

class Statement : public ASTNode {
public:
    virtual Statement* clone() const override = 0;
};
class Expression : public ASTNode {
public:
    virtual Expression* clone() const override = 0;
};

class BlockStatement : public Statement {
public:
    std::vector<std::unique_ptr<Statement>> statements;

    Type getType() const override { return Type::BLOCK; }
    BlockStatement* clone() const override {
        auto copy = new BlockStatement();
        for (const auto& stmt : statements) {
            copy->statements.push_back(std::unique_ptr<Statement>(static_cast<Statement*>(stmt->clone())));
        }
        return copy;
    }
};

class VariableDeclaration : public Statement {
public:
    std::string identifier;
    std::unique_ptr<Expression> initializer;

    Type getType() const override { return Type::VARIABLE_DECLARATION; }
    VariableDeclaration* clone() const override {
        auto copy = new VariableDeclaration();
        copy->identifier = identifier;
        if (initializer) {
            copy->initializer = std::unique_ptr<Expression>(static_cast<Expression*>(initializer->clone()));
        }
        return copy;
    }
};

class FunctionDeclaration : public Statement {
public:
    std::string name;
    std::vector<std::string> parameters;
    std::unique_ptr<BlockStatement> body;

    Type getType() const override { return Type::FUNCTION_DECLARATION; }
    FunctionDeclaration* clone() const override {
        auto copy = new FunctionDeclaration();
        copy->name = name;
        copy->parameters = parameters;
        if (body) {
            copy->body = std::unique_ptr<BlockStatement>(body->clone());
        }
        return copy;
    }
};

class LoopStatement : public Statement {
public:
    std::unique_ptr<Statement> init;
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Expression> increment;
    std::unique_ptr<BlockStatement> body;

    Type getType() const override { return Type::LOOP; }
    LoopStatement* clone() const override {
        auto copy = new LoopStatement();
        if (init) {
            copy->init = std::unique_ptr<Statement>(static_cast<Statement*>(init->clone()));
        }
        if (condition) {
            copy->condition = std::unique_ptr<Expression>(static_cast<Expression*>(condition->clone()));
        }
        if (increment) {
            copy->increment = std::unique_ptr<Expression>(static_cast<Expression*>(increment->clone()));
        }
        if (body) {
            copy->body = std::unique_ptr<BlockStatement>(body->clone());
        }
        return copy;
    }
};

class IfStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<BlockStatement> thenBranch;
    std::unique_ptr<BlockStatement> elseBranch;

    Type getType() const override { return Type::IF; }
    IfStatement* clone() const override {
        auto copy = new IfStatement();
        if (condition) {
            copy->condition = std::unique_ptr<Expression>(static_cast<Expression*>(condition->clone()));
        }
        if (thenBranch) {
            copy->thenBranch = std::unique_ptr<BlockStatement>(thenBranch->clone());
        }
        if (elseBranch) {
            copy->elseBranch = std::unique_ptr<BlockStatement>(elseBranch->clone());
        }
        return copy;
    }
};

class PrintStatement : public Statement {
public:
    bool isPrintln;
    std::vector<std::unique_ptr<Expression>> args;
    std::string directString;

    Type getType() const override { return Type::PRINT; }
    PrintStatement* clone() const override {
        auto copy = new PrintStatement();
        copy->isPrintln = isPrintln;
        copy->directString = directString;
        for (const auto& arg : args) {
            copy->args.push_back(std::unique_ptr<Expression>(static_cast<Expression*>(arg->clone())));
        }
        return copy;
    }
};

class ReturnStatement : public Statement {
public:
    std::unique_ptr<Expression> value;

    Type getType() const override { return Type::RETURN; }
    ReturnStatement* clone() const override {
        auto copy = new ReturnStatement();
        if (value) {
            copy->value = std::unique_ptr<Expression>(static_cast<Expression*>(value->clone()));
        }
        return copy;
    }
};

class BreakStatement : public Statement {
public:
    Type getType() const override { return Type::BREAK; }
    BreakStatement* clone() const override {
        return new BreakStatement();
    }
};

class ExpressionStatement : public Statement {
public:
    std::unique_ptr<Expression> expr;

    Type getType() const override { return Type::EXPRESSION; }
    ExpressionStatement* clone() const override {
        auto copy = new ExpressionStatement();
        if (expr) {
            copy->expr = std::unique_ptr<Expression>(static_cast<Expression*>(expr->clone()));
        }
        return copy;
    }
};

class BinaryExpression : public Expression {
public:
    TokenType op;
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;

    Type getType() const override { return Type::BINARY; }
    BinaryExpression* clone() const override {
        auto copy = new BinaryExpression();
        copy->op = op;
        if (left) {
            copy->left = std::unique_ptr<Expression>(static_cast<Expression*>(left->clone()));
        }
        if (right) {
            copy->right = std::unique_ptr<Expression>(static_cast<Expression*>(right->clone()));
        }
        return copy;
    }
};

class UnaryExpression : public Expression {
public:
    TokenType op;
    std::unique_ptr<Expression> expr;

    Type getType() const override { return Type::UNARY; }
    UnaryExpression* clone() const override {
        auto copy = new UnaryExpression();
        copy->op = op;
        if (expr) {
            copy->expr = std::unique_ptr<Expression>(static_cast<Expression*>(expr->clone()));
        }
        return copy;
    }
};

class Identifier : public Expression {
public:
    std::string name;

    Type getType() const override { return Type::IDENTIFIER; }
    Identifier* clone() const override {
        auto copy = new Identifier();
        copy->name = name;
        return copy;
    }
};

class Literal : public Expression {
public:
    TokenValue value;

    Type getType() const override { return Type::LITERAL; }
    Literal* clone() const override {
        auto copy = new Literal();
        copy->value = value;
        return copy;
    }
};

class CallExpression : public Expression {
public:
    std::unique_ptr<Expression> callee;
    std::vector<std::unique_ptr<Expression>> arguments;

    Type getType() const override { return Type::CALL; }
    CallExpression* clone() const override {
        auto copy = new CallExpression();
        if (callee) {
            copy->callee = std::unique_ptr<Expression>(static_cast<Expression*>(callee->clone()));
        }
        for (const auto& arg : arguments) {
            copy->arguments.push_back(std::unique_ptr<Expression>(static_cast<Expression*>(arg->clone())));
        }
        return copy;
    }
};

class ArrayExpression : public Expression {
public:
    std::vector<std::unique_ptr<Expression>> elements;

    Type getType() const override { return Type::ARRAY; }
    ArrayExpression* clone() const override {
        auto copy = new ArrayExpression();
        for (const auto& element : elements) {
            copy->elements.push_back(std::unique_ptr<Expression>(static_cast<Expression*>(element->clone())));
        }
        return copy;
    }
};

class ArrayAccessExpression : public Expression {
public:
    std::unique_ptr<Expression> array;
    std::unique_ptr<Expression> index;

    Type getType() const override { return Type::ARRAY_ACCESS; }
    ArrayAccessExpression* clone() const override {
        auto copy = new ArrayAccessExpression();
        if (array) {
            copy->array = std::unique_ptr<Expression>(static_cast<Expression*>(array->clone()));
        }
        if (index) {
            copy->index = std::unique_ptr<Expression>(static_cast<Expression*>(index->clone()));
        }
        return copy;
    }
};

class PropertyAccessExpression : public Expression {
public:
    std::unique_ptr<Expression> object;
    std::string property;

    Type getType() const override { return Type::PROPERTY_ACCESS; }
    PropertyAccessExpression* clone() const override {
        auto copy = new PropertyAccessExpression();
        if (object) {
            copy->object = std::unique_ptr<Expression>(static_cast<Expression*>(object->clone()));
        }
        copy->property = property;
        return copy;
    }
};

class Parser {
public:
    Parser(Lexer& lexer);
    std::unique_ptr<BlockStatement> parse();

private:
    Lexer& lexer_;
    Token currentToken_;

    void advance();
    void consume(TokenType expectedType, const std::string& errorMessage);
    bool match(TokenType type);
    bool check(TokenType type) const;

    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<BlockStatement> parseBlock();
    std::unique_ptr<VariableDeclaration> parseVariableDeclaration();
    std::unique_ptr<FunctionDeclaration> parseFunctionDeclaration();
    std::unique_ptr<LoopStatement> parseLoopStatement();
    std::unique_ptr<IfStatement> parseIfStatement();
    std::unique_ptr<PrintStatement> parsePrintStatement();
    std::unique_ptr<ReturnStatement> parseReturnStatement();
    std::unique_ptr<BreakStatement> parseBreakStatement();
    std::unique_ptr<ExpressionStatement> parseExpressionStatement();

    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseAssignment();
    std::unique_ptr<Expression> parseLogicalOr();
    std::unique_ptr<Expression> parseLogicalAnd();
    std::unique_ptr<Expression> parseEquality();
    std::unique_ptr<Expression> parseComparison();
    std::unique_ptr<Expression> parseTerm();
    std::unique_ptr<Expression> parseFactor();
    std::unique_ptr<Expression> parseUnary();
    std::unique_ptr<Expression> parsePrimary();
    std::unique_ptr<Expression> parseArrayExpression();
    std::unique_ptr<Expression> parseArrayAccess(std::unique_ptr<Expression> array);
    std::unique_ptr<Expression> parsePropertyAccess(std::unique_ptr<Expression> object);

    std::vector<std::unique_ptr<Expression>> parseExpressionList();
    std::vector<std::string> parseParameters();
};

#endif // PARSER_HPP