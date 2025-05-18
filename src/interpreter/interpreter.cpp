#include "interpreter.hpp"
#include <cmath>


double Value::asNumber() const {
    try {
        if (isDouble()) {
            return number_;
        } else if (isInteger()) {
            return static_cast<double>(integer_);
        } else if (isString()) {
            try {
                size_t pos;
                int intValue = std::stoi(string_, &pos);
                if (pos == string_.size()) {
                    return static_cast<double>(intValue);
                }
                return std::stod(string_);
            } catch (const std::exception&) {
                return 0.0;
            }
        } else if (isBoolean()) {
            return boolean_ ? 1.0 : 0.0;
        } else if (isNull()) {
            return 0.0;
        } else if (isArray()) {
            return static_cast<double>(array_.size());
        }
        return 0.0;
    } catch (...) {
        return 0.0;
    }
}

int Value::asInteger() const {
    try {
        if (isInteger()) {
            return integer_;
        } else if (isDouble()) {
            if (number_ == static_cast<int>(number_)) {
                return static_cast<int>(number_);
            }
            return static_cast<int>(number_);
        } else if (isString()) {
            try {
                return std::stoi(string_);
            } catch (const std::exception&) {
                return 0;
            }
        } else if (isBoolean()) {
            return boolean_ ? 1 : 0;
        } else if (isNull()) {
            return 0;
        } else if (isArray()) {
            return static_cast<int>(array_.size());
        }
        return 0;
    } catch (...) {
        return 0;
    }
}

std::string Value::asString() const {
    try {
        if (isString()) {
            return string_;
        } else if (isDouble()) {
            return std::to_string(number_);
        } else if (isInteger()) {
            return std::to_string(integer_);
        } else if (isBoolean()) {
            return boolean_ ? "true" : "false";
        } else if (isNull()) {
            return "null";
        } else if (isArray()) {
            return "[array]";
        } else if (isAnyFunction()) {
            return "<function " + functionName_ + ">";
        }
        return "";
    } catch (...) {
        return "";
    }
}

bool Value::asBoolean() const {
    try {
        if (isBoolean()) {
            return boolean_;
        } else if (isInteger()) {
            return integer_ != 0;
        } else if (isDouble()) {
            return number_ != 0.0;
        } else if (isString()) {
            return !string_.empty();
        } else if (isNull()) {
            return false;
        } else if (isArray()) {
            return !array_.empty();
        }
        return true;
    } catch (...) {
        return false;
    }
}

std::vector<Value> Value::asArray() const {
    if (isArray()) {
        return std::vector<Value>(array_);
    }
    std::vector<Value> singleElementArray;
    singleElementArray.push_back(*this);
    return singleElementArray;
}

Value Value::getArrayElement(int index) const {
    if (isArray()) {
        if (index < 0 || index >= static_cast<int>(array_.size())) {
            return Value();
        }
        return array_[index];
    } else if (isString()) {
        if (index == 0) {
            return *this;
        } else if (index > 0 && index < static_cast<int>(string_.size())) {
            return Value(std::string(1, string_[index]));
        }
        return Value();
    } else {
        if (index == 0) {
            return *this;
        }
        return Value();
    }
}

void Value::setArrayElement(int index, const Value& value) {
    try {
        if (isString()) {
            type_ = Type::ARRAY;
            array_.clear();
            array_.push_back(Value(string_));
        } else if (!isArray()) {
            type_ = Type::ARRAY;
            array_.clear();
        }

        if (index < 0) {
            return;
        }

        if (index >= static_cast<int>(array_.size())) {
            if (index > 1000) {
                return;
            }
            array_.resize(index + 1);
        }

        array_[index] = value;
    } catch (...) {
    }
}

int Value::getArraySize() const {
    if (!isArray()) {
        return 1;
    }
    return static_cast<int>(array_.size());
}

void Value::setFunction(const std::string& name, const std::vector<std::string>& params,
                        std::shared_ptr<BlockStatement> body) {
    type_ = Type::FUNCTION;
    functionName_ = name;
    parameters_ = params;
    body_ = body;
}

void Value::setNativeFunction(std::function<Value(Interpreter&, const std::vector<Value>&)> function) {
    type_ = Type::NATIVE_FUNCTION;
    nativeFunction_ = function;
}

Value Value::call(Interpreter& interpreter, const std::vector<Value>& arguments) {
    if (type_ == Type::NATIVE_FUNCTION) {
        return nativeFunction_(interpreter, arguments);
    }

    if (!isFunction()) {
        throw RuntimeError("Can only call functions");
    }

    if (arguments.size() != parameters_.size()) {
        throw RuntimeError("Expected " + std::to_string(parameters_.size()) +
                          " arguments but got " + std::to_string(arguments.size()));
    }

    auto environment = std::make_shared<Environment>(interpreter.getEnvironment());

    for (size_t i = 0; i < parameters_.size(); i++) {
        environment->define(parameters_[i], arguments[i]);
    }

    auto previousEnv = interpreter.getEnvironment();
    interpreter.setEnvironment(environment);

    try {
        interpreter.executeBlock(body_.get(), environment);

        interpreter.setEnvironment(previousEnv);

        return Value();
    } catch (Return& returnValue) {
        interpreter.setEnvironment(previousEnv);

        return returnValue.value();
    }
}

Value Value::operator+(const Value& other) const {
    if (isInteger() && other.isInteger()) {
        return Value(integer_ + other.integer_);
    }

    if (isNumber() && other.isNumber()) {
        double result = asNumber() + other.asNumber();
        if (result == static_cast<int>(result)) {
            return Value(static_cast<int>(result));
        }
        return Value(result);
    }

    if (isString() || other.isString()) {
        return Value(toString() + other.toString());
    }

    if (isArray() && other.isArray()) {
        std::vector<Value> result(array_);
        const std::vector<Value>& otherArray = other.array_;
        result.insert(result.end(), otherArray.begin(), otherArray.end());
        return Value(result);
    }

    if (isNumber() || other.isNumber()) {
        double result = asNumber() + other.asNumber();
        if (result == static_cast<int>(result)) {
            return Value(static_cast<int>(result));
        }
        return Value(result);
    }

    return Value(toString() + other.toString());
}

Value Value::operator-(const Value& other) const {
    if (isInteger() && other.isInteger()) {
        return Value(integer_ - other.integer_);
    }

    double result = asNumber() - other.asNumber();
    if (result == static_cast<int>(result)) {
        return Value(static_cast<int>(result));
    }
    return Value(result);
}

Value Value::operator*(const Value& other) const {
    if (isInteger() && other.isInteger()) {
        return Value(integer_ * other.integer_);
    }

    double result = asNumber() * other.asNumber();
    if (result == static_cast<int>(result)) {
        return Value(static_cast<int>(result));
    }
    return Value(result);
}

Value Value::operator/(const Value& other) const {
    double divisor = other.asNumber();
    if (divisor == 0) {
        return Value(0);
    }

    if (isInteger() && other.isInteger() && integer_ % other.integer_ == 0) {
        return Value(integer_ / other.integer_);
    }

    double result = asNumber() / divisor;
    if (result == static_cast<int>(result)) {
        return Value(static_cast<int>(result));
    }
    return Value(result);
}

Value Value::operator%(const Value& other) const {
    double divisor = other.asNumber();
    if (divisor == 0) {
        return Value(0);
    }

    if (isInteger() && other.isInteger()) {
        return Value(integer_ % other.integer_);
    }

    double result = fmod(asNumber(), divisor);
    if (result == static_cast<int>(result)) {
        return Value(static_cast<int>(result));
    }
    return Value(result);
}

bool Value::operator==(const Value& other) const {
    if (isNull() && other.isNull()) return true;

    if (isNumber() && other.isNumber()) {
        double num1 = asNumber();
        double num2 = other.asNumber();
        return num1 == num2;
    }

    if (isString() && other.isString()) {
        return string_ == other.string_;
    }

    if (isBoolean() && other.isBoolean()) {
        return boolean_ == other.boolean_;
    }

    if ((isNumber() || other.isNumber()) && (isString() || other.isString())) {
        try {
            double num1 = asNumber();
            double num2 = other.asNumber();
            return num1 == num2;
        } catch (...) {
            return false;
        }
    }

    if (isBoolean() || other.isBoolean()) {
        return asBoolean() == other.asBoolean();
    }

    if (isArray() || other.isArray()) {
        return toString() == other.toString();
    }

    return false;
}

bool Value::operator!=(const Value& other) const {
    return !(*this == other);
}

bool Value::operator<(const Value& other) const {

    if (isNumber() && other.isNumber()) {
        double num1 = asNumber();
        double num2 = other.asNumber();
        return num1 < num2;
    }

    if (isString() && other.isString()) {
        return string_ < other.string_;
    }

    if ((isNumber() || other.isNumber()) && (isString() || other.isString())) {
        try {
            double num1 = asNumber();
            double num2 = other.asNumber();
            return num1 < num2;
        } catch (...) {
            return toString() < other.toString();
        }
    }

    if (isBoolean() || other.isBoolean()) {
        double num1 = asNumber();
        double num2 = other.asNumber();
        return num1 < num2;
    }

    if (isArray() && other.isArray()) {
        return array_.size() < other.array_.size();
    }

    return toString() < other.toString();
}

bool Value::operator<=(const Value& other) const {
    if (isNumber() && other.isNumber()) {
        double num1 = asNumber();
        double num2 = other.asNumber();
        return num1 <= num2;
    }

    return (*this < other) || (*this == other);
}

bool Value::operator>(const Value& other) const {
    if (isNumber() && other.isNumber()) {
        double num1 = asNumber();
        double num2 = other.asNumber();
        return num1 > num2;
    }

    return !(*this <= other);
}

bool Value::operator>=(const Value& other) const {
    if (isNumber() && other.isNumber()) {
        double num1 = asNumber();
        double num2 = other.asNumber();
        return num1 >= num2;
    }

    return !(*this < other);
}

std::string Value::toString() const {
    try {
        switch (type_) {
            case Type::NULL_VALUE: return "null";
            case Type::NUMBER: return std::to_string(number_);
            case Type::INTEGER: return std::to_string(integer_);
            case Type::STRING: return string_;
            case Type::BOOLEAN: return boolean_ ? "true" : "false";
            case Type::ARRAY: {
                std::string result = "[";
                for (size_t i = 0; i < array_.size(); ++i) {
                    if (i > 0) result += ", ";
                    if (array_[i].isArray()) {
                        result += array_[i].toString();
                    } else if (array_[i].isInteger()) {
                        result += std::to_string(array_[i].integer_);
                    } else if (array_[i].isDouble()) {
                        result += std::to_string(array_[i].number_);
                    } else if (array_[i].isString()) {
                        result += array_[i].string_;
                    } else if (array_[i].isBoolean()) {
                        result += array_[i].boolean_ ? "true" : "false";
                    } else if (array_[i].isNull()) {
                        result += "null";
                    } else if (array_[i].isAnyFunction()) {
                        result += "<function>";
                    } else {
                        result += "unknown";
                    }
                }
                result += "]";
                return result;
            }
            case Type::FUNCTION: return "<function " + functionName_ + ">";
            case Type::NATIVE_FUNCTION: return "<native function>";
            default: return "unknown";
        }
    } catch (...) {
        return "<error>";
    }
}

void Environment::define(const std::string& name, const Value& value) {
    values_[name] = value;
}

Value Environment::get(const std::string& name) {
    auto it = values_.find(name);
    if (it != values_.end()) {
        return it->second;
    }

    if (enclosing_ != nullptr) {
        return enclosing_->get(name);
    }

    throw RuntimeError("Undefined variable '" + name + "'");
}

void Environment::assign(const std::string& name, const Value& value) {
    auto it = values_.find(name);
    if (it != values_.end()) {
        it->second = value;
        return;
    }

    if (enclosing_ != nullptr) {
        enclosing_->assign(name, value);
        return;
    }

    throw RuntimeError("Undefined variable '" + name + "'");
}

Interpreter::Interpreter() {
    globals_ = std::make_shared<Environment>();
    environment_ = globals_;

    defineNativeFunctions();
}

void Interpreter::defineNativeFunctions() {
}

void Interpreter::interpret(std::unique_ptr<BlockStatement> program) {
    try {
        executeBlock(program.get(), environment_);
    } catch (RuntimeError& error) {
        std::cerr << "Runtime Error: " << error.what() << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
    }
}

void Interpreter::executeBlock(const BlockStatement* statement, std::shared_ptr<Environment> environment) {
    auto previousEnv = environment_;

    try {
        environment_ = environment;

        for (const auto& stmt : statement->statements) {
            switch (stmt->getType()) {
                case ASTNode::Type::BLOCK:
                    executeBlock(static_cast<const BlockStatement*>(stmt.get()),
                                std::make_shared<Environment>(environment_));
                    break;
                case ASTNode::Type::VARIABLE_DECLARATION:
                    executeVariableDeclaration(static_cast<const VariableDeclaration*>(stmt.get()));
                    break;
                case ASTNode::Type::FUNCTION_DECLARATION:
                    executeFunctionDeclaration(static_cast<const FunctionDeclaration*>(stmt.get()));
                    break;
                case ASTNode::Type::LOOP:
                    executeLoopStatement(static_cast<const LoopStatement*>(stmt.get()));
                    break;
                case ASTNode::Type::IF:
                    executeIfStatement(static_cast<const IfStatement*>(stmt.get()));
                    break;
                case ASTNode::Type::PRINT:
                    executePrintStatement(static_cast<const PrintStatement*>(stmt.get()));
                    break;
                case ASTNode::Type::RETURN:
                    executeReturnStatement(static_cast<const ReturnStatement*>(stmt.get()));
                    break;
                case ASTNode::Type::BREAK:
                    executeBreakStatement(static_cast<const BreakStatement*>(stmt.get()));
                    break;
                case ASTNode::Type::EXPRESSION:
                    executeExpressionStatement(static_cast<const ExpressionStatement*>(stmt.get()));
                    break;
                default:
                    throw RuntimeError("Unknown statement type");
            }
        }
    } catch (Return& returnValue) {
        environment_ = previousEnv;
        throw;
    } catch (Break& breakException) {
        environment_ = previousEnv;
        throw;
    } catch (std::exception& e) {
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        environment_ = previousEnv;
        throw;
    } catch (...) {
        std::cerr << "Unknown exception in executeBlock" << std::endl;
        environment_ = previousEnv;
        throw;
    }

    environment_ = previousEnv;
}

void Interpreter::executeVariableDeclaration(const VariableDeclaration* statement) {
    Value value;

    if (statement->initializer) {
        value = evaluateExpression(statement->initializer.get());
    }

    environment_->define(statement->identifier, value);
}

void Interpreter::executeFunctionDeclaration(const FunctionDeclaration* statement) {
    Value function;
    function.setFunction(statement->name, statement->parameters,
                        std::shared_ptr<BlockStatement>(statement->body->clone()));

    environment_->define(statement->name, function);
}

void Interpreter::executeLoopStatement(const LoopStatement* statement) {
    auto loopEnv = std::make_shared<Environment>(environment_);
    auto previousEnv = environment_;
    environment_ = loopEnv;

    if (statement->init) {
        switch (statement->init->getType()) {
            case ASTNode::Type::VARIABLE_DECLARATION:
                executeVariableDeclaration(static_cast<const VariableDeclaration*>(statement->init.get()));
                break;
            case ASTNode::Type::EXPRESSION:
                executeExpressionStatement(static_cast<const ExpressionStatement*>(statement->init.get()));
                break;
            default:
                throw RuntimeError("Invalid loop initializer");
        }
    }

    try {
        while (true) {
            if (statement->condition) {
                Value conditionValue = evaluateExpression(statement->condition.get());
                if (!isTruthy(conditionValue)) {
                    break;
                }
            }

            try {
                executeBlock(statement->body.get(), environment_);
            } catch (Break&) {
                break;
            }

            if (statement->increment) {
                evaluateExpression(statement->increment.get());
            }
        }
    } catch (Return& returnValue) {
        environment_ = previousEnv;
        throw;
    }

    environment_ = previousEnv;
}

void Interpreter::executeIfStatement(const IfStatement* statement) {
    Value conditionValue = evaluateExpression(statement->condition.get());
    bool result = isTruthy(conditionValue);

    if (result) {
        executeBlock(statement->thenBranch.get(), std::make_shared<Environment>(environment_));
    } else if (statement->elseBranch) {
        executeBlock(statement->elseBranch.get(), std::make_shared<Environment>(environment_));
    }
}

void Interpreter::executePrintStatement(const PrintStatement* statement) {
    if (!statement->directString.empty()) {
        std::cout << statement->directString;
        if (statement->isPrintln) {
            std::cout << std::endl;
        }
        return;
    }

    for (size_t i = 0; i < statement->args.size(); i++) {
        if (i > 0) std::cout << " ";
        Value value = evaluateExpression(statement->args[i].get());
        std::cout << value.toString() << std::flush;
    }

    if (statement->isPrintln) {
        std::cout << std::endl << std::flush;
    } else {
        std::cout << std::flush;
    }
}

void Interpreter::executeReturnStatement(const ReturnStatement* statement) {
    Value value;

    if (statement->value) {
        value = evaluateExpression(statement->value.get());
    }

    throw Return(value);
}

void Interpreter::executeBreakStatement(const BreakStatement* statement) {
    throw Break();
}

void Interpreter::executeExpressionStatement(const ExpressionStatement* statement) {
    evaluateExpression(statement->expr.get());
}

Value Value::getProperty(const std::string& name) const {
    if (name == "length") {
        if (isArray()) {
            return Value(static_cast<int>(array_.size()));
        } else if (isString()) {
            return Value(static_cast<int>(string_.size()));
        } else {
            return Value(1);
        }
    }

    return Value();
}

Value Interpreter::evaluateExpression(const Expression* expression) {
    switch (expression->getType()) {
        case ASTNode::Type::BINARY:
            return evaluateBinaryExpression(static_cast<const BinaryExpression*>(expression));
        case ASTNode::Type::UNARY:
            return evaluateUnaryExpression(static_cast<const UnaryExpression*>(expression));
        case ASTNode::Type::IDENTIFIER:
            return evaluateIdentifier(static_cast<const Identifier*>(expression));
        case ASTNode::Type::LITERAL:
            return evaluateLiteral(static_cast<const Literal*>(expression));
        case ASTNode::Type::CALL:
            return evaluateCallExpression(static_cast<const CallExpression*>(expression));
        case ASTNode::Type::ARRAY:
            return evaluateArrayExpression(static_cast<const ArrayExpression*>(expression));
        case ASTNode::Type::ARRAY_ACCESS:
            return evaluateArrayAccessExpression(static_cast<const ArrayAccessExpression*>(expression));
        case ASTNode::Type::PROPERTY_ACCESS:
            return evaluatePropertyAccessExpression(static_cast<const PropertyAccessExpression*>(expression));
        default:
            throw RuntimeError("Unknown expression type");
    }
}

Value Interpreter::evaluateBinaryExpression(const BinaryExpression* expression) {
    if (expression->op == TokenType::IDENTIFIER ||
        expression->op == TokenType::NUMBER ||
        expression->op == TokenType::STRING ||
        expression->op == TokenType::EOF_TOKEN ||
        expression->op == TokenType::ERROR) {
        return evaluateExpression(expression->left.get());
    }

    Value left = evaluateExpression(expression->left.get());
    Value right = evaluateExpression(expression->right.get());

    switch (expression->op) {
        case TokenType::PLUS:
            return left + right;
        case TokenType::MINUS:
            return left - right;
        case TokenType::MULTIPLY:
            return left * right;
        case TokenType::DIVIDE:
            return left / right;
        case TokenType::MODULO:
            return left % right;
        case TokenType::EQUALS:
            return Value(left == right);
        case TokenType::NOT_EQUALS:
            return Value(left != right);
        case TokenType::LESS: {
            return Value(left < right);
        }
        case TokenType::LESS_EQ: {
            return Value(left <= right);
        }
        case TokenType::GREATER: {
            return Value(left > right);
        }
        case TokenType::GREATER_EQ: {
            return Value(left >= right);
        }
        case TokenType::AND: {
            bool leftTruthy = isTruthy(left);
            if (!leftTruthy) {
                return Value(false);
            }
            bool rightTruthy = isTruthy(right);
            return Value(rightTruthy);
        }
        case TokenType::OR: {
            bool leftTruthyOr = isTruthy(left);
            if (leftTruthyOr) {
                return Value(true);
            }
            bool rightTruthyOr = isTruthy(right);
            return Value(rightTruthyOr);
        }
        case TokenType::ASSIGN: {
            if (expression->left->getType() == ASTNode::Type::ARRAY_ACCESS) {
                const ArrayAccessExpression* arrayAccess = static_cast<const ArrayAccessExpression*>(expression->left.get());

                std::string arrayName;
                if (arrayAccess->array->getType() == ASTNode::Type::IDENTIFIER) {
                    const Identifier* arrayIdentifier = static_cast<const Identifier*>(arrayAccess->array.get());
                    arrayName = arrayIdentifier->name;
                } else {
                    throw RuntimeError("Cannot assign to an element of a non-variable array");
                }

                Value array = environment_->get(arrayName);
                Value indexValue = evaluateExpression(arrayAccess->index.get());
                Value rightValue = right;
                int index = static_cast<int>(indexValue.asNumber());
                array.setArrayElement(index, rightValue);
                environment_->assign(arrayName, array);

                return rightValue;
            }
            else if (expression->left->getType() != ASTNode::Type::IDENTIFIER) {
                throw RuntimeError("Invalid assignment target");
            }

            const Identifier* identifier = static_cast<const Identifier*>(expression->left.get());

            Value rightValue = right;

            if (expression->right->getType() == ASTNode::Type::BINARY) {
                const BinaryExpression* binExpr = static_cast<const BinaryExpression*>(expression->right.get());
                if (binExpr->op == TokenType::PLUS) {
                    Value leftVal = evaluateExpression(binExpr->left.get());
                    Value rightVal = evaluateExpression(binExpr->right.get());

                    if (leftVal.isNumber() && rightVal.isNumber()) {
                        double result = leftVal.asNumber() + rightVal.asNumber();
                        rightValue = Value(result);
                    }
                }
            }

            environment_->assign(identifier->name, rightValue);
            return rightValue;
        }
        default:
            throw RuntimeError("Unknown binary operator: " + tokenTypeToString(expression->op));
    }
}

Value Interpreter::evaluateUnaryExpression(const UnaryExpression* expression) {
    Value operand = evaluateExpression(expression->expr.get());

    switch (expression->op) {
        case TokenType::MINUS:
            return Value(-operand.asNumber());
        case TokenType::BANG: {
            bool result = !isTruthy(operand);
            return Value(result);
        }
        default:
            throw RuntimeError("Unknown unary operator: " + tokenTypeToString(expression->op));
    }
}

Value Interpreter::evaluateIdentifier(const Identifier* expression) {
    Value value = environment_->get(expression->name);
    return value;
}

Value Interpreter::evaluateLiteral(const Literal* expression) {
    const TokenValue& value = expression->value;

    if (std::holds_alternative<double>(value)) {
        double num = std::get<double>(value);
        if (num == static_cast<int>(num)) {
            return Value(static_cast<int>(num));
        }
        return Value(num);
    } else if (std::holds_alternative<std::string>(value)) {
        std::string str = std::get<std::string>(value);
        return Value(str);
    } else if (std::holds_alternative<bool>(value)) {
        bool b = std::get<bool>(value);
        return Value(b);
    } else {
        return Value();
    }
}

Value Interpreter::evaluateCallExpression(const CallExpression* expression) {
    if (expression->callee->getType() == ASTNode::Type::IDENTIFIER) {
        const Identifier* identifier = static_cast<const Identifier*>(expression->callee.get());
        Value callee = environment_->get(identifier->name);

        std::vector<Value> arguments;
        for (const auto& arg : expression->arguments) {
            arguments.push_back(evaluateExpression(arg.get()));
        }

        return callee.call(*this, arguments);
    } else {
        Value callee = evaluateExpression(expression->callee.get());

        std::vector<Value> arguments;
        for (const auto& arg : expression->arguments) {
            arguments.push_back(evaluateExpression(arg.get()));
        }

        return callee.call(*this, arguments);
    }
}

Value Interpreter::evaluateArrayExpression(const ArrayExpression* expression) {
    std::vector<Value> elements;
    for (const auto& element : expression->elements) {
        elements.push_back(evaluateExpression(element.get()));
    }
    return Value(elements);
}

Value Interpreter::evaluateArrayAccessExpression(const ArrayAccessExpression* expression) {
    Value array = evaluateExpression(expression->array.get());
    Value indexValue = evaluateExpression(expression->index.get());

    int index = static_cast<int>(indexValue.asNumber());
    return array.getArrayElement(index);
}

Value Interpreter::evaluatePropertyAccessExpression(const PropertyAccessExpression* expression) {
    Value object = evaluateExpression(expression->object.get());
    return object.getProperty(expression->property);
}

bool Interpreter::isTruthy(const Value& value) {
    return value.asBoolean();
}
