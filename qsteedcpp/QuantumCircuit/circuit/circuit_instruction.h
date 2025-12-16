#pragma once
#include <vector>
#include <optional>
#include <variant>
#include <memory>
#include <string>
#include <stdexcept>
#include "gates/base_gate.h"

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

class Delay {
public:
    int qubit_index;
    int duration;
    std::string unit;

    Delay(int pos, int dur, const std::string& u = "ns")
        : qubit_index(pos), duration(dur), unit(u) {}
};

class XYResonance {
public:
    int qubit_start;
    int qubit_end;
    int duration;
    std::string unit;

    XYResonance(int qs, int qe, int dur, const std::string& u = "ns")
        : qubit_start(qs), qubit_end(qe), duration(dur), unit(u) {}
};

struct Condition {
    int clbit_index;
    int value;

    Condition(int idx, int val) : clbit_index(idx), value(val) {}
};


class CircuitInstruction {
public:
    std::variant<std::unique_ptr<Gate>, Measurement, Barrier, Reset, Delay, XYResonance> operation;
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

    CircuitInstruction(const Delay& delay)
        : operation(delay), qubits{delay.qubit_index} {}

    CircuitInstruction(const XYResonance& xy)
        : operation(xy), qubits{xy.qubit_start, xy.qubit_end} {}

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

    bool is_delay() const {
        return std::holds_alternative<Delay>(operation);
    }

    bool is_xyresonance() const {
        return std::holds_alternative<XYResonance>(operation);
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
        } else if (is_delay()) {
            return "delay";
        } else if (is_xyresonance()) {
            return "xyresonance";
        }
        return "unknown";
    }

    std::optional<int> get_duration() const;
    std::optional<std::string> get_unit() const;
    
    std::string to_qasm(bool with_para = false) const;

    Matrix get_matrix() const {
        if (is_gate()) {
            return std::get<std::unique_ptr<Gate>>(operation)->get_matrix();
        } else {
            throw std::runtime_error("Cannot get matrix for non-gate instruction.");
        }
    }

    CircuitInstruction clone() const {
        return CircuitInstruction(*this); // Now we can just use the copy constructor
    }
};

} // namespace qsteedcpp