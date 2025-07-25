#pragma once

#include <string>
#include <vector>
#include "ftxui/dom/elements.hpp"
#include "circuit/circuit_instruction.h"

namespace qsteedcpp {

class FTXUICircuitDrawer {
private:
    const std::vector<CircuitInstruction>& instructions_;
    int num_qubits_;
    int num_clbits_;
    
    // Helper methods
    ftxui::Element draw_single_qubit_gate(const std::string& label, int qubit);
    ftxui::Element draw_two_qubit_gate(const std::string& label, 
                                       const std::vector<int>& qubits,
                                       const Gate* gate = nullptr);
    ftxui::Element draw_measurement(int qubit, int clbit);
    ftxui::Element draw_barrier(const std::vector<int>& qubits);
    
public:
    FTXUICircuitDrawer(const std::vector<CircuitInstruction>& instructions,
                       int num_qubits, int num_clbits)
        : instructions_(instructions), 
          num_qubits_(num_qubits), 
          num_clbits_(num_clbits) {}
    
    std::string draw();
    std::string draw_canvas();  // Canvas-based drawing method
    std::string draw_grid();    // Grid-based drawing method
};

} // namespace qsteedcpp