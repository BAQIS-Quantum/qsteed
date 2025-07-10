#pragma once

#include "standard_gates.h"
#include "parameter.h"
#include "../matrix.h"
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <iostream>

namespace qsteedcpp {

class QuantumCircuit {
private:
    int num_qubits_;
    std::vector<std::pair<std::unique_ptr<Gate>, std::vector<int>>> instructions_;
    
public:
    explicit QuantumCircuit(int num_qubits) : num_qubits_(num_qubits) {}
    
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
    

};

} // namespace qsteedcpp 