#pragma once
#include "expression.h"
#include "nodes/constant.h"
#include "nodes/parameter.h"
#include "nodes/binary_op.h"
#include "nodes/unary_op.h"

namespace qsteedcpp {

// Expr：值类型包装器（用户接口）
class Expr {
private:
    std::shared_ptr<Expression> expr_;

public:
    Expr(double value)
        : expr_(std::make_shared<Constant>(value)) {}

    Expr(const Parameter& param)
        : expr_(param.share()) {}  // Use share() to maintain parameter value synchronization

    explicit Expr(std::shared_ptr<Expression> expr)
        : expr_(std::move(expr)) {}

    Expr(const Expr&) = default;
    Expr(Expr&&) = default;
    Expr& operator=(const Expr&) = default;
    Expr& operator=(Expr&&) = default;

    const Expression* get() const { return expr_.get(); }
    std::shared_ptr<Expression> get_shared() const { return expr_; }

    // Deep copy: creates a completely independent copy including parameters
    Expr clone() const {
        return Expr(expr_->clone());
    }

    std::set<std::string> get_parameter_uuids() const {
        return expr_->get_parameter_uuids();
    }

    std::string to_string(bool numeric_params = false) const {
        return expr_->to_string(numeric_params);
    }

    // Evaluate the expression to a double value
    double eval() const {
        return expr_->eval();
    }

    // ========== 运算符重载（友元，返回 Expr 值类型）==========
    // 加法
    friend Expr operator+(const Expr& lhs, const Expr& rhs) {
        auto node = std::make_shared<BinaryOp>(lhs.expr_, rhs.expr_, BinaryOpType::ADD);
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
        auto node = std::make_shared<BinaryOp>(lhs.expr_, rhs.expr_, BinaryOpType::SUB);
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
        auto node = std::make_shared<BinaryOp>(lhs.expr_, rhs.expr_, BinaryOpType::MUL);
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
        auto node = std::make_shared<BinaryOp>(lhs.expr_, rhs.expr_, BinaryOpType::DIV);
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
        auto node = std::make_shared<UnaryOp>(expr_, UnaryOpType::NEG);
        return Expr(node);
    }

    // 数学函数
    friend Expr sin(const Expr& expr) {
        auto node = std::make_shared<UnaryOp>(expr.expr_, UnaryOpType::SIN);
        return Expr(node);
    }

    friend Expr cos(const Expr& expr) {
        auto node = std::make_shared<UnaryOp>(expr.expr_, UnaryOpType::COS);
        return Expr(node);
    }

    friend Expr tan(const Expr& expr) {
        auto node = std::make_shared<UnaryOp>(expr.expr_, UnaryOpType::TAN);
        return Expr(node);
    }

    friend Expr exp(const Expr& expr) {
        auto node = std::make_shared<UnaryOp>(expr.expr_, UnaryOpType::EXP);
        return Expr(node);
    }

    friend Expr log(const Expr& expr) {
        auto node = std::make_shared<UnaryOp>(expr.expr_, UnaryOpType::LOG);
        return Expr(node);
    }

    friend Expr sqrt(const Expr& expr) {
        auto node = std::make_shared<UnaryOp>(expr.expr_, UnaryOpType::SQRT);
        return Expr(node);
    }

    // pow 改为二元运算符（修复 hack）
    friend Expr pow(const Expr& base, const Expr& exponent) {
        auto node = std::make_shared<BinaryOp>(base.expr_, exponent.expr_, BinaryOpType::POW);
        return Expr(node);
    }

    friend Expr pow(const Expr& base, double exponent) {
        return pow(base, Expr(exponent));
    }

    friend Expr pow(double base, const Expr& exponent) {
        return pow(Expr(base), exponent);
    }
};

} // namespace qsteedcpp
