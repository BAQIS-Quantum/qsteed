#pragma once

#include "parameter.h"
#include "circuit_instruction.h"
#include <vector>
#include <memory>
#include <string>
#include <map>

namespace qsteedcpp {

struct ParameterGradInfo {
    size_t gate_index;
    size_t param_index;
    double grad_value;
    
    ParameterGradInfo(size_t gi, size_t pi, double gv) 
        : gate_index(gi), param_index(pi), grad_value(gv) {}
};

using ParameterGrads = std::map<std::string, std::vector<ParameterGradInfo>>;



class QuantumCircuit {
private:
    int num_qubits_;
    int num_clbits_;
    std::vector<CircuitInstruction> instructions_;
    
    // Grad
    mutable ParameterGrads parameter_grads_;
    mutable std::vector<std::string> variables_;
    mutable bool grads_computed_;
    
public:
    explicit QuantumCircuit(int num_qubits, int num_clbits = 0) 
        : num_qubits_(num_qubits), num_clbits_(num_clbits), grads_computed_(false) {}
    
    int num_qubits() const { return num_qubits_; }
    int num_clbits() const { return num_clbits_; }
    size_t size() const { return instructions_.size(); }
    size_t num_gates() const { 
        size_t count = 0;
        for (const auto& inst : instructions_) {
            if (inst.is_gate()) count++;
        }
        return count;
    }
    
    // Single-qubit gates
    void h(int qubit);
    void x(int qubit);
    void y(int qubit);
    void z(int qubit);
    void rx(const Parameter& theta, int qubit);
    void ry(const Parameter& phi, int qubit);
    void rz(const Parameter& lambda, int qubit);
    void s(int qubit);
    void sdg(int qubit);
    void t(int qubit);
    void tdg(int qubit);
    void p(const Parameter& lambda, int qubit);
    void u3(const Parameter& theta, const Parameter& phi, const Parameter& lambda, int qubit);
    
    // Two-qubit gates
    void cnot(int control, int target);
    void cz(int control, int target);
    void swap(int qubit1, int qubit2);
    void iswap(int qubit1, int qubit2);
    void rxx(const Parameter& theta, int qubit1, int qubit2);
    void ryy(const Parameter& theta, int qubit1, int qubit2);
    void rzz(const Parameter& theta, int qubit1, int qubit2);
    
    // Three-qubit gates
    void ccx(int control1, int control2, int target);
    void toffoli(int control1, int control2, int target);
    
    // Circuit operations
    void measure(int qubit, int clbit);
    void measure(const std::vector<int>& qubits, const std::vector<int>& clbits);
    void measure_all();  // 测量所有量子比特到对应的经典比特
    void barrier(const std::vector<int>& qubits = {});
    void reset(int qubit);
    void append_c_if(std::unique_ptr<Gate> gate, const std::vector<int>& qubits, int clbit, int value);
    

    const std::vector<CircuitInstruction>& get_instructions() const {
        return instructions_;
    }
    void add_gate(std::unique_ptr<Gate> gate, const std::vector<int>& qubits);
    
    // Parameter handling
    std::vector<std::string> get_parameters() const;
    const ParameterGrads& get_parameter_grads() const;
    const std::vector<std::string>& get_variables() const;
    void update_parameters(const std::map<std::string, double>& param_values);
    
    // Gradient computation
    std::map<std::string, double> compute_gradients_for_parameter(
        size_t inst_index, 
        size_t param_index,
        const std::map<std::string, double>& param_values) const;
    
    void print_parameter_grads() const;
    void print() const;
    void print_enhanced() const;  // 使用增强版 FTXUI 绘制
    void print_canvas() const;    // 使用 Canvas 绘制
    void print_interactive() const; // 交互式显示，支持滚动和缩放

private:
    void validate_qubit_index(int qubit) const;
    void validate_clbit_index(int clbit) const;
    void validate_gate_qubits(const std::vector<int>& qubits, const std::string& gate_name);

    void compute_parameter_grads() const;
};

} // namespace qsteedcpp