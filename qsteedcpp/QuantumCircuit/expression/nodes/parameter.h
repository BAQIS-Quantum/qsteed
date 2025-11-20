#pragma once
#include "../expression.h"
#include "../uuid_generator.h"

namespace qsteedcpp {

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

    double eval() const override {
        return value_;
    }
};

} // namespace qsteedcpp
