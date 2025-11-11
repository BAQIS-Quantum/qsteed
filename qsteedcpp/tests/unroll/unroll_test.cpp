#include <gtest/gtest.h>
#include "circuit/quantum_circuit.h"
#include "passes/include/unroll/unroll_pass.h"
#include <Eigen/Dense>

using namespace qsteedcpp;

class UnrollTest : public ::testing::Test {
protected:
    void SetUp() override {
        basis_gates_ = {"h", "cnot", "rz", "rx"};
        pass_ = std::make_unique<UnrollPass>(basis_gates_);
    }

    std::set<std::string> basis_gates_;
    std::unique_ptr<UnrollPass> pass_;
};

TEST_F(UnrollTest, Swap) {
    QuantumCircuit circuit(3);
    circuit.swap(0, 1);

    // circuit.print();
    pass_->run(circuit);
    // circuit.print();

    EXPECT_EQ(circuit.size(), 3);
    for (const auto& inst : circuit.get_instructions()) {
        EXPECT_TRUE(inst.is_gate());
        EXPECT_EQ(inst.name(), "cnot");
    }
}

TEST_F(UnrollTest, Mixed) {
    QuantumCircuit circuit(3, 3);
    circuit.h(0);
    circuit.swap(0, 1);
    circuit.cnot(1, 2);
    circuit.measure(0, 0);

    circuit.print();
    pass_->run(circuit);
    circuit.print();

    EXPECT_EQ(circuit.size(), 6);
}


TEST_F(UnrollTest, Recursive) {
    QuantumCircuit circuit(3);
    circuit.h(0);
    circuit.swap(0, 1);
    circuit.cnot(1, 2);
    circuit.ccx(0, 1, 2);
    
    circuit.print();
    pass_->run(circuit);
    // circuit.print();

}

TEST_F(UnrollTest, ToffoliDecomposition) {
    // 1. Define the circuit to be decomposed
    QuantumCircuit circuit_to_decompose(3);
    circuit_to_decompose.ccx(0, 1, 2);

    std::set<std::string> basis_gates = {"h", "cnot", "t", "tdg"};
    UnrollPass pass(basis_gates);

    pass.run(circuit_to_decompose);

    for (const auto& inst : circuit_to_decompose.get_instructions()) {
        std::cout << inst.name() << std::endl;
    }

    auto decomposed_unitary = circuit_to_decompose.get_unitary_matrix();
    decomposed_unitary.print();


    MatrixXcd m(8, 8);
    m.setIdentity();
    m(3, 3) = 0;
    m(7, 7) = 0;
    m(3, 7) = 1;
    m(7, 3) = 1;
    Matrix expected_unitary(m);


    const double tol = 1e-9;
    ASSERT_EQ(expected_unitary.rows(), decomposed_unitary.rows());
    ASSERT_EQ(expected_unitary.cols(), decomposed_unitary.cols());
    for (int i = 0; i < expected_unitary.rows(); ++i) {
        for (int j = 0; j < expected_unitary.cols(); ++j) {
            EXPECT_NEAR(expected_unitary(i, j).real(), decomposed_unitary(i, j).real(), tol);
            EXPECT_NEAR(expected_unitary(i, j).imag(), decomposed_unitary(i, j).imag(), tol);
        }
    }
}

TEST_F(UnrollTest, CZDecomposition) {
    // Test that CZ decomposes to H, CNOT, H
    QuantumCircuit circuit(2);
    circuit.cz(0, 1);

    std::set<std::string> basis = {"h", "cnot"};
    UnrollPass pass(basis);
    pass.run(circuit);

    EXPECT_EQ(circuit.size(), 3);
    auto instructions = circuit.get_instructions();
    EXPECT_EQ(instructions[0].name(), "h");
    EXPECT_EQ(instructions[1].name(), "cnot");
    EXPECT_EQ(instructions[2].name(), "h");
    
    // Check qubit positions
    EXPECT_EQ(instructions[0].qubits[0], 1);
    EXPECT_EQ(instructions[1].qubits[0], 0);
    EXPECT_EQ(instructions[1].qubits[1], 1);
    EXPECT_EQ(instructions[2].qubits[0], 1);
}

TEST_F(UnrollTest, RZZDecomposition) {
    // Test that RZZ(theta) decomposes to CNOT, RZ(theta), CNOT
    QuantumCircuit circuit(2);
    const double theta = 1.23;
    circuit.rzz(theta, 0, 1);

    std::set<std::string> basis = {"cnot", "rz"};
    UnrollPass pass(basis);
    pass.run(circuit);

    EXPECT_EQ(circuit.size(), 3);
    auto instructions = circuit.get_instructions();
    EXPECT_EQ(instructions[0].name(), "cnot");
    EXPECT_EQ(instructions[1].name(), "rz");
    EXPECT_EQ(instructions[2].name(), "cnot");

    // Check parameter
    auto params = std::get<std::unique_ptr<Gate>>(instructions[1].operation)->get_parameter_values();
    EXPECT_NEAR(params[0], theta, 1e-9);
}

TEST_F(UnrollTest, XtoRX) {
    // Test that X decomposes to RX(pi)
    QuantumCircuit circuit(1);
    circuit.x(0);

    std::set<std::string> basis = {"rx"};
    UnrollPass pass(basis);
    pass.run(circuit);

    EXPECT_EQ(circuit.size(), 1);
    auto instructions = circuit.get_instructions();
    EXPECT_EQ(instructions[0].name(), "rx");
    circuit.get_unitary_matrix().print();

    // Check parameter
    auto params = std::get<std::unique_ptr<Gate>>(instructions[0].operation)->get_parameter_values();
    EXPECT_NEAR(params[0], M_PI, 1e-9);
}