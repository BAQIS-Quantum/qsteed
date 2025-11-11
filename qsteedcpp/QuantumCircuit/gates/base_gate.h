#pragma once

#include <string>
#include <memory>
#include <set>
#include "symbolic/symbolic.h"

namespace qsteedcpp {

class Gate {
private:
    int qubit_count_;
    std::vector<Expr> param_expressions_;

public:
    Gate(int qubit_count)
        : qubit_count_(qubit_count) {}

    Gate(int qubit_count, std::vector<Expr> exprs)
        : qubit_count_(qubit_count), param_expressions_(std::move(exprs)) {}

    virtual ~Gate() = default;

    virtual const char* name() const = 0;

    int get_qubit_count() const { return qubit_count_; }

    bool has_parameters() const { return !param_expressions_.empty(); }
    size_t parameter_count() const { return param_expressions_.size(); }

    const Expr& get_parameter_expression(size_t index) const {
        if (index >= param_expressions_.size()) {
            throw std::out_of_range("Parameter index out of range.");
        }
        return param_expressions_[index];
    }

    const std::vector<Expr>& get_parameter_expressions() const {
        return param_expressions_;
    }

    // 收集此门中所有参数的 UUID
    std::set<std::string> get_parameter_uuids() const {
        std::set<std::string> uuids;
        for (const auto& expr : param_expressions_) {
            auto expr_uuids = expr.get_parameter_uuids();
            uuids.insert(expr_uuids.begin(), expr_uuids.end());
        }
        return uuids;
    }

    virtual std::unique_ptr<Gate> clone() const = 0;
};


template<typename Derived>
class ClonableGate : public Gate {
public:
    using Gate::Gate;
    
    std::unique_ptr<Gate> clone() const override {
        return std::make_unique<Derived>(static_cast<const Derived&>(*this));
    }
};

} // namespace qsteedcpp