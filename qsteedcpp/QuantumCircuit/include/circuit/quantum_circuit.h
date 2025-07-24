#pragma once

#include "standard_gates.h"
#include "parameter.h"
#include "circuit_instruction.h"
#include "../matrix.h"
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <iostream>
#include <set>

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
    
    void h(int qubit) {
        validate_qubit_index(qubit);
        add_gate(std::make_unique<HGate>(), {qubit});
    }
    
    void x(int qubit) {
        validate_qubit_index(qubit);
        add_gate(std::make_unique<XGate>(), {qubit});
    }
    
    void y(int qubit) {
        validate_qubit_index(qubit);
        add_gate(std::make_unique<YGate>(), {qubit});
    }
    
    void z(int qubit) {
        validate_qubit_index(qubit);
        add_gate(std::make_unique<ZGate>(), {qubit});
    }
    
    void rx(const Parameter& theta, int qubit) {
        validate_qubit_index(qubit);
        add_gate(std::make_unique<RXGate>(theta), {qubit});
    }
    
    void ry(const Parameter& phi, int qubit) {
        validate_qubit_index(qubit);
        add_gate(std::make_unique<RYGate>(phi), {qubit});
    }
    
    void rz(const Parameter& lambda, int qubit) {
        validate_qubit_index(qubit);
        add_gate(std::make_unique<RZGate>(lambda), {qubit});
    }
    
    void cnot(int control, int target) {
        validate_qubit_index(control);
        validate_qubit_index(target);
        if (control == target) {
            throw std::invalid_argument("Control and target qubits cannot be the same");
        }
        add_gate(std::make_unique<CNOTGate>(), {control, target});
    }
    
    void rxx(const Parameter& theta, int qubit1, int qubit2) {
        validate_qubit_index(qubit1);
        validate_qubit_index(qubit2);
        if (qubit1 == qubit2) {
            throw std::invalid_argument("Both qubits cannot be the same");
        }
        add_gate(std::make_unique<RXXGate>(theta), {qubit1, qubit2});
    }
    
    void ryy(const Parameter& theta, int qubit1, int qubit2) {
        validate_qubit_index(qubit1);
        validate_qubit_index(qubit2);
        if (qubit1 == qubit2) {
            throw std::invalid_argument("Both qubits cannot be the same");
        }
        add_gate(std::make_unique<RYYGate>(theta), {qubit1, qubit2});
    }
    
    void rzz(const Parameter& theta, int qubit1, int qubit2) {
        validate_qubit_index(qubit1);
        validate_qubit_index(qubit2);
        if (qubit1 == qubit2) {
            throw std::invalid_argument("Both qubits cannot be the same");
        }
        add_gate(std::make_unique<RZZGate>(theta), {qubit1, qubit2});
    }
    
    void s(int qubit) {
        validate_qubit_index(qubit);
        add_gate(std::make_unique<SGate>(), {qubit});
    }
    
    void sdg(int qubit) {
        validate_qubit_index(qubit);
        add_gate(std::make_unique<SdgGate>(), {qubit});
    }
    
    void t(int qubit) {
        validate_qubit_index(qubit);
        add_gate(std::make_unique<TGate>(), {qubit});
    }
    
    void tdg(int qubit) {
        validate_qubit_index(qubit);
        add_gate(std::make_unique<TdgGate>(), {qubit});
    }
    
    void cz(int control, int target) {
        validate_qubit_index(control);
        validate_qubit_index(target);
        if (control == target) {
            throw std::invalid_argument("Control and target qubits cannot be the same");
        }
        add_gate(std::make_unique<CZGate>(), {control, target});
    }
    
    void swap(int qubit1, int qubit2) {
        validate_qubit_index(qubit1);
        validate_qubit_index(qubit2);
        if (qubit1 == qubit2) {
            throw std::invalid_argument("Cannot swap a qubit with itself");
        }
        add_gate(std::make_unique<SwapGate>(), {qubit1, qubit2});
    }
    
    void iswap(int qubit1, int qubit2) {
        validate_qubit_index(qubit1);
        validate_qubit_index(qubit2);
        if (qubit1 == qubit2) {
            throw std::invalid_argument("Cannot iswap a qubit with itself");
        }
        add_gate(std::make_unique<iSwapGate>(), {qubit1, qubit2});
    }
    
    void ccx(int control1, int control2, int target) {
        validate_qubit_index(control1);
        validate_qubit_index(control2);
        validate_qubit_index(target);
        if (control1 == control2 || control1 == target || control2 == target) {
            throw std::invalid_argument("Control and target qubits must be different");
        }
        add_gate(std::make_unique<ToffoliGate>(), {control1, control2, target});
    }
    
    void toffoli(int control1, int control2, int target) {
        ccx(control1, control2, target);
    }
    
    void p(const Parameter& lambda, int qubit) {
        validate_qubit_index(qubit);
        add_gate(std::make_unique<PhaseGate>(lambda), {qubit});
    }
    
    void u3(const Parameter& theta, const Parameter& phi, const Parameter& lambda, int qubit) {
        validate_qubit_index(qubit);
        add_gate(std::make_unique<U3Gate>(theta, phi, lambda), {qubit});
    }
    
    void measure(int qubit, int clbit) {
        validate_qubit_index(qubit);
        validate_clbit_index(clbit);
        instructions_.emplace_back(Measurement(qubit, clbit));
    }
    
    void barrier(const std::vector<int>& qubits = {}) {
        std::vector<int> barrier_qubits = qubits;
        if (barrier_qubits.empty()) {
            for (int i = 0; i < num_qubits_; ++i) {
                barrier_qubits.push_back(i);
            }
        } else {
            for (int q : barrier_qubits) {
                validate_qubit_index(q);
            }
        }
        instructions_.emplace_back(Barrier(barrier_qubits));
    }
    
    void reset(int qubit) {
        validate_qubit_index(qubit);
        instructions_.emplace_back(Reset(qubit));
    }
    
    void append_c_if(std::unique_ptr<Gate> gate, const std::vector<int>& qubits, int clbit, int value) {
        validate_clbit_index(clbit);
        CircuitInstruction inst(std::move(gate), qubits);
        inst.c_if(clbit, value);
        instructions_.push_back(std::move(inst));
    }
    
    const std::vector<CircuitInstruction>& get_instructions() const {
        return instructions_;
    }
    
    void add_gate(std::unique_ptr<Gate> gate, const std::vector<int>& qubits) {
        if (static_cast<int>(qubits.size()) != gate->get_qubit_count()) {
            throw std::invalid_argument("Number of qubits doesn't match gate requirement");
        }
        
        for (int qubit : qubits) {
            validate_qubit_index(qubit);
        }
        
        instructions_.emplace_back(std::move(gate), qubits);
    }
    

    void print() const {
        std::cout << "QuantumCircuit with " << num_qubits_ << " qubits";
        if (num_clbits_ > 0) {
            std::cout << " and " << num_clbits_ << " classical bits";
        }
        std::cout << ":" << std::endl;
        
        for (size_t i = 0; i < instructions_.size(); ++i) {
            const auto& inst = instructions_[i];
            std::cout << "[" << i << "] " << inst.name();
            
            if (!inst.qubits.empty()) {
                std::cout << " q[";
                for (size_t j = 0; j < inst.qubits.size(); ++j) {
                    if (j > 0) std::cout << ",";
                    std::cout << inst.qubits[j];
                }
                std::cout << "]";
            }
            
            // 打印clbit索引
            if (!inst.clbits.empty()) {
                std::cout << " c[";
                for (size_t j = 0; j < inst.clbits.size(); ++j) {
                    if (j > 0) std::cout << ",";
                    std::cout << inst.clbits[j];
                }
                std::cout << "]";
            }
            
            if (inst.condition.has_value()) {
                std::cout << " if(c[" << inst.condition->clbit_index << "]==" << inst.condition->value << ")";
            }
            
            if (inst.is_gate()) {
                const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
                if (gate->has_parameters()) {
                    std::cout << " [" << gate->parameter_count() << " params]";
                }
            }
            
            std::cout << std::endl;
        }
    }
    
    // 获取电路中的所有参数
    std::vector<std::string> get_parameters() const {
        std::vector<std::string> all_params;
        for (const auto& inst : instructions_) {
            if (inst.is_gate()) {
                const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
                if (gate->has_parameters()) {
                    for (size_t i = 0; i < gate->parameter_count(); ++i) {
                        auto param_names = gate->get_parameter(i).get_parameters();
                        all_params.insert(all_params.end(), param_names.begin(), param_names.end());
                    }
                }
            }
        }
        return all_params;
    }
    
    // 获取参数梯度信息（类似Python版本的get_parameter_grads）
    const ParameterGrads& get_parameter_grads() const {
        if (!grads_computed_) {
            compute_parameter_grads();
        }
        return parameter_grads_;
    }
    
    // 获取电路中的所有变量（去重后的参数列表）
    const std::vector<std::string>& get_variables() const {
        if (!grads_computed_) {
            compute_parameter_grads();
        }
        return variables_;
    }
    
    // 更新参数值（类似Python版本的_update_params）
    void update_parameters(const std::map<std::string, double>& param_values) {
        for (auto& inst : instructions_) {
            if (inst.is_gate()) {
                auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
                if (gate->has_parameters()) {
                    gate->update_parameters(param_values);
                }
            }
        }
        // 参数更新后，需要重新计算梯度
        grads_computed_ = false;
    }
    
    // 计算特定参数的梯度（使用autodiff）
    std::map<std::string, double> compute_gradients_for_parameter(
        size_t inst_index, 
        size_t param_index,
        const std::map<std::string, double>& param_values) const {
        
        if (inst_index >= instructions_.size()) {
            throw std::out_of_range("Instruction index out of range");
        }
        
        const auto& inst = instructions_[inst_index];
        if (!inst.is_gate()) {
            throw std::invalid_argument("Instruction is not a gate");
        }
        
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        if (!gate->has_parameters() || param_index >= gate->parameter_count()) {
            throw std::out_of_range("Parameter index out of range");
        }
        
        const Parameter& param = gate->get_parameter(param_index);
        return param.compute_gradients(param_values);
    }
    
    // 打印参数梯度信息（用于调试）
    void print_parameter_grads() const {
        const auto& grads = get_parameter_grads();
        const auto& vars = get_variables();
        
        std::cout << "Parameter Gradients Information:" << std::endl;
        std::cout << "Total variables: " << vars.size() << std::endl;
        
        for (const auto& var : vars) {
            std::cout << "Variable: " << var << std::endl;
            auto it = grads.find(var);
            if (it != grads.end()) {
                for (const auto& grad_info : it->second) {
                    std::cout << "  Gate[" << grad_info.gate_index << "] Param[" 
                              << grad_info.param_index << "] Grad: " 
                              << grad_info.grad_value << std::endl;
                }
            }
        }
        std::cout << std::endl;
    }

private:
    void validate_qubit_index(int qubit) const {
        if (qubit < 0 || qubit >= num_qubits_) {
            throw std::out_of_range("Qubit index " + std::to_string(qubit) + " out of range [0, " + std::to_string(num_qubits_) + ")");
        }
    }
    
    void validate_clbit_index(int clbit) const {
        if (clbit < 0 || clbit >= num_clbits_) {
            throw std::out_of_range("Classical bit index " + std::to_string(clbit) + " out of range [0, " + std::to_string(num_clbits_) + ")");
        }
    }
    
    // 计算并缓存参数梯度信息
    void compute_parameter_grads() const {
        parameter_grads_.clear();
        std::set<std::string> unique_variables;
        
        for (size_t inst_idx = 0; inst_idx < instructions_.size(); ++inst_idx) {
            const auto& inst = instructions_[inst_idx];
            if (!inst.is_gate()) continue;
            
            const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
            
            if (gate->has_parameters()) {
                for (size_t param_idx = 0; param_idx < gate->parameter_count(); ++param_idx) {
                    const Parameter& param = gate->get_parameter(param_idx);
                    
                    // 获取这个参数中包含的所有变量名
                    auto param_variables = param.get_parameters();
                    
                    for (const auto& var_name : param_variables) {
                        if (!var_name.empty()) {
                            unique_variables.insert(var_name);
                            
                            // 计算这个变量在当前参数中的梯度值
                            double grad_value = 1.0;
                            if (param_variables.size() > 1 || param.to_string() != var_name) {
                                // 这是一个表达式，需要计算偏导数
                                std::map<std::string, double> dummy_values;
                                for (const auto& v : param_variables) {
                                    dummy_values[v] = 1.0;
                                }
                                
                                try {
                                    auto gradients = param.compute_gradients(dummy_values);
                                    auto it = gradients.find(var_name);
                                    if (it != gradients.end()) {
                                        grad_value = it->second;
                                    }
                                } catch (...) {
                                    grad_value = 1.0;
                                }
                            }
                            
                            // 添加到parameter_grads_映射中
                            parameter_grads_[var_name].emplace_back(inst_idx, param_idx, grad_value);
                        }
                    }
                }
            }
        }
        
        // 更新variables_列表
        variables_.clear();
        variables_.reserve(unique_variables.size());
        for (const auto& var : unique_variables) {
            variables_.push_back(var);
        }
        
        grads_computed_ = true;
    }

};

} // namespace qsteedcpp 