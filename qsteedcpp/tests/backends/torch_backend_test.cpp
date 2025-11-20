#include <gtest/gtest.h>
#include <torch/torch.h>
#include <cmath>
#include "backends/torch/torch_backend.hpp"
#include "circuit/quantum_circuit.h"

using namespace qsteedcpp;

class TorchBackendTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

// Test 1: Basic circuit with single H gate
TEST_F(TorchBackendTest, SingleHadamardGate) {
    // Create a 1-qubit circuit with H gate
    QuantumCircuit qc(1);
    qc.h(0);

    // Compile and run with TorchBackend
    TorchBackend backend;
    backend.compile(qc);

    EXPECT_TRUE(backend.is_compiled());

    torch::Tensor result = backend.run();

    // After H gate: |+⟩ = (|0⟩ + |1⟩)/√2
    // Expected amplitudes: [1/√2, 1/√2]
    auto result_cpu = result.cpu();
    auto result_accessor = result_cpu.accessor<c10::complex<double>, 1>();

    double expected = 1.0 / std::sqrt(2.0);
    EXPECT_NEAR(result_accessor[0].real(), expected, 1e-6);
    EXPECT_NEAR(result_accessor[0].imag(), 0.0, 1e-6);
    EXPECT_NEAR(result_accessor[1].real(), expected, 1e-6);
    EXPECT_NEAR(result_accessor[1].imag(), 0.0, 1e-6);
}

// Test 2: Two-qubit circuit with CNOT
TEST_F(TorchBackendTest, CNOTGate) {
    // Create a 2-qubit circuit: H on q0, then CNOT(q0, q1)
    QuantumCircuit qc(2);
    qc.h(0);
    qc.cnot(0, 1);

    // Compile and run
    TorchBackend backend;
    backend.compile(qc);
    torch::Tensor result = backend.run();

    // Expected: Bell state |Φ+⟩ = (|00⟩ + |11⟩)/√2
    // Amplitudes: [1/√2, 0, 0, 1/√2]
    auto result_cpu = result.cpu();
    auto result_flat = result_cpu.reshape({-1});
    auto result_accessor = result_flat.accessor<c10::complex<double>, 1>();

    double expected = 1.0 / std::sqrt(2.0);
    EXPECT_NEAR(result_accessor[0].real(), expected, 1e-6);  // |00⟩
    EXPECT_NEAR(result_accessor[1].real(), 0.0, 1e-6);       // |01⟩
    EXPECT_NEAR(result_accessor[2].real(), 0.0, 1e-6);       // |10⟩
    EXPECT_NEAR(result_accessor[3].real(), expected, 1e-6);  // |11⟩
}

// Test 3: Parametric gate (RX)
TEST_F(TorchBackendTest, ParametricGate) {
    // Create circuit with RX(π/2) gate
    QuantumCircuit qc(1);
    double angle = M_PI / 2.0;
    qc.rx(angle, 0);  // Note: rx(theta, qubit)

    // Compile and run
    TorchBackend backend;
    backend.compile(qc);
    torch::Tensor result = backend.run();

    // After RX(π/2): [cos(π/4), -i*sin(π/4)]
    auto result_cpu = result.cpu();
    auto result_accessor = result_cpu.accessor<c10::complex<double>, 1>();

    double expected_real = std::cos(M_PI / 4.0);
    double expected_imag = -std::sin(M_PI / 4.0);

    EXPECT_NEAR(result_accessor[0].real(), expected_real, 1e-6);
    EXPECT_NEAR(result_accessor[0].imag(), 0.0, 1e-6);
    EXPECT_NEAR(result_accessor[1].real(), 0.0, 1e-6);
    EXPECT_NEAR(result_accessor[1].imag(), expected_imag, 1e-6);
}

// Test 4: Reset functionality
TEST_F(TorchBackendTest, ResetBackend) {
    QuantumCircuit qc(1);
    qc.h(0);

    TorchBackend backend;
    backend.compile(qc);
    EXPECT_TRUE(backend.is_compiled());

    backend.reset();
    EXPECT_FALSE(backend.is_compiled());
}
