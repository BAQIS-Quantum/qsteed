#include <gtest/gtest.h>
#include <iostream>
#include <cmath>
#include "gates/standard_gates.h"
#include "circuit/parameter.h"
#include "gates/matrix.h"
#include "circuit/quantum_circuit.h"

using namespace qsteedcpp;

class GateTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    
    void TearDown() override {
    }
};

TEST_F(GateTest, BasicGateUsage) {
    std::cout << "=== 参数化门使用示例 ===" << std::endl;
    
    // H Gate
    HGate h_gate;
    auto h_matrix = h_gate.get_matrix();
    h_matrix.print();

    // RX Gate
    std::cout << "=== RX Gate ===" << std::endl;
    auto theta = Parameter::variable("theta");
    RXGate rx_gate(theta);

    std::map<std::string, double> params = {{"theta", M_PI}};
    rx_gate.get_matrix(params).print();



    // RY Gate
    std::cout << "=== RY Gate ===" << std::endl;
    auto expr = theta;
    RYGate ry_gate(expr);
    ry_gate.get_matrix(params).print();

    params["theta"] = M_PI / 4;
    ry_gate.get_matrix(params).print();

    
    auto alpha = Parameter::variable("alpha");
    auto ry_gate2 = RYGate(alpha * theta + M_PI / 6);

    std::map<std::string, double> params2 = {{"alpha", M_PI / 2}, {"theta", M_PI / 2}};
    ry_gate2.get_matrix(params2).print();

}

TEST_F(GateTest, ParameterDerivativeTest) {
    std::cout << "=== 参数求导测试 ===" << std::endl;
    
    auto x = Parameter::variable("x");
    auto y = Parameter::variable("y");
    auto expr = 2.0 * x + 3.0 * sin(y) + 1.0;
    std::map<std::string, double> params = {{"x", 1.0}, {"y", 2.0}};
    
    auto grad2 = expr.compute_gradients(params);
    std::cout << "∂/∂x = " << grad2["x"] << std::endl;
    std::cout << "∂/∂y = " << grad2["y"] << std::endl;
}

TEST_F(GateTest, CircuitUnitaryMatrix) {
    std::cout << "=== Circuit Unitary Matrix Test ===" << std::endl;

    // 1. Create a simple 2-qubit circuit to generate a Bell state
    QuantumCircuit qc(2);
    qc.h(0);
    qc.cnot(0, 1);

    // 2. Calculate the unitary matrix of the circuit
    Matrix result_matrix = qc.get_unitary_matrix();
    std::cout << "Result matrix:" << std::endl;
    result_matrix.print();

    // 3. Define the expected theoretical matrix for CNOT(0,1) * H(0)
    // Note: Qubit order is q1, q0. H(0) is I ⊗ H.
    double inv_sqrt2 = 1.0 / std::sqrt(2.0);
    Matrix expected_matrix(4, 4);
    expected_matrix(0, 0) = {inv_sqrt2, 0};
    expected_matrix(0, 1) = {inv_sqrt2, 0};
    expected_matrix(1, 2) = {inv_sqrt2, 0};
    expected_matrix(1, 3) = {-inv_sqrt2, 0};
    expected_matrix(2, 2) = {inv_sqrt2, 0};
    expected_matrix(2, 3) = {inv_sqrt2, 0};
    expected_matrix(3, 0) = {inv_sqrt2, 0};
    expected_matrix(3, 1) = {-inv_sqrt2, 0};

    std::cout << "Expected matrix:" << std::endl;
    expected_matrix.print();

    // 4. Compare the result with the expected matrix
    Matrix diff = result_matrix - expected_matrix;
    EXPECT_NEAR(diff.norm(), 0.0, 1e-9);
}
