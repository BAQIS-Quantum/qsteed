#pragma once
#include "../gates/base_gate.h"
#include <memory>
#include <vector>
#include <optional>
#include <variant>
#include <type_traits> // Required for std::decay_t

namespace qsteedcpp {

class Measurement {
public:
    std::vector<int> qubit_indices;
    std::vector<int> clbit_indices;
    
    Measurement(int q, int c) 
        : qubit_indices{q}, clbit_indices{c} {}
    
    Measurement(const std::vector<int>& qubits, const std::vector<int>& clbits) 
        : qubit_indices(qubits), clbit_indices(clbits) {
        if (qubits.size() != clbits.size()) {
            throw std::invalid_argument("Number of qubits and clbits must match in measurement");
        }
    }
    
    int qubit_index() const { return qubit_indices.empty() ? -1 : qubit_indices[0]; }
    int clbit_index() const { return clbit_indices.empty() ? -1 : clbit_indices[0]; }
    
    size_t size() const { return qubit_indices.size(); }
};

class Barrier {
public:
    std::vector<int> qubits;
    
    explicit Barrier(const std::vector<int>& q) : qubits(q) {}
};

class Reset {
public:
    int qubit_index;
    
    explicit Reset(int q) : qubit_index(q) {}
};

struct Condition {
    int clbit_index;
    int value;
    
    Condition(int idx, int val) : clbit_index(idx), value(val) {}
};


class CircuitInstruction {
public:
    std::variant<std::unique_ptr<Gate>, Measurement, Barrier, Reset> operation;
    std::vector<int> qubits;
    std::vector<int> clbits;
    std::optional<Condition> condition;
    
    // Constructors
    CircuitInstruction(std::unique_ptr<Gate> gate, const std::vector<int>& q)
        : operation(std::move(gate)), qubits(q) {}
    
    CircuitInstruction(const Measurement& meas)
        : operation(meas), qubits(meas.qubit_indices), clbits(meas.clbit_indices) {}
    
    CircuitInstruction(const Barrier& barrier)
        : operation(barrier), qubits(barrier.qubits) {}
    
    CircuitInstruction(const Reset& reset)
        : operation(reset), qubits{reset.qubit_index} {}

    // 1. Copy Constructor
    CircuitInstruction(const CircuitInstruction& other) :
        qubits(other.qubits),
        clbits(other.clbits),
        condition(other.condition)
    {
        operation = std::visit([](const auto& arg) -> decltype(operation) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::unique_ptr<Gate>>) {
                return arg->clone(); // Deep copy the gate
            } else {
                return arg; // Copy other variant types
            }
        }, other.operation);
    }

    // 2. Copy Assignment Operator
    CircuitInstruction& operator=(const CircuitInstruction& other) {
        if (this != &other) {
            qubits = other.qubits;
            clbits = other.clbits;
            condition = other.condition;
            operation = std::visit([](const auto& arg) -> decltype(operation) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::unique_ptr<Gate>>) {
                    return arg->clone();
                } else {
                    return arg;
                }
            }, other.operation);
        }
        return *this;
    }

    // --- END: Corrected copy constructor and assignment operator ---

    CircuitInstruction& c_if(int clbit, int value) {
        condition = Condition{clbit, value};
        return *this;
    }
    
    bool is_gate() const {
        return std::holds_alternative<std::unique_ptr<Gate>>(operation);
    }
    
    bool is_measurement() const {
        return std::holds_alternative<Measurement>(operation);
    }
    
    bool is_barrier() const {
        return std::holds_alternative<Barrier>(operation);
    }
    
    bool is_reset() const {
        return std::holds_alternative<Reset>(operation);
    }
    
    std::string name() const {
        if (is_gate()) {
            return std::get<std::unique_ptr<Gate>>(operation)->name();
        } else if (is_measurement()) {
            return "measure";
        } else if (is_barrier()) {
            return "barrier";
        } else if (is_reset()) {
            return "reset";
        }
        return "unknown";
    }
    
    // clone method for explicit deep copy
    CircuitInstruction clone() const {
        return CircuitInstruction(*this); // Now we can just use the copy constructor
    }
};

} // namespace qsteedcpp