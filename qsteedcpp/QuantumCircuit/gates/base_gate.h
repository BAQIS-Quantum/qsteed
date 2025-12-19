#pragma once

#include <string>
#include <memory>
#include <set>
#include "QuantumCircuit/expression/expr.h"
#include "QuantumCircuit/gates/matrix.hpp"

namespace qsteedcpp {

// Gate type enumeration for efficient gate identification
enum class GateType {
    // Single-qubit non-parametric gates
    H, X, Y, Z, S, SDG, T, TDG, ID, SX, SXDG, SY, SYDG, W, SW, SWDG,

    // Single-qubit parametric gates
    RX, RY, RZ, P, U3,

    // Two-qubit non-parametric gates
    CNOT, CZ, CY, CS, CT, SWAP, ISWAP,

    // Two-qubit parametric gates
    RXX, RYY, RZZ, CP,

    // Three-qubit gates
    TOFFOLI, FREDKIN,

    // Multi-qubit gates
    MCX, MCY, MCZ, MCRX, MCRY, MCRZ, CONTROLLED_U,

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

    // Copy constructor: deep copy parameters
    Gate(const Gate& other) {
        param_expressions_.reserve(other.param_expressions_.size());
        for (const auto& expr : other.param_expressions_) {
            param_expressions_.push_back(expr.clone());
        }
    }

    Gate& operator=(const Gate& other) {
        if (this != &other) {
            param_expressions_.clear();
            param_expressions_.reserve(other.param_expressions_.size());
            for (const auto& expr : other.param_expressions_) {
                param_expressions_.push_back(expr.clone());
            }
        }
        return *this;
    }

    virtual ~Gate() = default;

    virtual GateType type() const = 0;

    virtual const char* name() const = 0;

    virtual int get_qubit_count() const = 0;

    virtual Matrix get_matrix() const = 0;

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