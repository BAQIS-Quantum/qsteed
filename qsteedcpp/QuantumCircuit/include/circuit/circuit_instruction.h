#pragma once
#include "../gates/base_gate.h"
#include <memory>
#include <vector>
#include <optional>
#include <variant>

namespace qsteedcpp {

class Measurement {
public:
    int qubit_index;
    int clbit_index;
    
    Measurement(int q, int c) : qubit_index(q), clbit_index(c) {}
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
    
    CircuitInstruction(std::unique_ptr<Gate> gate, const std::vector<int>& q)
        : operation(std::move(gate)), qubits(q) {}
    
    CircuitInstruction(const Measurement& meas)
        : operation(meas), qubits{meas.qubit_index}, clbits{meas.clbit_index} {}
    
    CircuitInstruction(const Barrier& barrier)
        : operation(barrier), qubits(barrier.qubits) {}
    
    CircuitInstruction(const Reset& reset)
        : operation(reset), qubits{reset.qubit_index} {}
    
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
            return std::get<std::unique_ptr<Gate>>(operation)->get_name();
        } else if (is_measurement()) {
            return "measure";
        } else if (is_barrier()) {
            return "barrier";
        } else if (is_reset()) {
            return "reset";
        }
        return "unknown";
    }
};

} // namespace qsteedcpp