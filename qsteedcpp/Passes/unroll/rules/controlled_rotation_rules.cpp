#include "controlled_rotation_rules.h"
#include "Passes/unroll/rule_manager.h"
#include "Passes/unroll/decomposition_rules.h"
#include "QuantumCircuit/circuit/circuit_instruction.h"
#include "QuantumCircuit/gates/standard_gates.h"

namespace qsteedcpp {

namespace { // Anonymous namespace for local helpers and rule implementations

    void apply_condition(const CircuitInstruction& source, std::vector<CircuitInstruction>& targets) {
        if (source.condition.has_value()) {
            for (auto& target : targets) {
                target.condition = source.condition;
            }
        }
    }

    std::vector<CircuitInstruction> cy_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<SdgGate>(), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        result.emplace_back(std::make_unique<SGate>(), std::vector<int>{qubits[1]});
        apply_condition(inst, result);
        return result;
    }

    std::vector<CircuitInstruction> crx_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        // 获取 Expr 参数
        const auto& exprs = gate->get_parameter_expressions();
        const Expr& theta = exprs.empty() ? Expr(0.0) : exprs[0];
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<RYGate>(theta / 2.0), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        result.emplace_back(std::make_unique<RYGate>(-theta / 2.0), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        apply_condition(inst, result);
        return result;
    }

    std::vector<CircuitInstruction> cry_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        // 获取 Expr 参数
        const auto& exprs = gate->get_parameter_expressions();
        const Expr& theta = exprs.empty() ? Expr(0.0) : exprs[0];
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<RYGate>(theta / 2.0), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        result.emplace_back(std::make_unique<RYGate>(-theta / 2.0), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        apply_condition(inst, result);
        return result;
    }

    std::vector<CircuitInstruction> crz_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        // 获取 Expr 参数
        const auto& exprs = gate->get_parameter_expressions();
        const Expr& theta = exprs.empty() ? Expr(0.0) : exprs[0];
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<RZGate>(theta / 2.0), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        result.emplace_back(std::make_unique<RZGate>(-theta / 2.0), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        apply_condition(inst, result);
        return result;
    }

    std::vector<CircuitInstruction> cp_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        // 获取 Expr 参数
        const auto& exprs = gate->get_parameter_expressions();
        const Expr& theta = exprs.empty() ? Expr(0.0) : exprs[0];
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<PhaseGate>(theta / 2.0), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        result.emplace_back(std::make_unique<PhaseGate>(-theta / 2.0), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        result.emplace_back(std::make_unique<PhaseGate>(theta / 2.0), std::vector<int>{qubits[1]});
        apply_condition(inst, result);
        return result;
    }

} // anonymous namespace

void initialize_controlled_rotation_rules(RuleManager& manager) {
    manager.register_rule("cy", {
        .name = "cy_to_cnot",
        .target_gates = {"cx", "s", "sdg"},
        .decomposer = cy_to_cnot,
        .global_phase = 0.0,
        .priority = 1
    });
    manager.register_rule("crx", {
        .name = "crx_to_cnot",
        .target_gates = {"cx", "ry"}, 
        .decomposer = crx_to_cnot,
        .global_phase = 0.0,
        .priority = 1
    });
    manager.register_rule("cry", {
        .name = "cry_to_cnot",
        .target_gates = {"cx", "ry"},
        .decomposer = cry_to_cnot,
        .global_phase = 0.0,
        .priority = 1
    });
    manager.register_rule("crz", {
        .name = "crz_to_cnot",
        .target_gates = {"cx", "rz"},
        .decomposer = crz_to_cnot,
        .global_phase = 0.0,
        .priority = 1
    });
    manager.register_rule("cp", {
        .name = "cp_to_cnot",
        .target_gates = {"cx", "p"},
        .decomposer = cp_to_cnot,
        .global_phase = 0.0,
        .priority = 1
    });
}

} // namespace qsteedcpp