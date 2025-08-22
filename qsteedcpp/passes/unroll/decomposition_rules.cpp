#include "decomposition_rules.h"
#include "circuit/circuit_instruction.h"
#include "QuantumCircuit/include/gates/standard_gates.h"
namespace qsteedcpp {
    void initialize_standard_rules(RuleManager& manager) {
        // SWAP 门的规则
        manager.register_rule("swap", {
            .name = "swap_to_cnot",
            .target_gates = {"cx"},  // SWAP 分解为 3 个 CX
            .decomposer = rules::swap_to_cnot,
            .global_phase = 0.0,
            .priority = 1
        });
        manager.register_rule("swap", {
            .name = "swap_to_czh",
            .target_gates = {"cz", "h"},
            .decomposer = rules::swap_to_czh,
            .global_phase = 0.0,
            .priority = 2
        });
        manager.register_rule("swap", {
            .name = "swap_to_czrxry",
            .target_gates = {"cz", "rx", "ry"},
            .decomposer = rules::swap_to_czrxry,
            .global_phase = M_PI,
            .priority = 3
        });
        // H 门的规则
        manager.register_rule("h", {
            .name = "h_to_rz_rx_rz",
            .target_gates = {"rz", "rx"},  // H = RZ(π/2)·RX(π/2)·RZ(π/2)
            .decomposer = rules::h_to_rz_rx_rz,
            .global_phase = M_PI / 2.0,
            .priority = 1
        });
        

        // CNOT 门到 CP 的转换
        manager.register_rule(std::vector<std::string>{"cx", "cnot"}, {
            .name = "cnot_to_cp",
            .target_gates = {"h", "cp"}, // CNOT = H · P(π) · H
            .decomposer = rules::cnot_to_cp,
            .global_phase = 0.0,
            .priority = 2
        });
        // 单比特门到旋转门的转换
        manager.register_rule("x", {
            .name = "x_to_rx",
            .target_gates = {"rx"},  // X = Rx(π)
            .decomposer = rules::x_to_rx,
            .global_phase = M_PI / 2.0,
            .priority = 1
        });
        manager.register_rule("y", {
            .name = "y_to_ry",
            .target_gates = {"ry"},  // Y = Ry(π)
            .decomposer = rules::y_to_ry,
            .global_phase = M_PI / 2.0,
            .priority = 1
        });
        manager.register_rule("z", {
            .name = "z_to_rz",
            .target_gates = {"rz"},  // Z = Rz(π)
            .decomposer = rules::z_to_rz,
            .global_phase = M_PI / 2.0,
            .priority = 1
        });
        manager.register_rule("s", {
            .name = "s_to_rz",
            .target_gates = {"rz"},  // S = Rz(π/2)
            .decomposer = rules::s_to_rz,
            .global_phase = M_PI / 4.0,
            .priority = 1
        });
        manager.register_rule("t", {
            .name = "t_to_rz",
            .target_gates = {"rz"},  // T = Rz(π/4)
            .decomposer = rules::t_to_rz,
            .global_phase = M_PI / 8.0,
            .priority = 1
        });
        manager.register_rule("sdg", {
            .name = "sdg_to_rz",
            .target_gates = {"rz"},  // Sdg = Rz(-π/2)
            .decomposer = rules::sdg_to_rz,
            .global_phase = 7.0 * M_PI / 4.0,
            .priority = 1
        });
        manager.register_rule("tdg", {
            .name = "tdg_to_rz",
            .target_gates = {"rz"},  // Tdg = Rz(-π/4)
            .decomposer = rules::tdg_to_rz,
            .global_phase = 15.0 * M_PI / 8.0,
            .priority = 1
        });
        // CZ 门的规则
        manager.register_rule("cz", {
            .name = "cz_to_cnot",
            .target_gates = {"h", "cx"},  // CZ = H · CNOT · H
            .decomposer = rules::cz_to_cnot,
            .global_phase = 0.0,
            .priority = 2
        });
        // Toffoli 门的规则（ccx 和 toffoli 是同一个门的别名）
        manager.register_rule(std::vector<std::string>{"ccx", "toffoli"}, {
            .name = "toffoli_to_cnot",
            .target_gates = {"cx", "h", "t", "tdg"},  // Toffoli 分解为 6 个 CX + H + T/Tdg
            .decomposer = rules::toffoli_to_cnot,
            .global_phase = 0.0,
            .priority = 1
        });
        // 两量子比特参数化门分解规则
        manager.register_rule("rxx", {
            .name = "rxx_to_cnot",
            .target_gates = {"cx", "h", "rz"},  // RXX = H·H·CX·RZ·CX·H·H
            .decomposer = rules::rxx_to_cnot,
            .global_phase = 0.0,
            .priority = 1
        });
        manager.register_rule("ryy", {
            .name = "ryy_to_cnot",
            .target_gates = {"cx", "rx", "rz"},  // RYY = RX·RX·CX·RZ·CX·RX·RX
            .decomposer = rules::ryy_to_cnot,
            .global_phase = 0.0,
            .priority = 1
        });
        manager.register_rule("rzz", {
            .name = "rzz_to_cnot",
            .target_gates = {"cx", "rz"},  // RZZ = CX·RZ·CX
            .decomposer = rules::rzz_to_cnot,
            .global_phase = 0.0,
            .priority = 1
        });

        // Phase gate to RZ
        manager.register_rule("p", {
            .name = "phase_to_rz",
            .target_gates = {"rz"},  // P(lambda) = RZ(lambda) with global phase
            .decomposer = rules::phase_to_rz,
            // Global phase is lambda/2, handled dynamically in the decomposer
            .global_phase = 0.0, // Placeholder, actual phase is dynamic
            .priority = 1
        });

        // Controlled-Y
        manager.register_rule("cy", {
            .name = "cy_to_cnot",
            .target_gates = {"cx", "s", "sdg"},
            .decomposer = rules::cy_to_cnot,
            .global_phase = 0.0,
            .priority = 1
        });

        // Controlled-RX
        manager.register_rule("crx", {
            .name = "crx_to_cnot",
            .target_gates = {"cx", "ry", "s", "sdg"},
            .decomposer = rules::crx_to_cnot,
            .global_phase = 0.0,
            .priority = 1
        });

        // Controlled-RY
        manager.register_rule("cry", {
            .name = "cry_to_cnot",
            .target_gates = {"cx", "ry"},
            .decomposer = rules::cry_to_cnot,
            .global_phase = 0.0,
            .priority = 1
        });

        // Controlled-RZ
        manager.register_rule("crz", {
            .name = "crz_to_cnot",
            .target_gates = {"cx", "rz"},
            .decomposer = rules::crz_to_cnot,
            .global_phase = 0.0,
            .priority = 1
        });

        // Controlled-Phase
        manager.register_rule("cp", {
            .name = "cp_to_cnot",
            .target_gates = {"cx", "p"},
            .decomposer = rules::cp_to_cnot,
            .global_phase = 0.0,
            .priority = 1
        });

        // Fredkin (CSWAP)
        manager.register_rule(std::vector<std::string>{"fredkin", "cswap"}, {
            .name = "fredkin_to_toffoli",
            .target_gates = {"cx", "ccx"},
            .decomposer = rules::fredkin_to_toffoli,
            .global_phase = 0.0,
            .priority = 1
        });

        // iSWAP
        manager.register_rule("iswap", {
            .name = "iswap_to_cnot",
            .target_gates = {"cx", "h", "s"},
            .decomposer = rules::iswap_to_cnot,
            .global_phase = 0.0,
            .priority = 1
        });

        // RZ to RX and RY
        manager.register_rule("rz", {
            .name = "rz_to_rxry",
            .target_gates = {"rx", "ry"},
            .decomposer = rules::rz_to_rxry,
            .global_phase = 0.0,
            .priority = 2 // Higher priority than rz_to_rz
        });

        // SX and SXdg
        manager.register_rule("sx", {
            .name = "sx_to_rx",
            .target_gates = {"rx"},
            .decomposer = rules::sx_to_rx,
            .global_phase = M_PI / 4.0,
            .priority = 1
        });
        manager.register_rule("sxdg", {
            .name = "sxdg_to_rx",
            .target_gates = {"rx"},
            .decomposer = rules::sxdg_to_rx,
            .global_phase = 7.0 * M_PI / 4.0,
            .priority = 1
        });

        // SY and SYdg
        manager.register_rule("sy", {
            .name = "sy_to_ry",
            .target_gates = {"ry"},
            .decomposer = rules::sy_to_ry,
            .global_phase = M_PI / 4.0,
            .priority = 1
        });
        manager.register_rule("sydg", {
            .name = "sydg_to_ry",
            .target_gates = {"ry"},
            .decomposer = rules::sydg_to_ry,
            .global_phase = 7.0 * M_PI / 4.0,
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
    std::vector<CircuitInstruction> h_to_rz_rx_rz(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        // H = RZ(π/2) · RX(π/2) · RZ(π/2) (up to global phase)
        // Order of application is right to left, so add gates in reverse order of multiplication.
        result.emplace_back(std::make_unique<RZGate>(Parameter(M_PI / 2.0)), qubits);
        result.emplace_back(std::make_unique<RXGate>(Parameter(M_PI / 2.0)), qubits);
        result.emplace_back(std::make_unique<RZGate>(Parameter(M_PI / 2.0)), qubits);
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
    std::vector<CircuitInstruction> sdg_to_rz(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        // Sdg = Rz(-π/2)
        result.emplace_back(std::make_unique<RZGate>(Parameter(-M_PI/2)), qubits);
        if (inst.condition.has_value()) {
            result[0].condition = inst.condition;
        }
        return result;
    }
    std::vector<CircuitInstruction> tdg_to_rz(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        // Tdg = Rz(-π/4)
        result.emplace_back(std::make_unique<RZGate>(Parameter(-M_PI/4)), qubits);
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
    std::vector<CircuitInstruction> rxx_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        auto params = gate->get_parameter_values();
        double theta = params.empty() ? 0.0 : params[0];
        std::vector<CircuitInstruction> result;
        // RXX(theta) = H(q0) · H(q1) · CNOT(q0,q1) · RZ(theta) · CNOT(q0,q1) · H(q0) · H(q1)
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[0], qubits[1]});
        result.emplace_back(std::make_unique<RZGate>(theta), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[0], qubits[1]});
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        return result;
    }
    std::vector<CircuitInstruction> ryy_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        auto params = gate->get_parameter_values();
        double theta = params.empty() ? 0.0 : params[0];
        std::vector<CircuitInstruction> result;
        // RYY(theta) = RX(π/2) · RX(π/2) · CNOT(q0,q1) · RZ(theta) · CNOT(q0,q1) · RX(-π/2) · RX(-π/2)
        double pi_2 = M_PI / 2.0;
        result.emplace_back(std::make_unique<RXGate>(pi_2), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<RXGate>(pi_2), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[0], qubits[1]});
        result.emplace_back(std::make_unique<RZGate>(theta), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[0], qubits[1]});
        result.emplace_back(std::make_unique<RXGate>(-pi_2), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<RXGate>(-pi_2), std::vector<int>{qubits[1]});
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        return result;
    }
    std::vector<CircuitInstruction> rzz_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        auto params = gate->get_parameter_values();
        double theta = params.empty() ? 0.0 : params[0];
        std::vector<CircuitInstruction> result;
        // RZZ(theta) = CNOT(q0,q1) · RZ(theta) · CNOT(q0,q1)
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[0], qubits[1]});
        result.emplace_back(std::make_unique<RZGate>(theta), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[0], qubits[1]});
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        return result;
    }

    std::vector<CircuitInstruction> phase_to_rz(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        auto params = gate->get_parameter_values();
        double lambda = params.empty() ? 0.0 : params[0];

        std::vector<CircuitInstruction> result;
        // P(lambda) = RZ(lambda)
        // NOTE: This decomposition has a global phase of lambda/2, which is currently
        // not handled by the C++ RuleManager framework.
        result.emplace_back(std::make_unique<RZGate>(lambda), qubits);
        if (inst.condition.has_value()) {
            result[0].condition = inst.condition;
        }
        return result;
    }

    std::vector<CircuitInstruction> cy_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        // CY = Sdg(target) · CNOT · S(target)
        result.emplace_back(std::make_unique<SdgGate>(), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        result.emplace_back(std::make_unique<SGate>(), std::vector<int>{qubits[1]});
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        return result;
    }

    std::vector<CircuitInstruction> crx_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        auto params = gate->get_parameter_values();
        double theta = params.empty() ? 0.0 : params[0];
        std::vector<CircuitInstruction> result;
        // CRX(theta) = S(target) · CNOT · RY(-theta/2) · CNOT · RY(theta/2) · Sdg(target)
        result.emplace_back(std::make_unique<SGate>(), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        result.emplace_back(std::make_unique<RYGate>(-theta / 2.0), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        result.emplace_back(std::make_unique<RYGate>(theta / 2.0), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<SdgGate>(), std::vector<int>{qubits[1]});
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        return result;
    }

    std::vector<CircuitInstruction> cry_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        auto params = gate->get_parameter_values();
        double theta = params.empty() ? 0.0 : params[0];
        std::vector<CircuitInstruction> result;
        // CRY(theta) = RY(theta/2) · CNOT · RY(-theta/2) · CNOT
        result.emplace_back(std::make_unique<RYGate>(theta / 2.0), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        result.emplace_back(std::make_unique<RYGate>(-theta / 2.0), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        return result;
    }

    std::vector<CircuitInstruction> crz_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        auto params = gate->get_parameter_values();
        double theta = params.empty() ? 0.0 : params[0];
        std::vector<CircuitInstruction> result;
        // CRZ(theta) = RZ(theta/2) · CNOT · RZ(-theta/2) · CNOT
        result.emplace_back(std::make_unique<RZGate>(theta / 2.0), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        result.emplace_back(std::make_unique<RZGate>(-theta / 2.0), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        return result;
    }

    std::vector<CircuitInstruction> cnot_to_cp(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        // CNOT = H(target) · CP(π) · H(target)
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CPGate>(Parameter(M_PI)), qubits);
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        return result;
    }

    std::vector<CircuitInstruction> cp_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        auto params = gate->get_parameter_values();
        double theta = params.empty() ? 0.0 : params[0];
        std::vector<CircuitInstruction> result;
        // CP(theta) = P(theta/2) · CNOT · P(-theta/2) · CNOT · P(theta/2)
        result.emplace_back(std::make_unique<PhaseGate>(Parameter(theta / 2.0)), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        result.emplace_back(std::make_unique<PhaseGate>(Parameter(-theta / 2.0)), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        result.emplace_back(std::make_unique<PhaseGate>(Parameter(theta / 2.0)), std::vector<int>{qubits[1]});
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        return result;
    }

    std::vector<CircuitInstruction> fredkin_to_toffoli(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        // Fredkin = CNOT(q2, q1) · Toffoli(q0, q1, q2) · CNOT(q2, q1)
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[2], qubits[1]});
        result.emplace_back(std::make_unique<ToffoliGate>(), qubits);
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[2], qubits[1]});
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        return result;
    }

    std::vector<CircuitInstruction> iswap_to_cnot(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        // iSWAP = S(q0) · S(q1) · H(q0) · CNOT(q0, q1) · CNOT(q1, q0) · H(q1)
        result.emplace_back(std::make_unique<SGate>(), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<SGate>(), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<CNOTGate>(), qubits);
        result.emplace_back(std::make_unique<CNOTGate>(), std::vector<int>{qubits[1], qubits[0]});
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        return result;
    }

    std::vector<CircuitInstruction> rz_to_rxry(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        auto params = gate->get_parameter_values();
        double theta = params.empty() ? 0.0 : params[0];
        std::vector<CircuitInstruction> result;
        // RZ(theta) = RX(π/2) · RY(-theta) · RX(-π/2)
        result.emplace_back(std::make_unique<RXGate>(M_PI / 2.0), qubits);
        result.emplace_back(std::make_unique<RYGate>(-theta), qubits);
        result.emplace_back(std::make_unique<RXGate>(-M_PI / 2.0), qubits);
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        return result;
    }

    std::vector<CircuitInstruction> sx_to_rx(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        // SX = RX(π/2)
        result.emplace_back(std::make_unique<RXGate>(M_PI / 2.0), qubits);
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        return result;
    }

    std::vector<CircuitInstruction> sxdg_to_rx(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        // SXdg = RX(-π/2)
        result.emplace_back(std::make_unique<RXGate>(-M_PI / 2.0), qubits);
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        return result;
    }

    std::vector<CircuitInstruction> sy_to_ry(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        // SY = RY(π/2)
        result.emplace_back(std::make_unique<RYGate>(M_PI / 2.0), qubits);
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        return result;
    }

    std::vector<CircuitInstruction> sydg_to_ry(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        // SYdg = RY(-π/2)
        result.emplace_back(std::make_unique<RYGate>(-M_PI / 2.0), qubits);
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        return result;
    }

    std::vector<CircuitInstruction> swap_to_czh(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        // SWAP = H(q0)·CZ·H(q0)·H(q1)·CZ·H(q0)·H(q1)·CZ·H(q0)
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<CZGate>(), qubits);
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CZGate>(), qubits);
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[0]});
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[1]});
        result.emplace_back(std::make_unique<CZGate>(), qubits);
        result.emplace_back(std::make_unique<HGate>(), std::vector<int>{qubits[0]});
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        return result;
    }

    std::vector<CircuitInstruction> swap_to_czrxry(const CircuitInstruction& inst) {
        const auto& qubits = inst.qubits;
        std::vector<CircuitInstruction> result;
        // SWAP = RY(π/2)·RX(π)·CZ·RY(π/2)·RX(π)·RY(π/2)·RX(π)·CZ·RY(π/2)·RX(π)·RY(π/2)·RX(π)·CZ·RY(π/2)·RX(π)
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
        if (inst.condition.has_value()) {
            for (auto& new_inst : result) {
                new_inst.condition = inst.condition;
            }
        }
        return result;
    }

    } // namespace rules
} // namespace qsteedcpp
