#include <gtest/gtest.h>
#include <iostream>
#include <cmath>
#include "gates/standard_gates.h"
#include "circuit/parameter.h"
#include "gates/matrix.h"

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