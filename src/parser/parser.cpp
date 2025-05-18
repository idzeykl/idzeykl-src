#include "parser.hpp"
#include <iostream>

Parser::Parser(Lexer& lexer) : lexer_(lexer) {
    advance();
}

void Parser::advance() {
    currentToken_ = lexer_.nextToken();
}

void Parser::consume(TokenType expectedType, const std::string& errorMessage) {
    if (currentToken_.type == expectedType) {
        advance();
    }
    else {
        throw std::runtime_error(errorMessage + ". Found: " +
            tokenTypeToString(currentToken_.type) + " at line " +
            std::to_string(currentToken_.line) + ", column " +
            std::to_string(currentToken_.column));
    }
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const {
    return currentToken_.type == type;
}

std::unique_ptr<BlockStatement> Parser::parse() {
    auto block = std::make_unique<BlockStatement>();
    while (!lexer_.isEOF()) {
        block->statements.push_back(parseStatement());
    }
    return block;
}

std::unique_ptr<Statement> Parser::parseStatement() {
    switch (currentToken_.type) {
    case TokenType::LBRACE: return parseBlock();
    case TokenType::VAR: return parseVariableDeclaration();
    case TokenType::FUNC: return parseFunctionDeclaration();
    case TokenType::LOOP: return parseLoopStatement();
    case TokenType::IF: return parseIfStatement();
    case TokenType::PRINT:
    case TokenType::PRINTLN: return parsePrintStatement();
    case TokenType::RETURN: return parseReturnStatement();
    case TokenType::BREAK: return parseBreakStatement();
    default: return parseExpressionStatement();
    }
}

std::unique_ptr<BlockStatement> Parser::parseBlock() {
    consume(TokenType::LBRACE, "Expected '{' to start block");
    auto block = std::make_unique<BlockStatement>();

    while (!check(TokenType::RBRACE) && !lexer_.isEOF()) {
        block->statements.push_back(parseStatement());
    }

    consume(TokenType::RBRACE, "Expected '}' to end block");
    return block;
}

std::unique_ptr<VariableDeclaration> Parser::parseVariableDeclaration() {
    consume(TokenType::VAR, "Expected 'var' keyword");
    auto declaration = std::make_unique<VariableDeclaration>();

    if (!check(TokenType::IDENTIFIER)) {
        throw std::runtime_error("Expected variable name");
    }
    declaration->identifier = std::get<std::string>(currentToken_.value);
    advance();

    bool isArrayDeclaration = false;
    if (match(TokenType::LBRACKET)) {
        consume(TokenType::RBRACKET, "Expected ']' after '['");
        isArrayDeclaration = true;
    }

    if (match(TokenType::ASSIGN)) {
        if (isArrayDeclaration && check(TokenType::LBRACKET)) {
            advance();
            auto arrayExpr = std::make_unique<ArrayExpression>();

            if (!check(TokenType::RBRACKET)) {
                do {
                    arrayExpr->elements.push_back(parseExpression());
                } while (match(TokenType::COMMA));
            }

            consume(TokenType::RBRACKET, "Expected ']' after array initializer");
            declaration->initializer = std::move(arrayExpr);
        } else {
            declaration->initializer = parseExpression();
        }
    } else if (isArrayDeclaration) {
        declaration->initializer = std::make_unique<ArrayExpression>();
    }

    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");
    return declaration;
}

std::unique_ptr<FunctionDeclaration> Parser::parseFunctionDeclaration() {
    consume(TokenType::FUNC, "Expected 'func' keyword");
    auto func = std::make_unique<FunctionDeclaration>();

    if (!check(TokenType::IDENTIFIER)) {
        throw std::runtime_error("Expected function name");
    }
    func->name = std::get<std::string>(currentToken_.value);

    advance();

    consume(TokenType::LPAREN, "Expected '(' after function name");
    func->parameters = parseParameters();
    consume(TokenType::RPAREN, "Expected ')' after parameters");

    if (check(TokenType::LBRACE)) {
        func->body = parseBlock();
    }
    else {
        consume(TokenType::SEMICOLON, "Expected ';' or block after function declaration");
    }

    return func;
}

std::unique_ptr<LoopStatement> Parser::parseLoopStatement() {
    consume(TokenType::LOOP, "Expected 'loop' keyword");

    auto loop = std::make_unique<LoopStatement>();

    if (check(TokenType::LPAREN)) {
        consume(TokenType::LPAREN, "Expected '(' after 'loop'");

        if (!check(TokenType::RPAREN)) {
            if (check(TokenType::VAR) || check(TokenType::SEMICOLON)) {
                if (check(TokenType::VAR)) {
                    loop->init = parseVariableDeclaration();
                } else if (check(TokenType::SEMICOLON)) {
                    consume(TokenType::SEMICOLON, "Expected ';' after loop initializer");
                }

                if (!check(TokenType::SEMICOLON) && !check(TokenType::RPAREN)) {
                    loop->condition = parseExpression();
                }

                if (check(TokenType::SEMICOLON)) {
                    consume(TokenType::SEMICOLON, "Expected ';' after loop condition");

                    if (!check(TokenType::RPAREN)) {
                        loop->increment = parseExpression();
                    }
                }
            } else {
                loop->condition = parseExpression();
            }
        }

        consume(TokenType::RPAREN, "Expected ')' after loop");
    }

    loop->body = parseBlock();
    return loop;
}

std::unique_ptr<IfStatement> Parser::parseIfStatement() {
    consume(TokenType::IF, "Expected 'if' keyword");
    consume(TokenType::LPAREN, "Expected '(' after 'if'");

    auto ifStmt = std::make_unique<IfStatement>();
    ifStmt->condition = parseExpression();

    consume(TokenType::RPAREN, "Expected ')' after condition");
    ifStmt->thenBranch = parseBlock();

    if (match(TokenType::ELSE)) {
        if (check(TokenType::IF)) {
            ifStmt->elseBranch = std::make_unique<BlockStatement>();
            ifStmt->elseBranch->statements.push_back(parseIfStatement());
        }
        else {
            ifStmt->elseBranch = parseBlock();
        }
    }

    return ifStmt;
}

std::unique_ptr<PrintStatement> Parser::parsePrintStatement() {
    auto printStmt = std::make_unique<PrintStatement>();
    printStmt->isPrintln = (currentToken_.type == TokenType::PRINTLN);
    advance();

    if (check(TokenType::STRING)) {
        std::unique_ptr<Expression> expr = std::make_unique<Literal>();
        static_cast<Literal*>(expr.get())->value = currentToken_.value;
        advance();

        while (match(TokenType::PLUS)) {
            auto binExpr = std::make_unique<BinaryExpression>();
            binExpr->left = std::move(expr);
            binExpr->op = TokenType::PLUS;
            binExpr->right = parseExpression();
            expr = std::move(binExpr);
        }

        printStmt->args.push_back(std::move(expr));

        if (!printStmt->isPrintln) {
            consume(TokenType::SEMICOLON, "Expected ';' after print statement");
        }
        return printStmt;
    }

    consume(TokenType::LPAREN, "Expected '(' or string after print");
    if (!check(TokenType::RPAREN)) {
        printStmt->args = parseExpressionList();
    }
    consume(TokenType::RPAREN, "Expected ')' after print arguments");
    consume(TokenType::SEMICOLON, "Expected ';' after print statement");

    return printStmt;
}

std::unique_ptr<ReturnStatement> Parser::parseReturnStatement() {
    consume(TokenType::RETURN, "Expected 'return' keyword");
    auto returnStmt = std::make_unique<ReturnStatement>();

    if (!check(TokenType::SEMICOLON)) {
        returnStmt->value = parseExpression();
    }

    consume(TokenType::SEMICOLON, "Expected ';' after return statement");
    return returnStmt;
}

std::unique_ptr<BreakStatement> Parser::parseBreakStatement() {
    consume(TokenType::BREAK, "Expected 'break' keyword");
    consume(TokenType::SEMICOLON, "Expected ';' after 'break'");
    return std::make_unique<BreakStatement>();
}

std::unique_ptr<ExpressionStatement> Parser::parseExpressionStatement() {
    auto exprStmt = std::make_unique<ExpressionStatement>();
    exprStmt->expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression");
    return exprStmt;
}

std::unique_ptr<Expression> Parser::parseExpression() {
    return parseAssignment();
}

std::unique_ptr<Expression> Parser::parseAssignment() {
    auto expr = parseLogicalOr();

    if (match(TokenType::ASSIGN)) {
        auto assign = std::make_unique<BinaryExpression>();
        assign->left = std::move(expr);
        assign->op = TokenType::ASSIGN;
        assign->right = parseAssignment();
        return assign;
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parseLogicalOr() {
    auto expr = parseLogicalAnd();

    while (check(TokenType::OR)) {
        match(TokenType::OR);
        auto binExpr = std::make_unique<BinaryExpression>();
        binExpr->left = std::move(expr);
        binExpr->op = TokenType::OR;
        binExpr->right = parseLogicalAnd();
        expr = std::move(binExpr);
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parseLogicalAnd() {
    auto expr = parseEquality();

    while (check(TokenType::AND)) {
        match(TokenType::AND);
        auto binExpr = std::make_unique<BinaryExpression>();
        binExpr->left = std::move(expr);
        binExpr->op = TokenType::AND;
        binExpr->right = parseEquality();
        expr = std::move(binExpr);
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parseEquality() {
    auto expr = parseComparison();

    while (true) {
        TokenType opType;

        if (check(TokenType::EQUALS)) {
            advance();
            opType = TokenType::EQUALS;
        } else if (check(TokenType::NOT_EQUALS)) {
            advance();
            opType = TokenType::NOT_EQUALS;
        } else {
            break;
        }

        auto binExpr = std::make_unique<BinaryExpression>();
        binExpr->left = std::move(expr);
        binExpr->op = opType;
        binExpr->right = parseComparison();
        expr = std::move(binExpr);
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parseComparison() {
    auto expr = parseTerm();

    while (true) {
        TokenType opType;

        if (check(TokenType::LESS)) {
            advance();
            opType = TokenType::LESS;
        } else if (check(TokenType::LESS_EQ)) {
            advance();
            opType = TokenType::LESS_EQ;
        } else if (check(TokenType::GREATER)) {
            advance();
            opType = TokenType::GREATER;
        } else if (check(TokenType::GREATER_EQ)) {
            advance();
            opType = TokenType::GREATER_EQ;
        } else {
            break;
        }

        auto binExpr = std::make_unique<BinaryExpression>();
        binExpr->left = std::move(expr);
        binExpr->op = opType;
        binExpr->right = parseTerm();
        expr = std::move(binExpr);
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parseTerm() {
    auto expr = parseFactor();

    while (true) {
        TokenType opType;

        if (check(TokenType::PLUS)) {
            advance();
            opType = TokenType::PLUS;
        } else if (check(TokenType::MINUS)) {
            advance();
            opType = TokenType::MINUS;
        } else {
            break;
        }

        auto binExpr = std::make_unique<BinaryExpression>();
        binExpr->left = std::move(expr);
        binExpr->op = opType;
        binExpr->right = parseFactor();
        expr = std::move(binExpr);
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parseFactor() {
    auto expr = parseUnary();

    while (true) {
        TokenType opType;

        if (check(TokenType::MULTIPLY)) {
            advance();
            opType = TokenType::MULTIPLY;
        } else if (check(TokenType::DIVIDE)) {
            advance();
            opType = TokenType::DIVIDE;
        } else if (check(TokenType::MODULO)) {
            advance();
            opType = TokenType::MODULO;
        } else {
            break;
        }

        auto binExpr = std::make_unique<BinaryExpression>();
        binExpr->left = std::move(expr);
        binExpr->op = opType;
        binExpr->right = parseUnary();
        expr = std::move(binExpr);
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parseUnary() {
    if (check(TokenType::BANG)) {
        match(TokenType::BANG);
        auto unary = std::make_unique<UnaryExpression>();
        unary->op = TokenType::BANG;
        unary->expr = parseUnary();
        return unary;
    } else if (check(TokenType::MINUS)) {
        match(TokenType::MINUS);
        auto unary = std::make_unique<UnaryExpression>();
        unary->op = TokenType::MINUS;
        unary->expr = parseUnary();
        return unary;
    }

    return parsePrimary();
}

std::unique_ptr<Expression> Parser::parsePrimary() {
    if (check(TokenType::TRUE)) {
        match(TokenType::TRUE);
        auto literal = std::make_unique<Literal>();
        literal->value = TokenValue(true);
        return literal;
    } else if (check(TokenType::FALSE)) {
        match(TokenType::FALSE);
        auto literal = std::make_unique<Literal>();
        literal->value = TokenValue(false);
        return literal;
    } else if (check(TokenType::NULL_TOKEN)) {
        match(TokenType::NULL_TOKEN);
        auto literal = std::make_unique<Literal>();
        literal->value = TokenValue();
        return literal;
    }

    if (check(TokenType::NUMBER) || check(TokenType::STRING)) {
        auto literal = std::make_unique<Literal>();
        literal->value = currentToken_.value;
        advance();
        return literal;
    }

    if (check(TokenType::IDENTIFIER)) {
        auto identifier = std::make_unique<Identifier>();
        identifier->name = std::get<std::string>(currentToken_.value);
        advance();

        if (match(TokenType::LPAREN)) {
            auto call = std::make_unique<CallExpression>();
            call->callee = std::move(identifier);

            if (!check(TokenType::RPAREN)) {
                call->arguments = parseExpressionList();
            }

            consume(TokenType::RPAREN, "Expected ')' after arguments");
            return call;
        }

        if (match(TokenType::LBRACKET)) {
            return parseArrayAccess(std::move(identifier));
        }

        if (match(TokenType::DOT)) {
            return parsePropertyAccess(std::move(identifier));
        }

        return identifier;
    }

    if (check(TokenType::LPAREN)) {
        match(TokenType::LPAREN);
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }

    if (match(TokenType::LBRACKET)) {
        return parseArrayExpression();
    }

    throw std::runtime_error("Expected expression");
}

std::vector<std::unique_ptr<Expression>> Parser::parseExpressionList() {
    std::vector<std::unique_ptr<Expression>> args;

    do {
        args.push_back(parseExpression());
    } while (match(TokenType::COMMA));

    return args;
}

std::vector<std::string> Parser::parseParameters() {
    std::vector<std::string> params;

    if (!check(TokenType::RPAREN)) {
        do {
            if (!check(TokenType::IDENTIFIER)) {
                throw std::runtime_error("Expected parameter name");
            }

            params.push_back(std::get<std::string>(currentToken_.value));
            advance();
        } while (match(TokenType::COMMA));
    }

    return params;
}

std::unique_ptr<Expression> Parser::parseArrayExpression() {
    auto array = std::make_unique<ArrayExpression>();

    if (match(TokenType::RBRACKET)) {
        return array;
    }

    do {
        array->elements.push_back(parseExpression());
    } while (match(TokenType::COMMA));

    consume(TokenType::RBRACKET, "Expected ']' after array elements");
    return array;
}

std::unique_ptr<Expression> Parser::parseArrayAccess(std::unique_ptr<Expression> array) {
    auto arrayAccess = std::make_unique<ArrayAccessExpression>();
    arrayAccess->array = std::move(array);
    arrayAccess->index = parseExpression();
    consume(TokenType::RBRACKET, "Expected ']' after array index");

    if (match(TokenType::LBRACKET)) {
        return parseArrayAccess(std::move(arrayAccess));
    }

    if (match(TokenType::DOT)) {
        return parsePropertyAccess(std::move(arrayAccess));
    }

    return arrayAccess;
}

std::unique_ptr<Expression> Parser::parsePropertyAccess(std::unique_ptr<Expression> object) {
    auto propertyAccess = std::make_unique<PropertyAccessExpression>();
    propertyAccess->object = std::move(object);

    if (!check(TokenType::IDENTIFIER)) {
        throw std::runtime_error("Expected property name after '.'.");
    }

    propertyAccess->property = std::get<std::string>(currentToken_.value);
    advance();

    if (match(TokenType::DOT)) {
        return parsePropertyAccess(std::move(propertyAccess));
    }

    if (match(TokenType::LBRACKET)) {
        return parseArrayAccess(std::move(propertyAccess));
    }

    return propertyAccess;
}