#include <gtest/gtest.h>
#include "circuit/quantum_circuit.h"
#include "Passes/unroll/unroll_pass.h"

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
    // auto params = std::get<std::unique_ptr<Gate>>(instructions[1].operation)->get_parameter_values();
    // EXPECT_NEAR(params[0], theta, 1e-9);
}