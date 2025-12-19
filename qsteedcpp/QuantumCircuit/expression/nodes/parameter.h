#pragma once
#include "../expression.h"
#include "../uuid_generator.h"
#include <memory>

namespace qsteedcpp {

class Parameter : public Expression {
private:
    // Pimpl: Pointer to Implementation
    // All Parameter copies share the same Impl, so modifications are visible everywhere
    struct Impl {
        std::string uuid_;
        double value_;
        bool trainable_;

        Impl(double value, bool trainable)
            : uuid_(UUIDGenerator::generate()),
              value_(value),
              trainable_(trainable) {}
    };

    std::shared_ptr<Impl> impl_;

public:
    explicit Parameter(double value = 0.0, bool trainable = false)
        : impl_(std::make_shared<Impl>(value, trainable)) {}

    // Accessor methods now go through impl_
    std::string get_uuid() const { return impl_->uuid_; }
    std::string get_name() const { return "param_" + impl_->uuid_.substr(0, 8); }
    double get_value() const { return impl_->value_; }
    void set_value(double val) { impl_->value_ = val; }
    bool is_trainable() const { return impl_->trainable_; }
    bool is_constant() const { return !impl_->trainable_; }

    Type get_type() const override { return Type::PARAMETER; }

    std::set<std::string> get_parameter_uuids() const override {
        return {impl_->uuid_};
    }

    std::string to_string(bool numeric_params = false) const override {
        if (numeric_params) {
            return std::to_string(impl_->value_);
        }
        return get_name();
    }

    // share(): Create a new Parameter that shares the same internal state (pimpl pattern)
    // All modifications to one parameter will be visible to all shared copies
    std::shared_ptr<Expression> share() const {
        auto p = std::make_shared<Parameter>(*this);  // Copy constructor shares impl_
        return p;
    }

    // clone(): Create a true deep copy with independent state
    // The cloned parameter has a NEW UUID (separate instance) and same value
    // Name is derived from UUID, so cloned parameter will have different name
    std::shared_ptr<Expression> clone() const override {
        auto p = std::make_shared<Parameter>(impl_->value_, impl_->trainable_);
        // UUID is auto-generated in constructor (new unique ID)
        return p;
    }

    // 判断是否是同一个参数对象（基于 UUID）
    bool same(const Parameter& other) const {
        return impl_->uuid_ == other.impl_->uuid_;
    }

    // 比较操作符（基于值）
    bool operator==(const Parameter& other) const {
        return impl_->value_ == other.impl_->value_;
    }

    bool operator!=(const Parameter& other) const {
        return impl_->value_ != other.impl_->value_;
    }

    bool operator<(const Parameter& other) const {
        return impl_->value_ < other.impl_->value_;
    }

    bool operator<=(const Parameter& other) const {
        return impl_->value_ <= other.impl_->value_;
    }

    bool operator>(const Parameter& other) const {
        return impl_->value_ > other.impl_->value_;
    }

    bool operator>=(const Parameter& other) const {
        return impl_->value_ >= other.impl_->value_;
    }

    struct Hash {
        std::size_t operator()(const Parameter& param) const {
            return std::hash<std::string>{}(param.get_uuid());
        }
    };

    double eval() const override {
        return impl_->value_;
    }
};

} // namespace qsteedcpp
