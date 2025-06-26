#pragma once
#include "AST/ast.hpp"
#include "AST/visitor.hpp"
#include "SA/analyzer.hpp"
#include "parser.h"
#include "dag.h"
#include <unordered_set>
#include "gates/standard_gates.h"

namespace qarser {

    class AstToDagConverter : public BaseVisitor {
    public:
        DAGCircuit dag;
        std::unordered_map<std::string, int> qreg;
        std::unordered_map<std::string, int> creg;

        // 支持的门
        bool check_supported_gate = false;
        static inline const std::unordered_set<std::string> supported_gates = []() {
            std::unordered_set<std::string> gates;
            
            // 添加所有标准门
            // gates.insert(::qsteedcpp::IdGate::NAME);        // "ID"
            // gates.insert(::qsteedcpp::PauliXGate::NAME);    // "X"
            // gates.insert(::qsteedcpp::PauliYGate::NAME);    // "Y"
            // gates.insert(::qsteedcpp::PauliZGate::NAME);    // "Z"
            // gates.insert(::qsteedcpp::HadamardGate::NAME);  // "H"
            // gates.insert(::qsteedcpp::SGate::NAME);         // "S"
            // gates.insert(::qsteedcpp::SdgGate::NAME);       // "Sdg"
            // gates.insert(::qsteedcpp::TGate::NAME);         // "T"
            // gates.insert(::qsteedcpp::TdgGate::NAME);       // "Tdg"
            // gates.insert(::qsteedcpp::SXGate::NAME);        // "SX"
            // gates.insert(::qsteedcpp::SXdgGate::NAME);      // "SXdg"
            // gates.insert(::qsteedcpp::SYGate::NAME);        // "SY"
            // gates.insert(::qsteedcpp::SYdgGate::NAME);      // "SYdg"
            // gates.insert(::qsteedcpp::WGate::NAME);         // "W"
            // gates.insert(::qsteedcpp::SWGate::NAME);        // "SW"
            // gates.insert(::qsteedcpp::SWdgGate::NAME);      // "SWdg"
            // gates.insert(::qsteedcpp::CNOTGate::NAME);      // "CNOT"
            // gates.insert(::qsteedcpp::CZGate::NAME);        // "CZ"
            // gates.insert(::qsteedcpp::SWAPGate::NAME);      // "SWAP"
            // gates.insert(::qsteedcpp::ISwapGate::NAME);     // "ISWAP"
            // gates.insert(::qsteedcpp::ToffoliGate::NAME);   // "CCNOT"
            
            // 添加参数化门
            // gates.insert(::qsteedcpp::RXGate::NAME);        // "RX"
            // gates.insert(::qsteedcpp::RYGate::NAME);        // "RY"
            // gates.insert(::qsteedcpp::RZGate::NAME);        // "RZ"
            // gates.insert(::qsteedcpp::U3Gate::NAME);        // "U3"
            // gates.insert(::qsteedcpp::PhaseGate::NAME);     // "P"
            // gates.insert(::qsteedcpp::RXXGate::NAME);       // "RXX"
            // gates.insert(::qsteedcpp::RYYGate::NAME);       // "RYY"
            // gates.insert(::qsteedcpp::RZZGate::NAME);       // "RZZ"
            
            // // 添加新的受控门
            // gates.insert(::qsteedcpp::CYGate::NAME);        // "CY"
            // gates.insert(::qsteedcpp::CSGate::NAME);        // "CS"
            // gates.insert(::qsteedcpp::CTGate::NAME);        // "CT"
            // gates.insert(::qsteedcpp::CRXGate::NAME);       // "CRX"
            // gates.insert(::qsteedcpp::CRYGate::NAME);       // "CRY"
            // gates.insert(::qsteedcpp::CRZGate::NAME);       // "CRZ"
            
            // // 添加多控制门
            // gates.insert(::qsteedcpp::MCXGate::NAME);       // "MCX"
            // gates.insert(::qsteedcpp::MCYGate::NAME);       // "MCY"
            // gates.insert(::qsteedcpp::MCZGate::NAME);       // "MCZ"
            
            return gates;
        }();

    public:
        void visit(Program& program) {
            dag = DAGCircuit();
            last_qubit_node.clear();
            
            for (auto& stmt : program.statements) {
                stmt->accept(*this);
            }
        }

        void visit(QRegister& quantum_register) {
            qreg[quantum_register.name] = quantum_register.size;
        }

        void visit(CRegister& classical_register) {
            creg[classical_register.name] = classical_register.size;
        }

        void visit(Gate& gate) {
            // 检查门是否被支持
            if (check_supported_gate) {
                if (supported_gates.find(gate.name) == supported_gates.end()) {
                    std::string error_msg = "Unsupported gate: " + gate.name + "\nSupported gates are:\n";
                    for (const auto& supported_gate : supported_gates) {
                    error_msg += "  - " + supported_gate + "\n";
                    }
                    throw std::runtime_error(error_msg);
                }
            }

            std::vector<qubit_t> qubit_pos = get_gate_pos(gate);
            if (qubit_pos.empty()) {
                throw std::runtime_error("No qubits found for gate: " + gate.name);
            }

            InstructionNode node(gate.name, qubit_pos);
            dag.add_instruction_node_end(node);
        }

    private:
        std::vector<int> last_qubit_node;
        std::unordered_map<std::string, int> qubit_offset;

        void precompute_offsets() {
            int offset = 0;
            for (const auto& [name, size] : this->qreg) {
                qubit_offset[name] = offset;
                offset += size;
            }
        }

        int get_qubit_global_pos(const std::string& qreg_name, int index) {
            if (qubit_offset.empty() && !qreg.empty()) {
                precompute_offsets();
            }
            
            auto it = qubit_offset.find(qreg_name);
            if (it == qubit_offset.end()) {
                return -1;
            }
            return it->second + index;
        }

        std::vector<qubit_t> get_gate_pos(const Gate& gate) {
            std::vector<qubit_t> qubit_pos;
            for (const auto& qubit : gate.qubits) {
                auto it = qreg.find(qubit.name);
                if (it == qreg.end()) {
                    throw std::runtime_error("Qubit register not found: " + qubit.name);
                }

                if (qubit.isRefWholeRegister()) {
                    for (int i = 0; i < it->second; ++i) {
                        qubit_pos.push_back(get_qubit_global_pos(qubit.name, i));
                    }
                } else {
                    qubit_pos.push_back(get_qubit_global_pos(qubit.name, qubit.index));
                }
            }
            return qubit_pos;
        }
    };

} // namespace qarser