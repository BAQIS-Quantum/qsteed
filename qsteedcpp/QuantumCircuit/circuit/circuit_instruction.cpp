#include "circuit_instruction.h"
#include <sstream>
#include <algorithm>
#include <map>

namespace qsteedcpp {

std::optional<int> CircuitInstruction::get_duration() const {
    if (std::holds_alternative<Delay>(operation)) {
        return std::get<Delay>(operation).duration;
    } else if (std::holds_alternative<XYResonance>(operation)) {
        return std::get<XYResonance>(operation).duration;
    }
    return std::nullopt;
}

std::optional<std::string> CircuitInstruction::get_unit() const {
    if (std::holds_alternative<Delay>(operation)) {
        return std::get<Delay>(operation).unit;
    } else if (std::holds_alternative<XYResonance>(operation)) {
        return std::get<XYResonance>(operation).unit;
    }
    return std::nullopt;
}

std::string CircuitInstruction::to_qasm(bool with_para) const {
    std::stringstream ss;

    static const std::map<std::string, std::string> name_to_qasm = {
        {"CNOT", "cx"}, {"CX", "cx"},
        {"Toffoli", "ccx"}, {"CCX", "ccx"},
        {"Phase", "p"},
        {"Sdg", "sdg"},
        {"Tdg", "tdg"},
        {"Fredkin", "cswap"}, {"CSWAP", "cswap"}
    };

    if (is_gate()) {
        const auto& gate = std::get<std::unique_ptr<Gate>>(operation);
        std::string gate_name = name();

        auto it = name_to_qasm.find(gate_name);
        if (it != name_to_qasm.end()) {
            gate_name = it->second;
        } else {
            std::transform(gate_name.begin(), gate_name.end(), gate_name.begin(),
                            ::tolower);
        }
        
        ss << gate_name;

        if (gate->has_parameters()) {
            const auto& params = gate->get_parameter_expressions();
            ss << "(";
            for (size_t i = 0; i < params.size(); ++i) {
                ss << params[i].to_string(with_para);
                if (i < params.size() - 1) {
                    ss << ",";
                }
            }
            ss << ")";
        }

        ss << " ";
        for (size_t i = 0; i < qubits.size(); ++i) {
            ss << "q[" << qubits[i] << "]";
            if (i < qubits.size() - 1) {
                ss << ",";
            }
        }
        ss << ";";

    } else if (is_measurement()) {
        for (size_t i = 0; i < qubits.size(); ++i) {
            ss << "measure q[" << qubits[i] << "] -> c[" << clbits[i] << "];";
            if (i < qubits.size() - 1) {
                ss << "\n";
            }
        }
    } else if (is_barrier()) {
        ss << "barrier ";
        for (size_t i = 0; i < qubits.size(); ++i) {
            ss << "q[" << qubits[i] << "]";
            if (i < qubits.size() - 1) {
                ss << ",";
            }
        }
        ss << ";";
    } else if (is_reset()) {
        ss << "reset q[" << qubits[0] << "];";
    }
    // Note: Delay and XYResonance are not part of OpenQASM 2.0 and are ignored.

    return ss.str();
}

} // namespace qsteedcpp
