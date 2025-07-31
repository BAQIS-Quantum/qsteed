#pragma once
#include <cmath>
#include "rule_manager.h"
#include "circuit/circuit_instruction.h"

namespace qsteedcpp {

void initialize_standard_rules(RuleManager& manager);

namespace rules {

    // SWAP -> CNOT 分解
    std::vector<CircuitInstruction> swap_to_cnot(const CircuitInstruction& inst);

    // H -> RZ, RX 分解
    std::vector<CircuitInstruction> h_to_rzrx(const CircuitInstruction& inst);

    // CNOT -> CZ + H 分解
    std::vector<CircuitInstruction> cnot_to_czh(const CircuitInstruction& inst);

    // S -> RZ 分解
    std::vector<CircuitInstruction> s_to_rz(const CircuitInstruction& inst);

    // T -> RZ 分解
    std::vector<CircuitInstruction> t_to_rz(const CircuitInstruction& inst);

    // X -> RX 分解
    std::vector<CircuitInstruction> x_to_rx(const CircuitInstruction& inst);

    // Y -> RY 分解
    std::vector<CircuitInstruction> y_to_ry(const CircuitInstruction& inst);

    // Z -> RZ 分解
    std::vector<CircuitInstruction> z_to_rz(const CircuitInstruction& inst);

    // CZ -> CNOT + H 分解
    std::vector<CircuitInstruction> cz_to_cnot(const CircuitInstruction& inst);

    // Toffoli -> CNOT + T + H 分解
    std::vector<CircuitInstruction> toffoli_to_cnot(const CircuitInstruction& inst);

} // namespace rules

} // namespace qsteedcpp