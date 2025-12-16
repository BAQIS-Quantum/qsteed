#pragma once
#include <map>
#include <string>
#include <stdexcept>
#include <memory>
#include <algorithm>
#include "Qarser/AST/visitor.hpp"
#include "Qarser/AST/gate.hpp"
#include "QuantumCircuit/circuit/quantum_circuit.h"
#include "QuantumCircuit/circuit/instruction_factory.h"
#include "QuantumCircuit/expression/expr.h"

namespace qsteedcpp {
namespace qarser {

    class QasmToCircuit : public AstVisitor {
    private:
        std::unique_ptr<QuantumCircuit> qc;
        std::map<std::string, int> qubit_mapping;
        std::map<std::string, int> clbit_mapping;
        int total_qubits = 0;
        int total_clbits = 0;

    public:
        QasmToCircuit() = default;

        std::unique_ptr<QuantumCircuit> convert(Program& program) {
            // First pass: collect register information
            for (const auto& stmt : program.statements) {
                if (stmt->kind() == Statement::Kind::QREG) {
                    stmt->accept(*this);
                } else if (stmt->kind() == Statement::Kind::CREG) {
                    stmt->accept(*this);
                }
            }

            // Create quantum circuit
            qc = std::make_unique<QuantumCircuit>(total_qubits, total_clbits);

            // Second pass: process gates and operations
            for (const auto& stmt : program.statements) {
                if (stmt->kind() == Statement::Kind::GATE ||
                    stmt->kind() == Statement::Kind::MEASURE ||
                    stmt->kind() == Statement::Kind::BARRIER) {
                    stmt->accept(*this);
                }
            }

            return std::move(qc);
        }

        void visit(Program& node) override {}

        void visit(Include& node) override {}

        void visit(QRegister& node) override {
            qubit_mapping[node.name] = total_qubits;
            total_qubits += node.size;
        }

        void visit(CRegister& node) override {
            clbit_mapping[node.name] = total_clbits;
            total_clbits += node.size;
        }

        void visit(Gate& node) override {
            auto qubits = expand_register_refs(node.qubits);
            std::string gate_name_lower = node.name;
            std::transform(gate_name_lower.begin(), gate_name_lower.end(),
                          gate_name_lower.begin(), ::tolower);

            // node.params is already std::vector<Expr>, use directly
            CircuitInstruction inst = create_gate_instruction(gate_name_lower, node.params, qubits);
            qc->append(inst);
        }

        void visit(Measure& node) override {
            auto qubits = expand_register_refs(node.qubits);
            auto clbits = expand_register_refs_classical(node.cbits);

            if (qubits.size() != clbits.size()) {
                throw std::runtime_error("Measure: qubit and clbit count mismatch");
            }

            qc->measure(qubits, clbits);
        }

        void visit(Barrier& node) override {
            auto qubits = expand_register_refs(node.qubits);
            if (qubits.empty()) {
                qc->barrier();
            } else {
                qc->barrier(qubits);
            }
        }

        void visit(Reset& node) override {
            // TODO: Implement Reset
        }

        void visit(GateDef& node) override {
            // Custom gate definitions not yet supported
        }

    private:
        std::vector<int> expand_register_refs(const std::vector<RegisterRef>& refs) {
            std::vector<int> result;
            for (const auto& ref : refs) {
                auto it = qubit_mapping.find(ref.name);
                if (it == qubit_mapping.end()) {
                    throw std::runtime_error("Unknown quantum register: " + ref.name);
                }

                if (ref.isRefWholeRegister()) {
                    throw std::runtime_error("Whole register expansion not yet supported");
                } else {
                    result.push_back(it->second + ref.index);
                }
            }
            return result;
        }

        std::vector<int> expand_register_refs_classical(const std::vector<RegisterRef>& refs) {
            std::vector<int> result;
            for (const auto& ref : refs) {
                auto it = clbit_mapping.find(ref.name);
                if (it == clbit_mapping.end()) {
                    throw std::runtime_error("Unknown classical register: " + ref.name);
                }

                if (ref.isRefWholeRegister()) {
                    throw std::runtime_error("Whole register expansion not yet supported");
                } else {
                    result.push_back(it->second + ref.index);
                }
            }
            return result;
        }

        CircuitInstruction create_gate_instruction(const std::string& gate_name,
                                                   const std::vector<Expr>& params,
                                                   const std::vector<int>& qubits) {
            // Single-qubit non-parametric gates
            if (gate_name == "h" && params.empty() && qubits.size() == 1) {
                return H(qubits[0]);
            }
            if (gate_name == "x" && params.empty() && qubits.size() == 1) {
                return X(qubits[0]);
            }
            if (gate_name == "y" && params.empty() && qubits.size() == 1) {
                return Y(qubits[0]);
            }
            if (gate_name == "z" && params.empty() && qubits.size() == 1) {
                return Z(qubits[0]);
            }
            if (gate_name == "s" && params.empty() && qubits.size() == 1) {
                return S(qubits[0]);
            }
            if (gate_name == "sdg" && params.empty() && qubits.size() == 1) {
                return Sdg(qubits[0]);
            }
            if (gate_name == "t" && params.empty() && qubits.size() == 1) {
                return T(qubits[0]);
            }
            if (gate_name == "tdg" && params.empty() && qubits.size() == 1) {
                return Tdg(qubits[0]);
            }
            if (gate_name == "id" && params.empty() && qubits.size() == 1) {
                return Id(qubits[0]);
            }

            // Single-qubit parametric gates
            if (gate_name == "rx" && params.size() == 1 && qubits.size() == 1) {
                return RX(params[0], qubits[0]);
            }
            if (gate_name == "ry" && params.size() == 1 && qubits.size() == 1) {
                return RY(params[0], qubits[0]);
            }
            if (gate_name == "rz" && params.size() == 1 && qubits.size() == 1) {
                return RZ(params[0], qubits[0]);
            }
            if ((gate_name == "p" || gate_name == "u1") && params.size() == 1 && qubits.size() == 1) {
                return Phase(params[0], qubits[0]);
            }
            if ((gate_name == "u3" || gate_name == "u") && params.size() == 3 && qubits.size() == 1) {
                return U3(params[0], params[1], params[2], qubits[0]);
            }
            if (gate_name == "u2" && params.size() == 2 && qubits.size() == 1) {
                // u2(phi,lambda) = u3(pi/2, phi, lambda)
                return U3(Expr(M_PI/2), params[0], params[1], qubits[0]);
            }

            // Two-qubit non-parametric gates
            if ((gate_name == "cx" || gate_name == "cnot") && params.empty() && qubits.size() == 2) {
                return CX(qubits[0], qubits[1]);
            }
            if (gate_name == "cz" && params.empty() && qubits.size() == 2) {
                return CZ(qubits[0], qubits[1]);
            }
            if (gate_name == "cy" && params.empty() && qubits.size() == 2) {
                return CY(qubits[0], qubits[1]);
            }
            if (gate_name == "swap" && params.empty() && qubits.size() == 2) {
                return Swap(qubits[0], qubits[1]);
            }
            if (gate_name == "iswap" && params.empty() && qubits.size() == 2) {
                return ISwap(qubits[0], qubits[1]);
            }

            // Two-qubit parametric gates
            if (gate_name == "rxx" && params.size() == 1 && qubits.size() == 2) {
                return RXX(params[0], qubits[0], qubits[1]);
            }
            if (gate_name == "ryy" && params.size() == 1 && qubits.size() == 2) {
                return RYY(params[0], qubits[0], qubits[1]);
            }
            if (gate_name == "rzz" && params.size() == 1 && qubits.size() == 2) {
                return RZZ(params[0], qubits[0], qubits[1]);
            }
            if ((gate_name == "cp" || gate_name == "cu1") && params.size() == 1 && qubits.size() == 2) {
                return CP(params[0], qubits[0], qubits[1]);
            }
            if (gate_name == "crx" && params.size() == 1 && qubits.size() == 2) {
                return CRX(params[0], qubits[0], qubits[1]);
            }
            if (gate_name == "cry" && params.size() == 1 && qubits.size() == 2) {
                return CRY(params[0], qubits[0], qubits[1]);
            }
            if (gate_name == "crz" && params.size() == 1 && qubits.size() == 2) {
                return CRZ(params[0], qubits[0], qubits[1]);
            }

            // Three-qubit gates
            if ((gate_name == "ccx" || gate_name == "toffoli") && params.empty() && qubits.size() == 3) {
                return CCX(qubits[0], qubits[1], qubits[2]);
            }
            if ((gate_name == "cswap" || gate_name == "fredkin") && params.empty() && qubits.size() == 3) {
                return CSWAP(qubits[0], qubits[1], qubits[2]);
            }

            throw std::runtime_error("Unsupported gate: " + gate_name +
                                   " with " + std::to_string(params.size()) + " params and " +
                                   std::to_string(qubits.size()) + " qubits");
        }
    };

} // namespace qarser
} // namespace qsteedcpp
