#pragma once

#include <string>
#include <memory>
#include <set>
#include "expression/expr.h"

namespace qsteedcpp {

// Gate type enumeration for efficient gate identification
enum class GateType {
    // Single-qubit non-parametric gates
    H, X, Y, Z, S, SDG, T, TDG,

    // Single-qubit parametric gates
    RX, RY, RZ, P, U3,

    // Two-qubit non-parametric gates
    CNOT, CZ, SWAP, ISWAP,

    // Two-qubit parametric gates
    RXX, RYY, RZZ, CP,

    // Three-qubit gates
    TOFFOLI,

    // Custom/unknown gates
    CUSTOM
};

class Gate {
private:
    std::vector<Expr> param_expressions_;

public:
    Gate() = default;

    Gate(std::vector<Expr> exprs)
        : param_expressions_(std::move(exprs)) {}

    virtual ~Gate() = default;

    virtual GateType type() const = 0;
    virtual const char* name() const = 0;

    // Pure virtual function - subclasses return their static qubit_count
    virtual int get_qubit_count() const = 0;

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

    // Implement get_qubit_count by returning the Derived class's static constant
    int get_qubit_count() const override {
        return Derived::qubit_count;
    }

    std::unique_ptr<Gate> clone() const override {
        return std::make_unique<Derived>(static_cast<const Derived&>(*this));
    }
};

} // namespace qsteedcpp