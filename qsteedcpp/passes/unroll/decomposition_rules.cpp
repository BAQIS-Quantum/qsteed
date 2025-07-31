#include "decomposition_rules.h"
#include "circuit/circuit_instruction.h"
#include "gates/standard_gates.h"

namespace qsteedcpp {

    void initialize_standard_rules(RuleManager& manager) {
        // SWAP 门的规则
        manager.register_rule("swap", {
            .name = "swap_to_cnot",
            .target_gates = {"cx"},
            .decomposer = rules::swap_to_cnot,
            .global_phase = 0.0,
            .priority = 1
        });

        manager.register_rule("swap", {
            .name = "swap_to_cnot",
            .target_gates = {"cnot"},
            .decomposer = rules::swap_to_cnot,
            .global_phase = 0.0,
            .priority = 1
        });
        
        // H 门的规则
        manager.register_rule("h", {
            .name = "h_to_rzrx",
            .target_gates = {"rz", "rx"},
            .decomposer = rules::h_to_rzrx,
            .global_phase = 0.0,
            .priority = 1
        });
        
        // CNOT 门到 CZ 的转换
        manager.register_rule("cx", {
            .name = "cnot_to_czh",
            .target_gates = {"cz", "h"},
            .decomposer = rules::cnot_to_czh,
            .global_phase = 0.0,
            .priority = 2
        });

        manager.register_rule("cnot", {
            .name = "cnot_to_czh",
            .target_gates = {"cz", "h"},
            .decomposer = rules::cnot_to_czh,
            .global_phase = 0.0,
            .priority = 2
        });
        
        // 单比特门到旋转门的转换
        manager.register_rule("x", {
            .name = "x_to_rx",
            .target_gates = {"rx"},
            .decomposer = rules::x_to_rx,
            .global_phase = 0.0,
            .priority = 1
        });
        
        manager.register_rule("y", {
            .name = "y_to_ry",
            .target_gates = {"ry"},
            .decomposer = rules::y_to_ry,
            .global_phase = 0.0,
            .priority = 1
        });
        
        manager.register_rule("z", {
            .name = "z_to_rz",
            .target_gates = {"rz"},
            .decomposer = rules::z_to_rz,
            .global_phase = 0.0,
            .priority = 1
        });
        
        manager.register_rule("s", {
            .name = "s_to_rz",
            .target_gates = {"rz"},
            .decomposer = rules::s_to_rz,
            .global_phase = 0.0,
            .priority = 1
        });
        
        manager.register_rule("t", {
            .name = "t_to_rz",
            .target_gates = {"rz"},
            .decomposer = rules::t_to_rz,
            .global_phase = 0.0,
            .priority = 1
        });
        
        // CZ 门的规则
        manager.register_rule("cz", {
            .name = "cz_to_cnot",
            .target_gates = {"cx", "h"},
            .decomposer = rules::cz_to_cnot,
            .global_phase = 0.0,
            .priority = 2
        });
        
        // Toffoli 门的规则
        manager.register_rule("ccx", {
            .name = "toffoli_to_cnot",
            .target_gates = {"cx", "h", "t", "tdg"},
            .decomposer = rules::toffoli_to_cnot,
            .global_phase = 0.0,
            .priority = 1
        });
    }

    namespace rules {

    std::vector<CircuitInstruction> swap_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        
        // SWAP = CNOT(a,b) · CNOT(b,a) · CNOT(a,b)
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[0], qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[1], qubits[0]});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[0], qubits[1]});
        
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        
        return result;
    }

    std::vector<CircuitInstruction> h_to_rzrx(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        
        // H = Rz(π/2) · Rx(π/2) · Rz(π/2)
        result.emplace_back(std::make_unique<RZGate>(Parameter(M_PI/2)), qubits);
        result.emplace_back(std::make_unique<RXGate>(Parameter(M_PI/2)), qubits);
        result.emplace_back(std::make_unique<RZGate>(Parameter(M_PI/2)), qubits);
        
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        
        return result;
    }

    std::vector<CircuitInstruction> cnot_to_czh(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        
        // CNOT = H(target) · CZ · H(target)
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CZGate>(), qubits);
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        
        return result;
    }

    std::vector<CircuitInstruction> s_to_rz(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        
        // S = Rz(π/2)
        result.emplace_back(std::make_unique<RZGate>(Parameter(M_PI/2)), qubits);
        
        if (inst.condition.has_value()) {
            result[0].condition = inst.condition;
        }
        
        return result;
    }

    std::vector<CircuitInstruction> t_to_rz(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        
        // T = Rz(π/4)
        result.emplace_back(std::make_unique<RZGate>(Parameter(M_PI/4)), qubits);
        
        if (inst.condition.has_value()) {
            result[0].condition = inst.condition;
        }
        
        return result;
    }

    std::vector<CircuitInstruction> x_to_rx(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        
        // X = Rx(π)
        result.emplace_back(std::make_unique<RXGate>(Parameter(M_PI)), qubits);
        
        if (inst.condition.has_value()) {
            result[0].condition = inst.condition;
        }
        
        return result;
    }

    std::vector<CircuitInstruction> y_to_ry(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        
        // Y = Ry(π)
        result.emplace_back(std::make_unique<RYGate>(Parameter(M_PI)), qubits);
        
        if (inst.condition.has_value()) {
            result[0].condition = inst.condition;
        }
        
        return result;
    }

    std::vector<CircuitInstruction> z_to_rz(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        
        // Z = Rz(π)
        result.emplace_back(std::make_unique<RZGate>(Parameter(M_PI)), qubits);
        
        if (inst.condition.has_value()) {
            result[0].condition = inst.condition;
        }
        
        return result;
    }

    std::vector<CircuitInstruction> cz_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        
        // CZ = H(target) · CNOT · H(target)
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        
        return result;
    }

    std::vector<CircuitInstruction> toffoli_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        int ctrl1 = qubits[0];
        int ctrl2 = qubits[1];
        int targ = qubits[2];
        
        std::vector<CircuitInstruction> result;
        
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{targ});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl2, targ});
        result.emplace_back(std::make_unique<TdgGate>(), std::vector<int>{targ});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl1, targ});
        result.emplace_back(std::make_unique<TGate>(), std::vector<int>{targ});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl2, targ});
        result.emplace_back(std::make_unique<TGate>(), std::vector<int>{ctrl2});
        result.emplace_back(std::make_unique<TdgGate>(), std::vector<int>{targ});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl1, targ});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl1, ctrl2});
        result.emplace_back(std::make_unique<TGate>(), std::vector<int>{targ});
        result.emplace_back(std::make_unique<TGate>(), std::vector<int>{ctrl1});
        result.emplace_back(std::make_unique<TdgGate>(), std::vector<int>{ctrl2});
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{targ});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{ctrl1, ctrl2});
        
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        
        return result;
    }

    } // namespace rules

} // namespace qsteedcpp