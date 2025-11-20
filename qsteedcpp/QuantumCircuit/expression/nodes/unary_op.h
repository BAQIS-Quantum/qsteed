#pragma once
#include "../expression.h"
#include <cmath>
#include <stdexcept>

namespace qsteedcpp {

class UnaryOp : public Expression {
private:
    std::shared_ptr<Expression> operand_;
    UnaryOpType op_;

public:
    UnaryOp(std::shared_ptr<Expression> operand,
            UnaryOpType op)
        : operand_(std::move(operand)),
          op_(op) {}

    const Expression* get_operand() const { return operand_.get(); }
    UnaryOpType get_op() const { return op_; }

    Type get_type() const override { return Type::UNARY_OP; }

    std::set<std::string> get_parameter_uuids() const override {
        return operand_->get_parameter_uuids();
    }

    std::string to_string() const override {
        std::string op_str;
        switch (op_) {
            case UnaryOpType::NEG:  op_str = "-"; break;
            case UnaryOpType::SIN:  op_str = "sin"; break;
            case UnaryOpType::COS:  op_str = "cos"; break;
            case UnaryOpType::TAN:  op_str = "tan"; break;
            case UnaryOpType::EXP:  op_str = "exp"; break;
            case UnaryOpType::LOG:  op_str = "log"; break;
            case UnaryOpType::SQRT: op_str = "sqrt"; break;
        }
        if (op_ == UnaryOpType::NEG) {
            return "-" + operand_->to_string();
        }
        return op_str + "(" + operand_->to_string() + ")";
    }

    std::shared_ptr<Expression> clone() const override {
        return std::make_shared<UnaryOp>(operand_->clone(), op_);
    }

    double eval() const override {
        double val = operand_->eval();

        switch (op_) {
            case UnaryOpType::NEG:  return -val;
            case UnaryOpType::SIN:  return std::sin(val);
            case UnaryOpType::COS:  return std::cos(val);
            case UnaryOpType::TAN:  return std::tan(val);
            case UnaryOpType::EXP:  return std::exp(val);
            case UnaryOpType::LOG:  return std::log(val);
            case UnaryOpType::SQRT: return std::sqrt(val);
        }
        throw std::runtime_error("Unknown unary operator");
    }
};

} // namespace qsteedcpp
