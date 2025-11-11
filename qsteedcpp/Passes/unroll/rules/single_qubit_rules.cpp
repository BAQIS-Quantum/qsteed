#include "unroll/rules/single_qubit_rules.h"
#include "unroll/rule_manager.h"
#include "unroll/decomposition_rules.h"
#include "circuit/circuit_instruction.h"
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

    std::vector<CircuitInstruction> h_to_rz_rx_rz(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<RZGate>(Expr(M_PI / 2.0)), qubits);
        result.emplace_back(std::make_unique<RXGate>(Expr(M_PI / 2.0)), qubits);
        result.emplace_back(std::make_unique<RZGate>(Expr(M_PI / 2.0)), qubits);
        apply_condition(inst, result);
        return result;
    }

    std::vector<CircuitInstruction> x_to_rx(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<RXGate>(Expr(M_PI)), qubits);
        if (inst.condition.has_value()) { result[0].condition = inst.condition; }
        return result;
    }

    std::vector<CircuitInstruction> y_to_ry(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<RYGate>(Expr(M_PI)), qubits);
        if (inst.condition.has_value()) { result[0].condition = inst.condition; }
        return result;
    }

    std::vector<CircuitInstruction> z_to_rz(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<RZGate>(Expr(M_PI)), qubits);
        if (inst.condition.has_value()) { result[0].condition = inst.condition; }
        return result;
    }

    std::vector<CircuitInstruction> s_to_rz(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<RZGate>(Expr(M_PI/2)), qubits);
        if (inst.condition.has_value()) { result[0].condition = inst.condition; }
        return result;
    }

    std::vector<CircuitInstruction> t_to_rz(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<RZGate>(Expr(M_PI/4)), qubits);
        if (inst.condition.has_value()) { result[0].condition = inst.condition; }
        return result;
    }

    std::vector<CircuitInstruction> sdg_to_rz(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<RZGate>(Expr(-M_PI/2)), qubits);
        if (inst.condition.has_value()) { result[0].condition = inst.condition; }
        return result;
    }

    std::vector<CircuitInstruction> tdg_to_rz(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<RZGate>(Expr(-M_PI/4)), qubits);
        if (inst.condition.has_value()) { result[0].condition = inst.condition; }
        return result;
    }

    std::vector<CircuitInstruction> phase_to_rz(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        // 获取 Expr 参数（值语义，自动拷贝）
        const auto& exprs = gate->get_parameter_expressions();
        const Expr& lambda = exprs.empty() ? Expr(0.0) : exprs[0];
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<RZGate>(lambda), qubits);
        if (inst.condition.has_value()) { result[0].condition = inst.condition; }
        return result;
    }

    std::vector<CircuitInstruction> rz_to_rxry(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        // 获取 Expr 参数
        const auto& exprs = gate->get_parameter_expressions();
        const Expr& theta = exprs.empty() ? Expr(0.0) : exprs[0];
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<RXGate>(Expr(M_PI / 2.0)), qubits);
        result.emplace_back(std::make_unique<RYGate>(-theta), qubits);  // Expr 支持取负
        result.emplace_back(std::make_unique<RXGate>(Expr(-M_PI / 2.0)), qubits);
        apply_condition(inst, result);
        return result;
    }

    std::vector<CircuitInstruction> sx_to_rx(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<RXGate>(Expr(M_PI / 2.0)), qubits);
        if (inst.condition.has_value()) { result[0].condition = inst.condition; }
        return result;
    }

    std::vector<CircuitInstruction> sxdg_to_rx(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<RXGate>(Expr(-M_PI / 2.0)), qubits);
        if (inst.condition.has_value()) { result[0].condition = inst.condition; }
        return result;
    }

    std::vector<CircuitInstruction> sy_to_ry(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<RYGate>(Expr(M_PI / 2.0)), qubits);
        if (inst.condition.has_value()) { result[0].condition = inst.condition; }
        return result;
    }

    std::vector<CircuitInstruction> sydg_to_ry(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        result.emplace_back(std::make_unique<RYGate>(Expr(-M_PI / 2.0)), qubits);
        if (inst.condition.has_value()) { result[0].condition = inst.condition; }
        return result;
    }

} // anonymous namespace

void initialize_single_qubit_rules(RuleManager& manager) {
    manager.register_rule("h", {
        .name = "h_to_rz_rx_rz",
        .target_gates = {"rz", "rx"}, 
        .decomposer = h_to_rz_rx_rz,
        .global_phase = M_PI / 2.0,
        .priority = 1
    });
    manager.register_rule("x", {
        .name = "x_to_rx",
        .target_gates = {"rx"},
        .decomposer = x_to_rx,
        .global_phase = M_PI / 2.0,
        .priority = 1
    });
    manager.register_rule("y", {
        .name = "y_to_ry",
        .target_gates = {"ry"},
        .decomposer = y_to_ry,
        .global_phase = M_PI / 2.0,
        .priority = 1
    });
    manager.register_rule("z", {
        .name = "z_to_rz",
        .target_gates = {"rz"},
        .decomposer = z_to_rz,
        .global_phase = M_PI / 2.0,
        .priority = 1
    });
    manager.register_rule("s", {
        .name = "s_to_rz",
        .target_gates = {"rz"},
        .decomposer = s_to_rz,
        .global_phase = M_PI / 4.0,
        .priority = 1
    });
    manager.register_rule("t", {
        .name = "t_to_rz",
        .target_gates = {"rz"},
        .decomposer = t_to_rz,
        .global_phase = M_PI / 8.0,
        .priority = 1
    });
    manager.register_rule("sdg", {
        .name = "sdg_to_rz",
        .target_gates = {"rz"},
        .decomposer = sdg_to_rz,
        .global_phase = 7.0 * M_PI / 4.0,
        .priority = 1
    });
    manager.register_rule("tdg", {
        .name = "tdg_to_rz",
        .target_gates = {"rz"},
        .decomposer = tdg_to_rz,
        .global_phase = 15.0 * M_PI / 8.0,
        .priority = 1
    });
    manager.register_rule("p", {
        .name = "phase_to_rz",
        .target_gates = {"rz"},
        .decomposer = phase_to_rz,
        .global_phase = 0.0, 
        .priority = 1
    });
    manager.register_rule("rz", {
        .name = "rz_to_rxry",
        .target_gates = {"rx", "ry"},
        .decomposer = rz_to_rxry,
        .global_phase = 0.0,
        .priority = 2
    });
    manager.register_rule("sx", {
        .name = "sx_to_rx",
        .target_gates = {"rx"},
        .decomposer = sx_to_rx,
        .global_phase = M_PI / 4.0,
        .priority = 1
    });
    manager.register_rule("sxdg", {
        .name = "sxdg_to_rx",
        .target_gates = {"rx"},
        .decomposer = sxdg_to_rx,
        .global_phase = 7.0 * M_PI / 4.0,
        .priority = 1
    });
    manager.register_rule("sy", {
        .name = "sy_to_ry",
        .target_gates = {"ry"},
        .decomposer = sy_to_ry,
        .global_phase = M_PI / 4.0,
        .priority = 1
    });
    manager.register_rule("sydg", {
        .name = "sydg_to_ry",
        .target_gates = {"ry"},
        .decomposer = sydg_to_ry,
        .global_phase = 7.0 * M_PI / 4.0,
        .priority = 1
    });
}

} // namespace qsteedcpp