#include <gtest/gtest.h>
#include "gates/parameter.h"
#include <autodiff/reverse/var.hpp>

using namespace qsteedcpp;
using namespace autodiff;

class ParameterTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    
    void TearDown() override {
    }
};

TEST_F(ParameterTest, BasicVarUsage) {
    // 直接使用var类型
    Parameter theta = 0.5;
    
    // 测试基本数值
    EXPECT_DOUBLE_EQ(static_cast<double>(theta), 0.5);
    
    // 测试运算
    Parameter result = theta * 2.0 + 1.0;
    EXPECT_DOUBLE_EQ(static_cast<double>(result), 2.0);  // 0.5 * 2 + 1 = 2
}

TEST_F(ParameterTest, ReverseModeDifferentiation) {
    // f(x) = x^2 + 2*x + 1
    auto f = [](var x) {
        return x * x + 2.0 * x + 1.0;
    };
    
    var x = 2.0;
    var result = f(x);
    
    auto [dx] = derivatives(result, wrt(x));
    
    EXPECT_DOUBLE_EQ(static_cast<double>(result), 9.0);      // f(2) = 4 + 4 + 1 = 9
    EXPECT_DOUBLE_EQ(static_cast<double>(dx), 6.0);          // f'(x) = 2x + 2, f'(2) = 6
}

TEST_F(ParameterTest, MultiVariableGradient) {
    // f(x, y) = x*y + x^2 + y^2
    auto f = [](var x, var y) {
        return x * y + x * x + y * y;
    };
    
    var x = 1.0;
    var y = 2.0;
    var result = f(x, y);
    
    auto [dx, dy] = derivatives(result, wrt(x, y));
    
    EXPECT_DOUBLE_EQ(static_cast<double>(result), 7.0);  // f(1,2) = 1*2 + 1^2 + 2^2 = 7
    EXPECT_DOUBLE_EQ(static_cast<double>(dx), 4.0);      // ∂f/∂x = y + 2x = 2 + 2 = 4
    EXPECT_DOUBLE_EQ(static_cast<double>(dy), 5.0);      // ∂f/∂y = x + 2y = 1 + 4 = 5
}

TEST_F(ParameterTest, ParameterizedFunction) {
    // f(theta) = sin(theta) + cos(theta)
    auto parameterized_func = [](const std::map<std::string, double>& params) {
        Parameter theta = get_parameter("theta", params, 0.0);
        return sin(theta) + cos(theta);
    };
    
    std::map<std::string, double> params1 = {{"theta", 0.0}};
    Parameter result1 = parameterized_func(params1);
    EXPECT_NEAR(static_cast<double>(result1), 1.0, 1e-10);  // sin(0) + cos(0) = 0 + 1 = 1
    
    std::map<std::string, double> params2 = {{"theta", M_PI/2}};
    Parameter result2 = parameterized_func(params2);
    EXPECT_NEAR(static_cast<double>(result2), 1.0, 1e-10);  // sin(π/2) + cos(π/2) = 1 + 0 = 1
}

TEST_F(ParameterTest, QuantumCircuitSimulation) {
    var theta1 = 0.5;
    var theta2 = 1.0;
    var theta3 = 0.3;
    
    var circuit_output = sin(theta1) * cos(theta2) + exp(theta3);
    
    auto [dtheta1, dtheta2, dtheta3] = derivatives(circuit_output, wrt(theta1, theta2, theta3));
    
    double expected_value = std::sin(0.5) * std::cos(1.0) + std::exp(0.3);
    EXPECT_NEAR(static_cast<double>(circuit_output), expected_value, 1e-10);
    
    // 验证梯度值（解析解）
    double expected_dtheta1 = std::cos(0.5) * std::cos(1.0);
    double expected_dtheta2 = -std::sin(0.5) * std::sin(1.0);
    double expected_dtheta3 = std::exp(0.3);
    
    EXPECT_NEAR(static_cast<double>(dtheta1), expected_dtheta1, 1e-10);
    EXPECT_NEAR(static_cast<double>(dtheta2), expected_dtheta2, 1e-10);
    EXPECT_NEAR(static_cast<double>(dtheta3), expected_dtheta3, 1e-10);
}


