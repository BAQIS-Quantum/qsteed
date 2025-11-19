#pragma once
#include <string>
#include <memory>
#include <set>
#include <cmath>
#include "uuid_generator.h"

namespace qsteedcpp {

// ========== 符号计算系统 ==========
// 1. 内部：Expression 继承体系（Constant, Parameter, BinaryOp, UnaryOp）
// 2. 外部：Expr 值类型包装器（用户友好，隐藏指针）

class Expression;
class Parameter;
class Expr;

// ========== 1. Expression System ==========
class Expression {
public:
    virtual ~Expression() = default;

    // 节点类型
    enum class Type {
        CONSTANT,
        PARAMETER,
        BINARY_OP,
        UNARY_OP
    };

    virtual Type get_type() const = 0;
    virtual std::set<std::string> get_parameter_uuids() const = 0;
    virtual std::string to_string() const = 0;
    virtual std::shared_ptr<Expression> clone() const = 0;
};


class Constant : public Expression {
private:
    double value_;

public:
    explicit Constant(double value) : value_(value) {}

    double get_value() const { return value_; }

    Type get_type() const override { return Type::CONSTANT; }

    std::set<std::string> get_parameter_uuids() const override {
        return {};
    }

    std::string to_string() const override {
        return std::to_string(value_);
    }

    std::shared_ptr<Expression> clone() const override {
        return std::make_shared<Constant>(value_);
    }
};


class Parameter : public Expression {
private:
    std::string uuid_;
    double value_;
    bool trainable_;

public:
    explicit Parameter(double value = 0.0, bool trainable = false)
        : uuid_(UUIDGenerator::generate()),
          value_(value),
          trainable_(trainable) {}

    std::string get_uuid() const { return uuid_; }
    double get_value() const { return value_; }
    void set_value(double val) { value_ = val; }
    bool is_trainable() const { return trainable_; }
    bool is_constant() const { return !trainable_; }

    Type get_type() const override { return Type::PARAMETER; }

    std::set<std::string> get_parameter_uuids() const override {
        return {uuid_};
    }

    std::string to_string() const override {
        return "param_" + uuid_.substr(0, 8) + "=" + std::to_string(value_);
    }

    std::shared_ptr<Expression> clone() const override {
        auto p = std::make_shared<Parameter>(value_, trainable_);
        const_cast<Parameter*>(p.get())->uuid_ = uuid_;
        return p;
    }

    // 判断是否是同一个参数对象（基于 UUID）
    bool same(const Parameter& other) const {
        return uuid_ == other.uuid_;
    }

    // 比较操作符（基于值）
    bool operator==(const Parameter& other) const {
        return value_ == other.value_;
    }

    bool operator!=(const Parameter& other) const {
        return value_ != other.value_;
    }

    bool operator<(const Parameter& other) const {
        return value_ < other.value_;
    }

    bool operator<=(const Parameter& other) const {
        return value_ <= other.value_;
    }

    bool operator>(const Parameter& other) const {
        return value_ > other.value_;
    }

    bool operator>=(const Parameter& other) const {
        return value_ >= other.value_;
    }

    struct Hash {
        std::size_t operator()(const Parameter& param) const {
            return std::hash<std::string>{}(param.get_uuid());
        }
    };
};


class BinaryOp : public Expression {
private:
    std::shared_ptr<Expression> left_;
    std::shared_ptr<Expression> right_;
    std::string op_;

public:
    BinaryOp(std::shared_ptr<Expression> left,
             std::shared_ptr<Expression> right,
             const std::string& op)
        : left_(std::move(left)),
          right_(std::move(right)),
          op_(op) {}

    const Expression* get_left() const { return left_.get(); }
    const Expression* get_right() const { return right_.get(); }
    std::string get_op() const { return op_; }

    Type get_type() const override { return Type::BINARY_OP; }

    std::set<std::string> get_parameter_uuids() const override {
        auto left_params = left_->get_parameter_uuids();
        auto right_params = right_->get_parameter_uuids();
        left_params.insert(right_params.begin(), right_params.end());
        return left_params;
    }

    std::string to_string() const override {
        return "(" + left_->to_string() + " " + op_ + " " + right_->to_string() + ")";
    }

    std::shared_ptr<Expression> clone() const override {
        return std::make_shared<BinaryOp>(left_->clone(), right_->clone(), op_);
    }
};


class UnaryOp : public Expression {
private:
    std::shared_ptr<Expression> operand_;
    std::string op_;

public:
    UnaryOp(std::shared_ptr<Expression> operand,
            const std::string& op)
        : operand_(std::move(operand)),
          op_(op) {}

    const Expression* get_operand() const { return operand_.get(); }
    std::string get_op() const { return op_; }

    Type get_type() const override { return Type::UNARY_OP; }

    std::set<std::string> get_parameter_uuids() const override {
        return operand_->get_parameter_uuids();
    }

    std::string to_string() const override {
        return op_ + "(" + operand_->to_string() + ")";
    }

    std::shared_ptr<Expression> clone() const override {
        return std::make_shared<UnaryOp>(operand_->clone(), op_);
    }
};

// ========== 3. Expr：值类型包装器（用户接口）==========

class Expr {
private:
    std::shared_ptr<Expression> expr_;

public:
    Expr(double value)
        : expr_(std::make_shared<Constant>(value)) {}

    Expr(const Parameter& param)
        : expr_(param.clone()) {}

    // 从 Expression 指针构造（内部使用）
    explicit Expr(std::shared_ptr<Expression> expr)
        : expr_(std::move(expr)) {}

    // 拷贝/移动（默认，shared_ptr 自动管理）
    Expr(const Expr&) = default;
    Expr(Expr&&) = default;
    Expr& operator=(const Expr&) = default;
    Expr& operator=(Expr&&) = default;

    // 访问内部 Expression（供后端使用）
    const Expression* get() const { return expr_.get(); }
    std::shared_ptr<Expression> get_shared() const { return expr_; }

    // 获取表达式中所有参数的 UUID
    std::set<std::string> get_parameter_uuids() const {
        return expr_->get_parameter_uuids();
    }

    std::string to_string() const {
        return expr_->to_string();
    }

    // ========== 运算符重载（友元，返回 Expr 值类型）==========
    // 加法
    friend Expr operator+(const Expr& lhs, const Expr& rhs) {
        auto node = std::make_shared<BinaryOp>(lhs.expr_, rhs.expr_, "+");
        return Expr(node);
    }

    friend Expr operator+(const Expr& lhs, double rhs) {
        return lhs + Expr(rhs);
    }

    friend Expr operator+(double lhs, const Expr& rhs) {
        return Expr(lhs) + rhs;
    }

    // 减法
    friend Expr operator-(const Expr& lhs, const Expr& rhs) {
        auto node = std::make_shared<BinaryOp>(lhs.expr_, rhs.expr_, "-");
        return Expr(node);
    }

    friend Expr operator-(const Expr& lhs, double rhs) {
        return lhs - Expr(rhs);
    }

    friend Expr operator-(double lhs, const Expr& rhs) {
        return Expr(lhs) - rhs;
    }

    // 乘法
    friend Expr operator*(const Expr& lhs, const Expr& rhs) {
        auto node = std::make_shared<BinaryOp>(lhs.expr_, rhs.expr_, "*");
        return Expr(node);
    }

    friend Expr operator*(const Expr& lhs, double rhs) {
        return lhs * Expr(rhs);
    }

    friend Expr operator*(double lhs, const Expr& rhs) {
        return Expr(lhs) * rhs;
    }

    // 除法
    friend Expr operator/(const Expr& lhs, const Expr& rhs) {
        auto node = std::make_shared<BinaryOp>(lhs.expr_, rhs.expr_, "/");
        return Expr(node);
    }

    friend Expr operator/(const Expr& lhs, double rhs) {
        return lhs / Expr(rhs);
    }

    friend Expr operator/(double lhs, const Expr& rhs) {
        return Expr(lhs) / rhs;
    }

    // 负号
    Expr operator-() const {
        auto node = std::make_shared<UnaryOp>(expr_, "-");
        return Expr(node);
    }

    // 数学函数
    friend Expr sin(const Expr& expr) {
        auto node = std::make_shared<UnaryOp>(expr.expr_, "sin");
        return Expr(node);
    }

    friend Expr cos(const Expr& expr) {
        auto node = std::make_shared<UnaryOp>(expr.expr_, "cos");
        return Expr(node);
    }

    friend Expr tan(const Expr& expr) {
        auto node = std::make_shared<UnaryOp>(expr.expr_, "tan");
        return Expr(node);
    }

    friend Expr exp(const Expr& expr) {
        auto node = std::make_shared<UnaryOp>(expr.expr_, "exp");
        return Expr(node);
    }

    friend Expr log(const Expr& expr) {
        auto node = std::make_shared<UnaryOp>(expr.expr_, "log");
        return Expr(node);
    }

    friend Expr sqrt(const Expr& expr) {
        auto node = std::make_shared<UnaryOp>(expr.expr_, "sqrt");
        return Expr(node);
    }

    friend Expr pow(const Expr& expr, double exponent) {
        auto node = std::make_shared<UnaryOp>(
            expr.expr_,
            "pow(" + std::to_string(exponent) + ")"
        );
        return Expr(node);
    }
};


} // namespace qsteedcpp