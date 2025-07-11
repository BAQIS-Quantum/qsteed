#pragma once

#include "base_gate.h"
#include <memory>
#include <vector>
#include <optional>

namespace qsteedcpp {

// 测量操作
class Measurement {
public:
    int qubit_index;
    int clbit_index;
    
    Measurement(int q, int c) : qubit_index(q), clbit_index(c) {}
};

// 条件
struct Condition {
    int clbit_index;
    int value;
    
    Condition(int idx, int val) : clbit_index(idx), value(val) {}
};

// 扩展的指令类型
class CircuitInstruction {
public:
    // 使用 variant 支持不同类型的操作
    std::variant<std::unique_ptr<Gate>, Measurement> operation;
    std::vector<int> qubits;
    std::vector<int> clbits;
    std::optional<Condition> condition;
    
    // 门指令
    CircuitInstruction(std::unique_ptr<Gate> gate, const std::vector<int>& q)
        : operation(std::move(gate)), qubits(q) {}
    
    // 测量指令
    CircuitInstruction(const Measurement& meas)
        : operation(meas), qubits{meas.qubit_index}, clbits{meas.clbit_index} {}
    
    // 带条件的指令
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
};

} // namespace qsteedcpp