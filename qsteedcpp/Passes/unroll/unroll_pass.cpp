#include "Passes/unroll/unroll_pass.h"
#include "Passes/unroll/decomposition_rules.h"
#include "QuantumCircuit/circuit/quantum_circuit.h"
#include <algorithm>

namespace qsteedcpp {

UnrollPass::UnrollPass(const std::set<std::string>& basis_gates, 
                       bool initialize_default_rules)
    : basis_gates_(basis_gates), 
      initialize_default_rules_(initialize_default_rules) {
    
    if (initialize_default_rules_) {
        initialize_standard_rules(rule_manager_);
    }
}

void UnrollPass::add_rule(const std::string& gate_name, 
                          const RuleManager::DecompositionRule& rule) {
    rule_manager_.register_rule(gate_name, rule);
}

void UnrollPass::run(QuantumCircuit& circuit) {
    if (!should_run(circuit)) {
        return;
    }
    
    QuantumCircuit temp_circuit(circuit.num_qubits(), circuit.num_clbits());
    bool fully_decomposed = true;
    
    for (const auto& inst : circuit.get_instructions()) {
        if (!inst.is_gate()) {
            if (inst.is_measurement()) {
                const auto& meas = std::get<Measurement>(inst.operation);
                temp_circuit.measure(meas.qubit_indices, meas.clbit_indices);
            } else if (inst.is_barrier()) {
                temp_circuit.barrier(inst.qubits);
            } else if (inst.is_reset()) {
                temp_circuit.reset(inst.qubits[0]);
            }
            continue;
        }
        
        auto gate_name = inst.name();
        std::transform(gate_name.begin(), gate_name.end(), gate_name.begin(), ::tolower);
        
        if (is_basis_gate(gate_name)) {
            const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
            temp_circuit.add_gate(gate->clone(), inst.qubits);
        } else {
            auto decomposed = decompose_instruction(inst);
            if (decomposed.empty()) {
                std::cerr << "Warning: UnrollPass failed - Gate '" << gate_name 
                         << "' cannot be decomposed to basis gates. Circuit unchanged." << std::endl;
                fully_decomposed = false;
                break;
            } else {
                // 检查分解结果是否都是基础门
                for (const auto& decomp_inst : decomposed) {
                    if (decomp_inst.is_gate()) {
                        auto decomp_name = decomp_inst.name();
                        std::transform(decomp_name.begin(), decomp_name.end(), decomp_name.begin(), ::tolower);
                        if (!is_basis_gate(decomp_name)) {
                            std::cerr << "Warning: UnrollPass failed - Gate '" << gate_name 
                                     << "' decomposed to non-basis gate '" << decomp_name 
                                     << "'. Circuit unchanged." << std::endl;
                            fully_decomposed = false;
                            break;
                        }
                    }
                }
                
                if (!fully_decomposed) {
                    break;
                }
                
                for (auto& decomp_inst : decomposed) {
                    if (decomp_inst.is_gate()) {
                        auto& gate = std::get<std::unique_ptr<Gate>>(decomp_inst.operation);
                        temp_circuit.add_gate(std::move(gate), decomp_inst.qubits);
                    }
                }
            }
        }
    }
    
    if (fully_decomposed) {
        circuit = std::move(temp_circuit);
    }
}

bool UnrollPass::try_run(QuantumCircuit& circuit) {
    if (!should_run(circuit)) {
        return true;
    }
    
    QuantumCircuit new_circuit(circuit.num_qubits(), circuit.num_clbits());
    bool fully_decomposed = true;
    
    for (const auto& inst : circuit.get_instructions()) {
        if (!inst.is_gate()) {
            if (inst.is_measurement()) {
                const auto& meas = std::get<Measurement>(inst.operation);
                new_circuit.measure(meas.qubit_indices, meas.clbit_indices);
            } else if (inst.is_barrier()) {
                new_circuit.barrier(inst.qubits);
            } else if (inst.is_reset()) {
                new_circuit.reset(inst.qubits[0]);
            }
            continue;
        }
        
        auto gate_name = inst.name();
        std::transform(gate_name.begin(), gate_name.end(), gate_name.begin(), ::tolower);
        
        if (is_basis_gate(gate_name)) {
            const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
            new_circuit.add_gate(gate->clone(), inst.qubits);
        } else {
            auto decomposed = decompose_instruction(inst);
            if (decomposed.empty()) {
                std::cerr << "Warning: Gate '" << gate_name 
                         << "' cannot be decomposed to basis gates. Keeping original gate." << std::endl;
                const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
                new_circuit.add_gate(gate->clone(), inst.qubits);
                fully_decomposed = false;
            } else {
                for (auto& decomp_inst : decomposed) {
                    if (decomp_inst.is_gate()) {
                        auto& gate = std::get<std::unique_ptr<Gate>>(decomp_inst.operation);
                        new_circuit.add_gate(std::move(gate), decomp_inst.qubits);
                    }
                }
            }
        }
    }
    
    circuit = std::move(new_circuit);
    return fully_decomposed;
}

bool UnrollPass::should_run(const QuantumCircuit& circuit) const {
    for (const auto& inst : circuit.get_instructions()) {
        if (!inst.is_gate()) continue;
        
        auto gate_name = inst.name();
        std::transform(gate_name.begin(), gate_name.end(), gate_name.begin(), ::tolower);
        
        if (!is_basis_gate(gate_name)) {
            return true;
        }
    }
    return false;
}

bool UnrollPass::is_basis_gate(const std::string& gate_name) const {
    return basis_gates_.count(gate_name) > 0;
}

std::vector<CircuitInstruction> UnrollPass::decompose_instruction(
    const CircuitInstruction& inst) {
    
    std::vector<CircuitInstruction> result;
    std::vector<CircuitInstruction> work_stack;
    
    work_stack.push_back(inst.clone());
    while (!work_stack.empty()) {
        CircuitInstruction current = std::move(work_stack.back());
        work_stack.pop_back();
        
        auto gate_name = current.name();
        std::transform(gate_name.begin(), gate_name.end(), gate_name.begin(), ::tolower);
        
        if (is_basis_gate(gate_name)) {
            result.push_back(std::move(current));
        } else {
            auto rule = rule_manager_.select_rule(gate_name, basis_gates_);
            
            if (rule.has_value()) {
                auto decomposed = rule->decomposer(current);
                for (auto it = decomposed.rbegin(); it != decomposed.rend(); ++it) {
                    work_stack.push_back(std::move(*it));
                }
            } else {
                result.push_back(std::move(current));
            }
        }
    }
    
    return result;
}

} // namespace qsteedcpp