#pragma once
#include <set>
#include <string>
#include "../base_pass.h"
#include "rule_manager.h"

namespace qsteedcpp {

    class UnrollPass : public BasePass {
    private:
        RuleManager rule_manager_;
        std::set<std::string> basis_gates_;
        bool initialize_default_rules_;
        
    public:
        explicit UnrollPass(const std::set<std::string>& basis_gates, 
                        bool initialize_default_rules = true);
        
        void add_rule(const std::string& gate_name, 
                    const RuleManager::DecompositionRule& rule);
        
        void run(QuantumCircuit& circuit) override;
        
        // 尝试运行，返回是否完全分解到基础门
        bool try_run(QuantumCircuit& circuit);
        
        std::string name() const override { return "UnrollPass"; }
        
        bool should_run(const QuantumCircuit& circuit) const override;
        
        const std::set<std::string>& get_basis_gates() const { return basis_gates_; }
        
        void set_basis_gates(const std::set<std::string>& basis_gates) { 
            basis_gates_ = basis_gates; 
        }

    private:
        bool is_basis_gate(const std::string& gate_name) const;
        
        std::vector<CircuitInstruction> decompose_instruction(
            const CircuitInstruction& inst);
    };

} // namespace qsteedcpp