#include "Passes/unroll/rules/two_qubit_rules.h"
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

    std::vector<CircuitInstruction> cnot_to_cp(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CPGate>(Expr(M_PI)), qubits);
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        apply_condition(inst, result);
        return result;
    }

    std::vector<CircuitInstruction> cz_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        apply_condition(inst, result);
        return result;
    }

    std::vector<CircuitInstruction> rxx_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        // 获取 Expr 参数
        const auto& exprs = gate->get_parameter_expressions();
        const Expr& theta = exprs.empty() ? Expr(0.0) : exprs[0];
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[0], qubits[1]});
        result.emplace_back(std::make_unique<RZGate>(theta), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[0], qubits[1]});
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        apply_condition(inst, result);
        return result;
    }

    std::vector<CircuitInstruction> ryy_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        // 获取 Expr 参数
        const auto& exprs = gate->get_parameter_expressions();
        const Expr& theta = exprs.empty() ? Expr(0.0) : exprs[0];
        std::vector<CircuitInstruction> result;
        Expr pi_2 = Expr(M_PI / 2.0);
        result.emplace_back(std::make_unique<RXGate>(pi_2), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<RXGate>(pi_2), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[0], qubits[1]});
        result.emplace_back(std::make_unique<RZGate>(theta), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[0], qubits[1]});
        result.emplace_back(std::make_unique<RXGate>(-pi_2), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<RXGate>(-pi_2), std::vector<int>{qubits[1]});
        apply_condition(inst, result);
        return result;
    }

    std::vector<CircuitInstruction> rzz_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        // 获取 Expr 参数
        const auto& exprs = gate->get_parameter_expressions();
        const Expr& theta = exprs.empty() ? Expr(0.0) : exprs[0];
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[0], qubits[1]});
        result.emplace_back(std::make_unique<RZGate>(theta), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[0], qubits[1]});
        apply_condition(inst, result);
        return result;
    }

    std::vector<CircuitInstruction> iswap_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<SGate>(), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<SGate>(), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[1], qubits[0]});
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        apply_condition(inst, result);
        return result;
    }

} // anonymous namespace

void initialize_two_qubit_rules(RuleManager& manager) {
    manager.register_rule(std::vector<std::string>{"cx", "cnot"}, {
        .name = "cnot_to_cp",
        .target_gates = {"h", "cp"}, 
        .decomposer = cnot_to_cp,
        .global_phase = 0.0,
        .priority = 2
    });
    manager.register_rule("cz", {
        .name = "cz_to_cnot",
        .target_gates = {"h", "cx"},
        .decomposer = cz_to_cnot,
        .global_phase = 0.0,
        .priority = 2
    });
    manager.register_rule("rxx", {
        .name = "rxx_to_cnot",
        .target_gates = {"cx", "h", "rz"},
        .decomposer = rxx_to_cnot,
        .global_phase = 0.0,
        .priority = 1
    });
    manager.register_rule("ryy", {
        .name = "ryy_to_cnot",
        .target_gates = {"cx", "rx", "rz"},
        .decomposer = ryy_to_cnot,
        .global_phase = 0.0,
        .priority = 1
    });
    manager.register_rule("rzz", {
        .name = "rzz_to_cnot",
        .target_gates = {"cx", "rz"},
        .decomposer = rzz_to_cnot,
        .global_phase = 0.0,
        .priority = 1
    });
    manager.register_rule("iswap", {
        .name = "iswap_to_cnot",
        .target_gates = {"cx", "h", "s"},
        .decomposer = iswap_to_cnot,
        .global_phase = 0.0,
        .priority = 1
    });
}

} // namespace qsteedcpp