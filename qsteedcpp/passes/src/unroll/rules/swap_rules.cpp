#include "unroll/rules/swap_rules.h"
#include "unroll/rule_manager.h"
#include "circuit/circuit_instruction.h"
#include "QuantumCircuit/include/gates/standard_gates.h"

namespace qsteedcpp {

namespace { // Anonymous namespace for local helpers and rule implementations

    void apply_condition(const CircuitInstruction& source, std::vector<CircuitInstruction>& targets) {
        if (source.condition.has_value()) {
            for (auto& target : targets) {
                target.condition = source.condition;
            }
        }
    }

    std::vector<CircuitInstruction> swap_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[0], qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[1], qubits[0]});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[0], qubits[1]});
        apply_condition(inst, result);
        return result;
    }

    std::vector<CircuitInstruction> swap_to_czh(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<CZGate>(), qubits);
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CZGate>(), qubits);
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CZGate>(), qubits);
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[0]});
        apply_condition(inst, result);
        return result;
    }

    std::vector<CircuitInstruction> swap_to_czrxry(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<RYGate>(M_PI / 2.0), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<RXGate>(M_PI), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<CZGate>(), qubits);
        result.emplace_back(std::make_unique<RYGate>(M_PI / 2.0), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<RXGate>(M_PI), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<RYGate>(M_PI / 2.0), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<RXGate>(M_PI), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CZGate>(), qubits);
        result.emplace_back(std::make_unique<RYGate>(M_PI / 2.0), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<RXGate>(M_PI), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<RYGate>(M_PI / 2.0), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<RXGate>(M_PI), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CZGate>(), qubits);
        result.emplace_back(std::make_unique<RYGate>(M_PI / 2.0), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<RXGate>(M_PI), std::vector<int>{qubits[0]});
        apply_condition(inst, result);
        return result;
    }

} // anonymous namespace

void initialize_swap_rules(RuleManager& manager) {
    // SWAP 门的规则
    manager.register_rule("swap", {
        .name = "swap_to_cnot",
        .target_gates = {"cx"},  // SWAP 分解为 3 个 CX
        .decomposer = swap_to_cnot,
        .global_phase = 0.0,
        .priority = 1
    });
    manager.register_rule("swap", {
        .name = "swap_to_czh",
        .target_gates = {"cz", "h"},
        .decomposer = swap_to_czh,
        .global_phase = 0.0,
        .priority = 2
    });
    manager.register_rule("swap", {
        .name = "swap_to_czrxry",
        .target_gates = {"cz", "rx", "ry"},
        .decomposer = swap_to_czrxry,
        .global_phase = M_PI,
        .priority = 3
    });
}

} // namespace qsteedcpp