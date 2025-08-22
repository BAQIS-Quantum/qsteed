#pragma once

#include <vector>
#include <memory>
#include <string>
#include <map>
#include <algorithm>
#include <cmath>
#include "parameter.h"
#include "circuit_instruction.h"
#include "gates/matrix.h"
#include "gates/base_gate.h"


namespace qsteedcpp {

    struct ParameterGradInfo {
        size_t gate_index;
        size_t param_index;
        double grad_value;
        
        ParameterGradInfo(size_t gi, size_t pi, double gv) 
            : gate_index(gi), param_index(pi), grad_value(gv) {}
    };

    using ParameterGrads = std::map<std::string, std::vector<ParameterGradInfo>>;



    class QuantumCircuit {
    private:
        int num_qubits_;
        int num_clbits_;
        std::vector<CircuitInstruction> instructions_;
        
        // Grad
        mutable ParameterGrads parameter_grads_;
        mutable std::vector<std::string> variables_;
        mutable bool grads_computed_;
        
    public:
        explicit QuantumCircuit(int num_qubits, int num_clbits = 0) 
            : num_qubits_(num_qubits), num_clbits_(num_clbits), grads_computed_(false) {}
        
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
        
        // Single-qubit gates
        void h(int qubit);
        void x(int qubit);
        void y(int qubit);
        void z(int qubit);
        void rx(const Parameter& theta, int qubit);
        void ry(const Parameter& phi, int qubit);
        void rz(const Parameter& lambda, int qubit);
        void s(int qubit);
        void sdg(int qubit);
        void t(int qubit);
        void tdg(int qubit);
        void p(const Parameter& lambda, int qubit);
        void u3(const Parameter& theta, const Parameter& phi, const Parameter& lambda, int qubit);
        
        // Two-qubit gates
        void cnot(int control, int target);
        void cz(int control, int target);
        void swap(int qubit1, int qubit2);
        void iswap(int qubit1, int qubit2);
        void rxx(const Parameter& theta, int qubit1, int qubit2);
        void ryy(const Parameter& theta, int qubit1, int qubit2);
        void rzz(const Parameter& theta, int qubit1, int qubit2);
        
        // Three-qubit gates
        void ccx(int control1, int control2, int target);
        void toffoli(int control1, int control2, int target);
        
        // Circuit operations
        void measure(int qubit, int clbit);
        void measure(const std::vector<int>& qubits, const std::vector<int>& clbits);
        void measure_all();  // 测量所有量子比特到对应的经典比特
        void barrier(const std::vector<int>& qubits = {});
        void reset(int qubit);
        void append_c_if(std::unique_ptr<Gate> gate, const std::vector<int>& qubits, int clbit, int value);
        

        const std::vector<CircuitInstruction>& get_instructions() const {
            return instructions_;
        }
        void add_gate(std::unique_ptr<Gate> gate, const std::vector<int>& qubits);
        
        // Parameter handling
        std::vector<std::string> get_parameters() const;
        const ParameterGrads& get_parameter_grads() const;
        const std::vector<std::string>& get_variables() const;
        void update_parameters(const std::map<std::string, double>& param_values);
        
        // Gradient computation
        std::map<std::string, double> compute_gradients_for_parameter(
            size_t inst_index, 
            size_t param_index,
            const std::map<std::string, double>& param_values) const;
        
        void print_parameter_grads() const;
        void print() const;

        // Unitary matrix representation
        Matrix get_unitary_matrix() const;

    private:
        void validate_qubit_index(int qubit) const;
        void validate_clbit_index(int clbit) const;
        void validate_gate_qubits(const std::vector<int>& qubits, const std::string& gate_name);

        void compute_parameter_grads() const;
    };


    // --- Implementation of get_unitary_matrix ---

    inline Matrix QuantumCircuit::get_unitary_matrix() const {
        int n_qubits = this->num_qubits();
        if (n_qubits == 0) {
            return Matrix::identity(1);
        }
        long long dim = 1LL << n_qubits;

        Matrix total_unitary = Matrix::identity(dim);
        Matrix I2 = Matrix::identity(2);

        for (const auto& instr : instructions_) {
            // Skip non-unitary or classically-controlled instructions
            if (!instr.is_gate() || instr.condition.has_value()) {
                continue;
            }

            const auto& gate_ptr = std::get<std::unique_ptr<Gate>>(instr.operation);
            Matrix gate_matrix = gate_ptr->get_matrix();
            const auto& q_indices = instr.qubits;

            Matrix expanded_gate;

            // Efficient path for single-qubit gates
            if (q_indices.size() == 1) {
                int target_q = q_indices[0];
                // Correctly build the tensor product: Op_{n-1} ⊗ ... ⊗ Op_0
                // Start with the operator for the highest-indexed qubit (n-1)
                expanded_gate = (n_qubits - 1 == target_q) ? gate_matrix : I2;
                // Iteratively apply tensor product for the remaining qubits, from n-2 down to 0
                for (int i = n_qubits - 2; i >= 0; --i) {
                    const Matrix& op_i = (i == target_q) ? gate_matrix : I2;
                    expanded_gate = kronecker_product(expanded_gate, op_i);
                }

            } else { // General (but slower) method for multi-qubit gates
                expanded_gate = Matrix(dim, dim);
                
                // Create a map from the global qubit index to its local index within the gate.
                // The order of q_indices is crucial for non-symmetric gates like CNOT.
                // DO NOT sort q_indices.
                std::map<int, int> q_map;
                for(size_t i = 0; i < q_indices.size(); ++i) {
                    q_map[q_indices[i]] = i;
                }

                for (long long i = 0; i < dim; ++i) {
                    for (long long j = 0; j < dim; ++j) {
                        long long i_unaffected_bits = 0;
                        long long j_unaffected_bits = 0;
                        long long i_local_idx = 0;
                        long long j_local_idx = 0;

                        // Decompose global indices i and j into local (gate) and unaffected parts
                        for (int k = 0; k < n_qubits; ++k) {
                            auto it = q_map.find(k);
                            if (it != q_map.end()) { // This qubit is a target of the gate
                                if ((i >> k) & 1) i_local_idx |= (1LL << it->second);
                                if ((j >> k) & 1) j_local_idx |= (1LL << it->second);
                            } else { // This qubit is unaffected
                                if ((i >> k) & 1) i_unaffected_bits |= (1LL << k);
                                if ((j >> k) & 1) j_unaffected_bits |= (1LL << k);
                            }
                        }

                        // If the unaffected parts of the basis states are the same,
                        // the matrix element is determined by the local gate matrix.
                        if (i_unaffected_bits == j_unaffected_bits) {
                            expanded_gate(i, j) = gate_matrix(i_local_idx, j_local_idx);
                        } else {
                            expanded_gate(i, j) = {0.0, 0.0}; // Otherwise, it's zero
                        }
                    }
                }
            }
            
            // Left-multiply the new gate's matrix to the total unitary
            total_unitary = expanded_gate * total_unitary;
        }

        return total_unitary;
    }

}; // namespace qsteedcpp
