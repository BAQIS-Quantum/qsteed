#pragma once

#include "standard_gates.h"
#include "parameter.h"
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
    std::vector<std::pair<std::unique_ptr<Gate>, std::vector<int>>> instructions_;
    
    // Grad
    mutable ParameterGrads parameter_grads_;
    mutable std::vector<std::string> variables_;
    mutable bool grads_computed_;
    
public:
    explicit QuantumCircuit(int num_qubits) 
        : num_qubits_(num_qubits), grads_computed_(false) {}
    
    int num_qubits() const { return num_qubits_; }
    size_t num_gates() const { return instructions_.size(); }
    
    void h(int qubit) {
        if (qubit >= num_qubits_) {
            throw std::out_of_range("Qubit index out of range");
        }
        instructions_.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubit});
    }
    
    void x(int qubit) {
        if (qubit >= num_qubits_) {
            throw std::out_of_range("Qubit index out of range");
        }
        instructions_.emplace_back(std::make_unique<XGate>(), std::vector<int>{qubit});
    }
    
    void y(int qubit) {
        if (qubit >= num_qubits_) {
            throw std::out_of_range("Qubit index out of range");
        }
        instructions_.emplace_back(std::make_unique<YGate>(), std::vector<int>{qubit});
    }
    
    void z(int qubit) {
        if (qubit >= num_qubits_) {
            throw std::out_of_range("Qubit index out of range");
        }
        instructions_.emplace_back(std::make_unique<ZGate>(), std::vector<int>{qubit});
    }
    
    void rx(const Parameter& theta, int qubit) {
        if (qubit >= num_qubits_) {
            throw std::out_of_range("Qubit index out of range");
        }
        instructions_.emplace_back(std::make_unique<RXGate>(theta), std::vector<int>{qubit});
    }
    
    void ry(const Parameter& phi, int qubit) {
        if (qubit >= num_qubits_) {
            throw std::out_of_range("Qubit index out of range");
        }
        instructions_.emplace_back(std::make_unique<RYGate>(phi), std::vector<int>{qubit});
    }
    
    void rz(const Parameter& lambda, int qubit) {
        if (qubit >= num_qubits_) {
            throw std::out_of_range("Qubit index out of range");
        }
        instructions_.emplace_back(std::make_unique<RZGate>(lambda), std::vector<int>{qubit});
    }
    
    void cnot(int control, int target) {
        if (control >= num_qubits_ || target >= num_qubits_) {
            throw std::out_of_range("Qubit index out of range");
        }
        if (control == target) {
            throw std::invalid_argument("Control and target qubits cannot be the same");
        }
        instructions_.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{control, target});
    }
    
    void rxx(const Parameter& theta, int qubit1, int qubit2) {
        if (qubit1 >= num_qubits_ || qubit2 >= num_qubits_) {
            throw std::out_of_range("Qubit index out of range");
        }
        if (qubit1 == qubit2) {
            throw std::invalid_argument("Both qubits cannot be the same");
        }
        instructions_.emplace_back(std::make_unique<RXXGate>(theta), std::vector<int>{qubit1, qubit2});
    }
    
    void ryy(const Parameter& theta, int qubit1, int qubit2) {
        if (qubit1 >= num_qubits_ || qubit2 >= num_qubits_) {
            throw std::out_of_range("Qubit index out of range");
        }
        if (qubit1 == qubit2) {
            throw std::invalid_argument("Both qubits cannot be the same");
        }
        instructions_.emplace_back(std::make_unique<RYYGate>(theta), std::vector<int>{qubit1, qubit2});
    }
    
    void rzz(const Parameter& theta, int qubit1, int qubit2) {
        if (qubit1 >= num_qubits_ || qubit2 >= num_qubits_) {
            throw std::out_of_range("Qubit index out of range");
        }
        if (qubit1 == qubit2) {
            throw std::invalid_argument("Both qubits cannot be the same");
        }
        instructions_.emplace_back(std::make_unique<RZZGate>(theta), std::vector<int>{qubit1, qubit2});
    }
    
    void add_gate(std::unique_ptr<Gate> gate, const std::vector<int>& qubits) {
        if (static_cast<int>(qubits.size()) != gate->get_qubit_count()) {
            throw std::invalid_argument("Number of qubits doesn't match gate requirement");
        }
        
        for (int qubit : qubits) {
            if (qubit >= num_qubits_) {
                throw std::out_of_range("Qubit index out of range");
            }
        }
        
        instructions_.emplace_back(std::move(gate), qubits);
    }
    

    void print() const {
        std::cout << "QuantumCircuit with " << num_qubits_ << " qubits and " << num_gates() << " gates:" << std::endl;
        for (size_t i = 0; i < instructions_.size(); ++i) {
            const auto& instruction = instructions_[i];
            std::cout << "Gate " << i << ": " << instruction.first->get_name();
            std::cout << " on qubits [";
            for (size_t j = 0; j < instruction.second.size(); ++j) {
                if (j > 0) std::cout << ", ";
                std::cout << instruction.second[j];
            }
            std::cout << "]";
            if (instruction.first->has_parameters()) {
                std::cout << " with " << instruction.first->parameter_count() << " parameters";
            }
            std::cout << std::endl;
        }
    }
    
    // 获取电路中的所有参数
    std::vector<std::string> get_parameters() const {
        std::vector<std::string> all_params;
        for (const auto& instruction : instructions_) {
            if (instruction.first->has_parameters()) {
                for (size_t i = 0; i < instruction.first->parameter_count(); ++i) {
                    auto param_names = instruction.first->get_parameter(i).get_parameters();
                    all_params.insert(all_params.end(), param_names.begin(), param_names.end());
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
        for (auto& instruction : instructions_) {
            if (instruction.first->has_parameters()) {
                instruction.first->update_parameters(param_values);
            }
        }
        // 参数更新后，需要重新计算梯度
        grads_computed_ = false;
    }
    
    // 计算特定参数的梯度（使用autodiff）
    std::map<std::string, double> compute_gradients_for_parameter(
        size_t gate_index, 
        size_t param_index,
        const std::map<std::string, double>& param_values) const {
        
        if (gate_index >= instructions_.size()) {
            throw std::out_of_range("Gate index out of range");
        }
        
        const auto& gate = instructions_[gate_index].first;
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
    // 计算并缓存参数梯度信息
    void compute_parameter_grads() const {
        parameter_grads_.clear();
        std::set<std::string> unique_variables;
        
        // 遍历所有门和它们的参数
        for (size_t gate_idx = 0; gate_idx < instructions_.size(); ++gate_idx) {
            const auto& gate = instructions_[gate_idx].first;
            
            if (gate->has_parameters()) {
                for (size_t param_idx = 0; param_idx < gate->parameter_count(); ++param_idx) {
                    const Parameter& param = gate->get_parameter(param_idx);
                    
                    // 获取这个参数中包含的所有变量名
                    auto param_variables = param.get_parameters();
                    
                    for (const auto& var_name : param_variables) {
                        if (!var_name.empty()) {
                            unique_variables.insert(var_name);
                            
                            // 计算这个变量在当前参数中的梯度值
                            // 这里我们假设对于简单的Parameter，梯度为1.0
                            // 对于复杂的ParameterExpression，需要使用autodiff计算
                            double grad_value = 1.0;
                            
                            // 如果参数包含表达式，计算实际的梯度
                            if (param_variables.size() > 1 || param.to_string() != var_name) {
                                // 这是一个表达式，需要计算偏导数
                                std::map<std::string, double> dummy_values;
                                for (const auto& v : param_variables) {
                                    dummy_values[v] = 1.0; // 使用默认值
                                }
                                
                                try {
                                    auto gradients = param.compute_gradients(dummy_values);
                                    auto it = gradients.find(var_name);
                                    if (it != gradients.end()) {
                                        grad_value = it->second;
                                    }
                                } catch (...) {
                                    // 如果梯度计算失败，使用默认值1.0
                                    grad_value = 1.0;
                                }
                            }
                            
                            // 添加到parameter_grads_映射中
                            parameter_grads_[var_name].emplace_back(gate_idx, param_idx, grad_value);
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