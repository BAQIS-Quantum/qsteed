#pragma once

#include <vector>
#include <string>
#include <set>
#include <cmath>
#include "circuit_instruction.h"
#include "expression/expr.h"


namespace qsteedcpp {

    class QuantumCircuit {
    private:
        int num_qubits_;
        int num_clbits_;
        std::vector<CircuitInstruction> instructions_;
        
    public:
        explicit QuantumCircuit(int num_qubits, int num_clbits = 0) 
            : num_qubits_(num_qubits), num_clbits_(num_clbits) {}
        
        int num_qubits() const { return num_qubits_; }
        int num_clbits() const { return num_clbits_; }
        size_t size() const { return instructions_.size(); }
        size_t num_gates() const { 
            size_t count = 0;
            for (const auto& inst : instructions_) {
                if (inst.is_gate()) count++;
            }
            return count;
        }
        
        // Single-qubit gates (non-parameterized)
        void h(int qubit);
        void x(int qubit);
        void y(int qubit);
        void z(int qubit);
        void s(int qubit);
        void sdg(int qubit);
        void t(int qubit);
        void tdg(int qubit);

        // Single-qubit gates (parameterized)
        void rx(const Expr& theta, int qubit);
        void rx(const Parameter& theta, int qubit);
        void rx(double theta, int qubit);

        void ry(const Expr& phi, int qubit);
        void ry(const Parameter& phi, int qubit);
        void ry(double phi, int qubit);

        void rz(const Expr& lambda, int qubit);
        void rz(const Parameter& lambda, int qubit);
        void rz(double lambda, int qubit);

        void p(const Expr& lambda, int qubit);
        void p(const Parameter& lambda, int qubit);
        void p(double lambda, int qubit);

        void u3(const Expr& theta, const Expr& phi, const Expr& lambda, int qubit);
        void u3(double theta, double phi, double lambda, int qubit);

        // Two-qubit gates (non-parameterized)
        void cnot(int control, int target);
        void cz(int control, int target);
        void swap(int qubit1, int qubit2);
        void iswap(int qubit1, int qubit2);

        // Two-qubit gates (parameterized)
        void rxx(const Expr& theta, int qubit1, int qubit2);
        void rxx(double theta, int qubit1, int qubit2);

        void ryy(const Expr& theta, int qubit1, int qubit2);
        void ryy(double theta, int qubit1, int qubit2);

        void rzz(const Expr& theta, int qubit1, int qubit2);
        void rzz(double theta, int qubit1, int qubit2);
        
        // Three-qubit gates
        void ccx(int control1, int control2, int target);
        void toffoli(int control1, int control2, int target);
        
        // Circuit operations
        void measure(int qubit, int clbit);
        void measure(const std::vector<int>& qubits, const std::vector<int>& clbits);
        void measure_all();
        void barrier(const std::vector<int>& qubits = {});
        void reset(int qubit);
        void append_c_if(std::unique_ptr<Gate> gate, const std::vector<int>& qubits, int clbit, int value);
        

        const std::vector<CircuitInstruction>& get_instructions() const {
            return instructions_;
        }
        void add_gate(std::unique_ptr<Gate> gate, const std::vector<int>& qubits);

        std::set<std::string> get_all_parameter_uuids() const;

        void print() const;

    private:
        void validate_qubit_index(int qubit) const;
        void validate_clbit_index(int clbit) const;
        void validate_gate_qubits(const std::vector<int>& qubits, const std::string& gate_name);
    };

} // namespace qsteedcpp


