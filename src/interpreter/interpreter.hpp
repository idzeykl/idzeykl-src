#ifndef INTERPRETER_HPP
#define INTERPRETER_HPP

#include "../parser/parser.hpp"
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <stdexcept>
#include <iostream>

class Environment;
class Value;
class Interpreter;

class RuntimeError : public std::runtime_error {
public:
    RuntimeError(const std::string& message) : std::runtime_error(message) {}
};

class Value {
public:
    enum class Type {
        NULL_VALUE,
        NUMBER,
        INTEGER,
        STRING,
        BOOLEAN,
        ARRAY,
        FUNCTION,
        NATIVE_FUNCTION
    };

    Value() : type_(Type::NULL_VALUE), number_(0.0), integer_(0) {}
    Value(double number) : type_(Type::NUMBER), number_(number), integer_(0) {}
    Value(int integer) : type_(Type::INTEGER), number_(0.0), integer_(integer) {}
    Value(const std::string& str) : type_(Type::STRING), string_(str), integer_(0) {}
    Value(bool boolean) : type_(Type::BOOLEAN), boolean_(boolean), integer_(0) {}
    Value(const std::vector<Value>& array) : type_(Type::ARRAY), array_(array), integer_(0) {}

    bool isNull() const { return type_ == Type::NULL_VALUE; }
    bool isNumber() const { return type_ == Type::NUMBER || type_ == Type::INTEGER; }
    bool isInteger() const { return type_ == Type::INTEGER; }
    bool isDouble() const { return type_ == Type::NUMBER; }
    bool isString() const { return type_ == Type::STRING; }
    bool isBoolean() const { return type_ == Type::BOOLEAN; }
    bool isArray() const { return type_ == Type::ARRAY; }
    bool isFunction() const { return type_ == Type::FUNCTION; }
    bool isNativeFunction() const { return type_ == Type::NATIVE_FUNCTION; }
    bool isAnyFunction() const { return type_ == Type::FUNCTION || type_ == Type::NATIVE_FUNCTION; }

    double asNumber() const;
    int asInteger() const;
    std::string asString() const;
    bool asBoolean() const;
    std::vector<Value> asArray() const;
    Value getArrayElement(int index) const;
    void setArrayElement(int index, const Value& value);
    int getArraySize() const;
    Value getProperty(const std::string& name) const;

    void setFunction(const std::string& name, const std::vector<std::string>& params,
                     std::shared_ptr<BlockStatement> body);
    void setNativeFunction(std::function<Value(Interpreter&, const std::vector<Value>&)> function);
    Value call(Interpreter& interpreter, const std::vector<Value>& arguments);

    Value operator+(const Value& other) const;
    Value operator-(const Value& other) const;
    Value operator*(const Value& other) const;
    Value operator/(const Value& other) const;
    Value operator%(const Value& other) const;
    bool operator==(const Value& other) const;
    bool operator!=(const Value& other) const;
    bool operator<(const Value& other) const;
    bool operator<=(const Value& other) const;
    bool operator>(const Value& other) const;
    bool operator>=(const Value& other) const;

    std::string toString() const;

private:
    Type type_;
    double number_;
    int integer_;
    std::string string_;
    bool boolean_;
    std::vector<Value> array_;

    std::string functionName_;
    std::vector<std::string> parameters_;
    std::shared_ptr<BlockStatement> body_;
    std::function<Value(Interpreter&, const std::vector<Value>&)> nativeFunction_;
};

class Environment {
public:
    Environment() : enclosing_(nullptr) {}
    Environment(std::shared_ptr<Environment> enclosing) : enclosing_(enclosing) {}

    void define(const std::string& name, const Value& value);
    Value get(const std::string& name);
    void assign(const std::string& name, const Value& value);

    std::shared_ptr<Environment> getEnclosing() const { return enclosing_; }

    const std::unordered_map<std::string, Value>& getValues() const { return values_; }

private:
    std::unordered_map<std::string, Value> values_;
    std::shared_ptr<Environment> enclosing_;
};

class Return : public std::runtime_error {
public:
    Return(const Value& value) : std::runtime_error(""), value_(value) {}
    Value value() const { return value_; }

private:
    Value value_;
};

class Break : public std::runtime_error {
public:
    Break() : std::runtime_error("") {}
};

class Interpreter {
public:
    Interpreter();

    void interpret(std::unique_ptr<BlockStatement> program);

    void executeBlock(const BlockStatement* statement, std::shared_ptr<Environment> environment);
    void executeVariableDeclaration(const VariableDeclaration* statement);
    void executeFunctionDeclaration(const FunctionDeclaration* statement);
    void executeLoopStatement(const LoopStatement* statement);
    void executeIfStatement(const IfStatement* statement);
    void executePrintStatement(const PrintStatement* statement);
    void executeReturnStatement(const ReturnStatement* statement);
    void executeBreakStatement(const BreakStatement* statement);
    void executeExpressionStatement(const ExpressionStatement* statement);

    Value evaluateExpression(const Expression* expression);
    Value evaluateBinaryExpression(const BinaryExpression* expression);
    Value evaluateUnaryExpression(const UnaryExpression* expression);
    Value evaluateIdentifier(const Identifier* expression);
    Value evaluateLiteral(const Literal* expression);
    Value evaluateCallExpression(const CallExpression* expression);
    Value evaluateArrayExpression(const ArrayExpression* expression);
    Value evaluateArrayAccessExpression(const ArrayAccessExpression* expression);
    Value evaluatePropertyAccessExpression(const PropertyAccessExpression* expression);

    std::shared_ptr<Environment> getEnvironment() { return environment_; }
    void setEnvironment(std::shared_ptr<Environment> environment) { environment_ = environment; }

private:
    std::shared_ptr<Environment> environment_;
    std::shared_ptr<Environment> globals_;

    void defineNativeFunctions();
    bool isTruthy(const Value& value);
};

#endif // INTERPRETER_HPP
