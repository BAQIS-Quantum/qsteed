#pragma once
#include "../expression.h"
#include <cmath>
#include <stdexcept>

namespace qsteedcpp {

class BinaryOp : public Expression {
private:
    std::shared_ptr<Expression> left_;
    std::shared_ptr<Expression> right_;
    BinaryOpType op_;

public:
    BinaryOp(std::shared_ptr<Expression> left,
             std::shared_ptr<Expression> right,
             BinaryOpType op)
        : left_(std::move(left)),
          right_(std::move(right)),
          op_(op) {}

    const Expression* get_left() const { return left_.get(); }
    const Expression* get_right() const { return right_.get(); }
    BinaryOpType get_op() const { return op_; }

    Type get_type() const override { return Type::BINARY_OP; }

    std::set<std::string> get_parameter_uuids() const override {
        auto left_params = left_->get_parameter_uuids();
        auto right_params = right_->get_parameter_uuids();
        left_params.insert(right_params.begin(), right_params.end());
        return left_params;
    }

    std::string to_string(bool numeric_params = false) const override {
        std::string op_str;
        switch (op_) {
            case BinaryOpType::ADD: op_str = "+"; break;
            case BinaryOpType::SUB: op_str = "-"; break;
            case BinaryOpType::MUL: op_str = "*"; break;
            case BinaryOpType::DIV: op_str = "/"; break;
            case BinaryOpType::POW: op_str = "^"; break;
        }
        return "(" + left_->to_string(numeric_params) + " " + op_str + " " + right_->to_string(numeric_params) + ")";
    }

    std::shared_ptr<Expression> clone() const override {
        return std::make_shared<BinaryOp>(left_->clone(), right_->clone(), op_);
    }

    double eval() const override {
        double left_val = left_->eval();
        double right_val = right_->eval();

        switch (op_) {
            case BinaryOpType::ADD: return left_val + right_val;
            case BinaryOpType::SUB: return left_val - right_val;
            case BinaryOpType::MUL: return left_val * right_val;
            case BinaryOpType::DIV: return left_val / right_val;
            case BinaryOpType::POW: return std::pow(left_val, right_val);
        }
        throw std::runtime_error("Unknown binary operator");
    }
};

} // namespace qsteedcpp
