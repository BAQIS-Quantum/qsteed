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
    std::vector<CircuitInstruction> h_to_rz_rx_rz(const CircuitInstruction& inst);

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

    // cxx(Toffoli) -> CNOT + T + H 分解
    std::vector<CircuitInstruction> toffoli_to_cnot(const CircuitInstruction& inst);
    
    // Sdg -> RZ 分解
    std::vector<CircuitInstruction> sdg_to_rz(const CircuitInstruction& inst);
    
    // Tdg -> RZ 分解
    std::vector<CircuitInstruction> tdg_to_rz(const CircuitInstruction& inst);
    
    // RXX -> CNOT + H + RZ 分解
    std::vector<CircuitInstruction> rxx_to_cnot(const CircuitInstruction& inst);
    
    // RYY -> CNOT + RX + RZ 分解
    std::vector<CircuitInstruction> ryy_to_cnot(const CircuitInstruction& inst);
    
    // RZZ -> CNOT + RZ 分解
    std::vector<CircuitInstruction> rzz_to_cnot(const CircuitInstruction& inst);

    // P -> RZ 分解
    std::vector<CircuitInstruction> phase_to_rz(const CircuitInstruction& inst);

    // CY -> CNOT + S 分解
    std::vector<CircuitInstruction> cy_to_cnot(const CircuitInstruction& inst);

    // CRX -> CNOT + RY + S 分解
    std::vector<CircuitInstruction> crx_to_cnot(const CircuitInstruction& inst);

    // CRY -> CNOT + RY 分解
    std::vector<CircuitInstruction> cry_to_cnot(const CircuitInstruction& inst);

    // CRZ -> CNOT + RZ 分解
    std::vector<CircuitInstruction> crz_to_cnot(const CircuitInstruction& inst);

    // CNOT -> CP + H 分解
    std::vector<CircuitInstruction> cnot_to_cp(const CircuitInstruction& inst);

    // CP -> CNOT + P 分解
    std::vector<CircuitInstruction> cp_to_cnot(const CircuitInstruction& inst);

    // Fredkin -> Toffoli + CNOT 分解
    std::vector<CircuitInstruction> fredkin_to_toffoli(const CircuitInstruction& inst);

    // iSWAP -> CNOT + H + S 分解
    std::vector<CircuitInstruction> iswap_to_cnot(const CircuitInstruction& inst);

    // RZ -> RX + RY 分解
    std::vector<CircuitInstruction> rz_to_rxry(const CircuitInstruction& inst);

    // SX -> RX 分解
    std::vector<CircuitInstruction> sx_to_rx(const CircuitInstruction& inst);

    // SXdg -> RX 分解
    std::vector<CircuitInstruction> sxdg_to_rx(const CircuitInstruction& inst);

    // SY -> RY 分解
    std::vector<CircuitInstruction> sy_to_ry(const CircuitInstruction& inst);

    // SYdg -> RY 分解
    std::vector<CircuitInstruction> sydg_to_ry(const CircuitInstruction& inst);

    // SWAP -> CZ + H 分解
    std::vector<CircuitInstruction> swap_to_czh(const CircuitInstruction& inst);

    // SWAP -> CZ + RX + RY 分解
    std::vector<CircuitInstruction> swap_to_czrxry(const CircuitInstruction& inst);

} // namespace rules

} // namespace qsteedcpp