#pragma once
#include "../expression.h"

namespace qsteedcpp {

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

    std::string to_string(bool numeric_params = false) const override {
        return std::to_string(value_);
    }

    std::shared_ptr<Expression> clone() const override {
        return std::make_shared<Constant>(value_);
    }

    double eval() const override {
        return value_;
    }
};

} // namespace qsteedcpp
