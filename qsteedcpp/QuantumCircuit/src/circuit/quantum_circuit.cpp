#include <set>
#include <iostream>
#include "circuit/quantum_circuit.h"
#include "gates/standard_gates.h"
#include "circuit/circuit_drawer.h"

namespace qsteedcpp {

void QuantumCircuit::validate_qubit_index(int qubit) const {
    if (qubit < 0 || qubit >= num_qubits_) {
        throw std::out_of_range("Qubit index " + std::to_string(qubit) + " out of range [0, " + std::to_string(num_qubits_) + ")");
    }
}

void QuantumCircuit::validate_clbit_index(int clbit) const {
    if (clbit < 0 || clbit >= num_clbits_) {
        throw std::out_of_range("Classical bit index " + std::to_string(clbit) + " out of range [0, " + std::to_string(num_clbits_) + ")");
    }
}

void QuantumCircuit::validate_gate_qubits(const std::vector<int>& qubits, const std::string& gate_name) {
    for (int qubit : qubits) {
        validate_qubit_index(qubit);
    }
    
    if (qubits.size() > 1) {
        if (qubits.size() == 2 && qubits[0] == qubits[1]) {
            throw std::invalid_argument(gate_name + " requires different qubits");
        }
        else if (qubits.size() > 2) {
            std::set<int> unique_qubits(qubits.begin(), qubits.end());
            if (unique_qubits.size() != qubits.size()) {
                throw std::invalid_argument(gate_name + " cannot operate on duplicate qubits");
            }
        }
    }
}


// Gates
void QuantumCircuit::h(int qubit) {
    add_gate(std::make_unique<HGate>(), {qubit});
}

void QuantumCircuit::x(int qubit) {
    add_gate(std::make_unique<XGate>(), {qubit});
}

void QuantumCircuit::y(int qubit) {
    add_gate(std::make_unique<YGate>(), {qubit});
}

void QuantumCircuit::z(int qubit) {
    add_gate(std::make_unique<ZGate>(), {qubit});
}

void QuantumCircuit::rx(const Parameter& theta, int qubit) {
    add_gate(std::make_unique<RXGate>(theta), {qubit});
}

void QuantumCircuit::ry(const Parameter& phi, int qubit) {
    add_gate(std::make_unique<RYGate>(phi), {qubit});
}

void QuantumCircuit::rz(const Parameter& lambda, int qubit) {
    add_gate(std::make_unique<RZGate>(lambda), {qubit});
}

void QuantumCircuit::cnot(int control, int target) {
    add_gate(std::make_unique<CNOTGate>(), {control, target});
}

void QuantumCircuit::rxx(const Parameter& theta, int qubit1, int qubit2) {
    add_gate(std::make_unique<RXXGate>(theta), {qubit1, qubit2});
}

void QuantumCircuit::ryy(const Parameter& theta, int qubit1, int qubit2) {
    add_gate(std::make_unique<RYYGate>(theta), {qubit1, qubit2});
}

void QuantumCircuit::rzz(const Parameter& theta, int qubit1, int qubit2) {
    add_gate(std::make_unique<RZZGate>(theta), {qubit1, qubit2});
}

void QuantumCircuit::s(int qubit) {
    add_gate(std::make_unique<SGate>(), {qubit});
}

void QuantumCircuit::sdg(int qubit) {
    add_gate(std::make_unique<SdgGate>(), {qubit});
}

void QuantumCircuit::t(int qubit) {
    add_gate(std::make_unique<TGate>(), {qubit});
}

void QuantumCircuit::tdg(int qubit) {
    add_gate(std::make_unique<TdgGate>(), {qubit});
}

void QuantumCircuit::cz(int control, int target) {
    add_gate(std::make_unique<CZGate>(), {control, target});
}

void QuantumCircuit::swap(int qubit1, int qubit2) {
    add_gate(std::make_unique<SwapGate>(), {qubit1, qubit2});
}

void QuantumCircuit::iswap(int qubit1, int qubit2) {
    add_gate(std::make_unique<iSwapGate>(), {qubit1, qubit2});
}

void QuantumCircuit::ccx(int control1, int control2, int target) {
    add_gate(std::make_unique<ToffoliGate>(), {control1, control2, target});
}

void QuantumCircuit::toffoli(int control1, int control2, int target) {
    ccx(control1, control2, target);
}

void QuantumCircuit::p(const Parameter& lambda, int qubit) {
    add_gate(std::make_unique<PhaseGate>(lambda), {qubit});
}

void QuantumCircuit::u3(const Parameter& theta, const Parameter& phi, const Parameter& lambda, int qubit) {
    add_gate(std::make_unique<U3Gate>(theta, phi, lambda), {qubit});
}

// Non-gate operations
void QuantumCircuit::measure(int qubit, int clbit) {
    validate_qubit_index(qubit);
    validate_clbit_index(clbit);
    instructions_.emplace_back(Measurement(qubit, clbit));
}

void QuantumCircuit::measure(const std::vector<int>& qubits, const std::vector<int>& clbits) {
    if (qubits.size() != clbits.size()) {
        throw std::invalid_argument("Number of qubits and clbits must match in measurement");
    }
    
    // 验证所有索引
    for (int qubit : qubits) {
        validate_qubit_index(qubit);
    }
    for (int clbit : clbits) {
        validate_clbit_index(clbit);
    }
    
    // 创建批量测量
    instructions_.emplace_back(Measurement(qubits, clbits));
}

void QuantumCircuit::measure_all() {
    if (num_qubits_ > num_clbits_) {
        throw std::invalid_argument("Not enough classical bits to measure all qubits");
    }
    
    std::vector<int> qubits(num_qubits_);
    std::vector<int> clbits(num_qubits_);
    
    for (int i = 0; i < num_qubits_; ++i) {
        qubits[i] = i;
        clbits[i] = i;
    }
    
    measure(qubits, clbits);
}

void QuantumCircuit::barrier(const std::vector<int>& qubits) {
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

void QuantumCircuit::reset(int qubit) {
    validate_qubit_index(qubit);
    instructions_.emplace_back(Reset(qubit));
}

void QuantumCircuit::append_c_if(std::unique_ptr<Gate> gate, const std::vector<int>& qubits, int clbit, int value) {
    validate_clbit_index(clbit);
    CircuitInstruction inst(std::move(gate), qubits);
    inst.c_if(clbit, value);
    instructions_.push_back(std::move(inst));
}

void QuantumCircuit::add_gate(std::unique_ptr<Gate> gate, const std::vector<int>& qubits) {
    if (static_cast<int>(qubits.size()) != gate->get_qubit_count()) {
        throw std::invalid_argument("Number of qubits doesn't match gate requirement");
    }
    
    validate_gate_qubits(qubits, gate->name());
    instructions_.emplace_back(std::move(gate), qubits);
}

void QuantumCircuit::print() const {
    CircuitDrawer drawer(instructions_, num_qubits_, num_clbits_);
    std::cout << drawer.draw() << std::endl;
}


// Parameter-related methods
std::vector<std::string> QuantumCircuit::get_parameters() const {
    std::vector<std::string> all_params;
    for (const auto& inst : instructions_) {
        if (inst.is_gate()) {
            const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
            if (gate->has_parameters()) {
                for (size_t i = 0; i < gate->parameter_count(); ++i) {
                    auto param_names = gate->get_parameter(i).get_variables();
                    all_params.insert(all_params.end(), param_names.begin(), param_names.end());
                }
            }
        }
    }
    return all_params;
}

const ParameterGrads& QuantumCircuit::get_parameter_grads() const {
    if (!grads_computed_) {
        compute_parameter_grads();
    }
    return parameter_grads_;
}

const std::vector<std::string>& QuantumCircuit::get_variables() const {
    if (!grads_computed_) {
        compute_parameter_grads();
    }
    return variables_;
}

void QuantumCircuit::update_parameters(const std::map<std::string, double>& param_values) {
    for (auto& inst : instructions_) {
        if (inst.is_gate()) {
            auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
            if (gate->has_parameters()) {
                gate->update_parameters(param_values);
            }
        }
    }
    grads_computed_ = false;
}

std::map<std::string, double> QuantumCircuit::compute_gradients_for_parameter(
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

void QuantumCircuit::print_parameter_grads() const {
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

// Private method implementation
void QuantumCircuit::compute_parameter_grads() const {
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
                auto param_variables = param.get_variables();
                
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

} // namespace qsteedcpp