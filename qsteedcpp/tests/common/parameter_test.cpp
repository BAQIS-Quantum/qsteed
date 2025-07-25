#include <gtest/gtest.h>
#include "circuit/parameter.h"
#include <iostream>
#include <cmath>

using namespace qsteedcpp;

class ParameterTest : public ::testing::Test {
protected:
    void SetUp() override {
    }
    
    void TearDown() override {
    }
};

// 测试基本算术运算
TEST_F(ParameterTest, BasicArithmetic) {
    Parameter a = Parameter::variable("a");
    Parameter b = Parameter::variable("b");
    
    std::map<std::string, double> params = {{"a", 10.0}, {"b", 3.0}};
    
    // 加法
    Parameter sum = a + b;
    EXPECT_DOUBLE_EQ(sum.value(params), 13.0);
    
    // 减法
    Parameter diff = a - b;
    EXPECT_DOUBLE_EQ(diff.value(params), 7.0);
    
    // 乘法
    Parameter product = a * b;
    EXPECT_DOUBLE_EQ(product.value(params), 30.0);
    
    // 除法
    Parameter quotient = a / b;
    EXPECT_DOUBLE_EQ(quotient.value(params), 10.0 / 3.0);
    
    // 负号
    Parameter neg_a = -a;
    EXPECT_DOUBLE_EQ(neg_a.value(params), -10.0);
}

// 测试与常数的混合运算
TEST_F(ParameterTest, MixedArithmetic) {
    Parameter x = Parameter::variable("x");
    std::map<std::string, double> params = {{"x", 5.0}};
    
    // 变量 + 常数
    Parameter expr1 = x + 3.0;
    EXPECT_DOUBLE_EQ(expr1.value(params), 8.0);
    
    // 常数 + 变量
    Parameter expr2 = 2.0 + x;
    EXPECT_DOUBLE_EQ(expr2.value(params), 7.0);
    
    // 变量 * 常数
    Parameter expr3 = x * 4.0;
    EXPECT_DOUBLE_EQ(expr3.value(params), 20.0);
    
    // 常数 / 变量
    Parameter expr4 = 15.0 / x;
    EXPECT_DOUBLE_EQ(expr4.value(params), 3.0);
}

// 测试数学函数
TEST_F(ParameterTest, MathematicalFunctions) {
    Parameter theta = Parameter::variable("theta");
    std::map<std::string, double> params = {{"theta", M_PI / 4}}; // 45度
    
    // 三角函数
    EXPECT_NEAR(sin(theta).value(params), std::sin(M_PI / 4), 1e-10);
    EXPECT_NEAR(cos(theta).value(params), std::cos(M_PI / 4), 1e-10);
    EXPECT_NEAR(tan(theta).value(params), std::tan(M_PI / 4), 1e-10);
    
    // 指数和对数
    Parameter x = Parameter::variable("x");
    std::map<std::string, double> params2 = {{"x", 2.0}};
    
    EXPECT_NEAR(exp(x).value(params2), std::exp(2.0), 1e-10);
    EXPECT_NEAR(log(x).value(params2), std::log(2.0), 1e-10);
    EXPECT_NEAR(sqrt(x).value(params2), std::sqrt(2.0), 1e-10);
    
    // 幂函数
    EXPECT_NEAR(pow(x, 3.0).value(params2), std::pow(2.0, 3.0), 1e-10);
}

// 测试复杂表达式
TEST_F(ParameterTest, ComplexExpressions) {
    Parameter theta = Parameter::variable("theta");
    Parameter phi = Parameter::variable("phi");
    
    std::map<std::string, double> params = {
        {"theta", M_PI / 6},  // 30度
        {"phi", M_PI / 3}     // 60度
    };
    
    // 复杂表达式：sin(theta) * cos(phi) + theta * 2.0
    Parameter complex_expr = sin(theta) * cos(phi) + theta * 2.0;
    
    double expected = std::sin(M_PI / 6) * std::cos(M_PI / 3) + (M_PI / 6) * 2.0;
    EXPECT_NEAR(complex_expr.value(params), expected, 1e-10);
    
    // 测试表达式字符串表示
    std::string expr_str = complex_expr.to_string();
    std::cout << "Complex expression: " << expr_str << std::endl;
    EXPECT_FALSE(expr_str.empty());
}

// 测试参数提取
TEST_F(ParameterTest, ParameterExtraction) {
    Parameter a = Parameter::variable("a");
    Parameter b = Parameter::variable("b");
    Parameter c = Parameter::variable("c");
    
    // 简单表达式
    Parameter expr1 = a + b;
    auto params1 = expr1.get_variables();
    EXPECT_EQ(params1.size(), 2);
    EXPECT_TRUE(std::find(params1.begin(), params1.end(), "a") != params1.end());
    EXPECT_TRUE(std::find(params1.begin(), params1.end(), "b") != params1.end());
    
    // 复杂表达式
    Parameter expr2 = sin(a) * cos(b) + c * 2.0;
    auto params2 = expr2.get_variables();
    EXPECT_EQ(params2.size(), 3);
    EXPECT_TRUE(std::find(params2.begin(), params2.end(), "a") != params2.end());
    EXPECT_TRUE(std::find(params2.begin(), params2.end(), "b") != params2.end());
    EXPECT_TRUE(std::find(params2.begin(), params2.end(), "c") != params2.end());
    
    // 常量表达式
    Parameter expr3 = Parameter(3.14) + Parameter(2.0);
    auto params3 = expr3.get_variables();
    EXPECT_EQ(params3.size(), 0);
}

// 测试链式运算
TEST_F(ParameterTest, ChainedOperations) {
    Parameter x = Parameter::variable("x");
    std::map<std::string, double> params = {{"x", 2.0}};
    
    // 链式运算：sin(cos(x)) + exp(sqrt(x))
    Parameter expr = sin(cos(x)) + exp(sqrt(x));
    
    double expected = std::sin(std::cos(2.0)) + std::exp(std::sqrt(2.0));
    EXPECT_NEAR(expr.value(params), expected, 1e-10);
    
    std::cout << "Chained expression: " << expr.to_string() << std::endl;
}

// 测试 autodiff 梯度计算
TEST_F(ParameterTest, AutodiffGradient) {
    // 创建参数化表达式：f(x, y) = sin(x) * cos(y) + x^2
    Parameter x = Parameter::variable("x");
    Parameter y = Parameter::variable("y");
    Parameter expr = sin(x) * cos(y) + pow(x, 2.0);
    
    // 设置参数值
    std::map<std::string, double> params = {{"x", 1.0}, {"y", 2.0}};
    
    // 计算正向值
    double value = expr.value(params);
    double expected_value = std::sin(1.0) * std::cos(2.0) + std::pow(1.0, 2.0);
    EXPECT_NEAR(value, expected_value, 1e-10);
    
    // 计算梯度
    auto gradients = expr.compute_gradients(params);
    
    // 手动计算理论梯度值
    // ∂f/∂x = cos(x) * cos(y) + 2*x
    // ∂f/∂y = -sin(x) * sin(y)
    double expected_grad_x = std::cos(1.0) * std::cos(2.0) + 2.0 * 1.0;
    double expected_grad_y = -std::sin(1.0) * std::sin(2.0);
    
    // 验证梯度
    EXPECT_NEAR(gradients["x"], expected_grad_x, 1e-8);
    EXPECT_NEAR(gradients["y"], expected_grad_y, 1e-8);
    
    // 打印结果
    std::cout << "Expression: " << expr.to_string() << std::endl;
    std::cout << "Value at (x=1, y=2): " << value << std::endl;
    std::cout << "Gradient ∂f/∂x: " << gradients["x"] << " (expected: " << expected_grad_x << ")" << std::endl;
    std::cout << "Gradient ∂f/∂y: " << gradients["y"] << " (expected: " << expected_grad_y << ")" << std::endl;
}

