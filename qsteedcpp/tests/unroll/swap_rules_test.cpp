#include <gtest/gtest.h>
#include <Eigen/Dense>
#include <complex>
#include <set>
#include <string>
#include <functional>
#include "circuit/quantum_circuit.h"
#include "passes/include/unroll/unroll_pass.h"

using namespace qsteedcpp;

namespace {

void AssertMatrixAlmostEqual(const Matrix& original, const Matrix& decomposed, double global_phase = 0.0, double tol = 1e-9) {
    ASSERT_EQ(original.rows(), decomposed.rows());
    ASSERT_EQ(original.cols(), decomposed.cols());

    std::complex<double> phase_factor = std::exp(std::complex<double>(0, global_phase));

    for (int i = 0; i < original.rows(); ++i) {
        for (int j = 0; j < original.cols(); ++j) {
            auto val1 = original(i, j) * phase_factor;
            auto val2 = decomposed(i, j);
            EXPECT_NEAR(val1.real(), val2.real(), tol) << "Matrices differ at (" << i << "," << j << ")";
            EXPECT_NEAR(val1.imag(), val2.imag(), tol) << "Matrices differ at (" << i << "," << j << ")";
        }
    }
}

void VerifyRule(
    std::function<void(QuantumCircuit&)> apply_gate,
    int num_qubits,
    const std::set<std::string>& basis_gates,
    double global_phase = 0.0
) {
    // 1. Original circuit
    QuantumCircuit original_circuit(num_qubits);
    apply_gate(original_circuit);
    auto original_unitary = original_circuit.get_unitary_matrix();

    // 2. Decomposed circuit
    QuantumCircuit decomposed_circuit(num_qubits);
    apply_gate(decomposed_circuit);
    
    UnrollPass pass(basis_gates);
    pass.run(decomposed_circuit);
    
    auto decomposed_unitary = decomposed_circuit.get_unitary_matrix();

    // 3. Compare unitaries
    AssertMatrixAlmostEqual(original_unitary, decomposed_unitary, global_phase);
}

} // namespace

class SwapRuleTest : public ::testing::Test {};

TEST_F(SwapRuleTest, SwapToCnot) {
    VerifyRule(
        [](QuantumCircuit& qc) { qc.swap(0, 1); },
        2, {"cx", "cnot"}, 0.0
    );
}

TEST_F(SwapRuleTest, SwapToCzH) {
    VerifyRule(
        [](QuantumCircuit& qc) { qc.swap(0, 1); },
        2, {"cz", "h"}, 0.0
    );
}

TEST_F(SwapRuleTest, SwapToCzRxRy) {
    VerifyRule(
        [](QuantumCircuit& qc) { qc.swap(0, 1); },
        2, {"cz", "rx", "ry"}, M_PI
    );
}
