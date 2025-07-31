#include <gtest/gtest.h>
#include "circuit/quantum_circuit.h"
#include "passes/unroll/unroll_pass.h"

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

    circuit.print();
    pass_->run(circuit);
    circuit.print();

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
    circuit.print();

}