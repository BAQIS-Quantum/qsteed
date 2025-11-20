#include <gtest/gtest.h>
#include <torch/torch.h>
#include <cmath>
#include "backends/torch/torch_backend.hpp"
#include "circuit/quantum_circuit.h"
#include "expression/expr.h"

using namespace qsteedcpp;

class TorchAutogradSimpleTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

// Test 1: 验证自动参数管理
TEST_F(TorchAutogradSimpleTest, AutoParameterManagement) {
    QuantumCircuit qc(1);
    Parameter theta_param(0.5, true);
    Expr theta(theta_param);
    qc.rx(theta, 0);

    TorchBackend backend;
    backend.compile(qc);

    EXPECT_TRUE(backend.is_compiled());
    EXPECT_EQ(backend.num_parameters(), 1);

    // 参数应该自动初始化为默认值 0.5
    auto params = backend.parameters();
    EXPECT_EQ(params.size(), 1);
    std::cout << "Parameter value: " << params[0] << std::endl;
}


// Test 2: 验证 run() 不需要参数
TEST_F(TorchAutogradSimpleTest, SimpleRun) {
    QuantumCircuit qc(1);
    Parameter theta_param(M_PI / 4.0, true);
    Expr theta(theta_param);
    qc.rx(theta, 0);

    TorchBackend backend;
    backend.compile(qc);

    torch::Tensor result = backend.run();

    EXPECT_EQ(result.dim(), 1);  // 1D vector
    EXPECT_EQ(result.size(0), 2);  // 2 elements

    std::cout << "Result: " << result << std::endl;
}

// Test 3: 验证梯度计算
TEST_F(TorchAutogradSimpleTest, GradientComputation) {
    QuantumCircuit qc(1);
    Parameter theta_param(M_PI / 4.0, true);  // 使用非零初始值
    Expr theta(theta_param);
    qc.rx(theta, 0);

    TorchBackend backend;
    backend.compile(qc);

    // Run
    torch::Tensor result = backend.run();

    // Compute loss: 测量到 |0⟩ 的概率
    // |ψ₀|² = Re(ψ₀)² + Im(ψ₀)²
    auto real_part = torch::real(result[0]);
    auto imag_part = torch::imag(result[0]);
    torch::Tensor loss = real_part.pow(2) + imag_part.pow(2);

    // Backward
    loss.backward();

    auto params = backend.parameters();
    EXPECT_TRUE(params[0].grad().defined());

    std::cout << "Parameter: " << params[0] << std::endl;
    std::cout << "Loss: " << loss << std::endl;
    std::cout << "Gradient: " << params[0].grad() << std::endl;

    // Gradient should not be zero (at θ=π/4)
    EXPECT_NE(params[0].grad().item<double>(), 0.0);
}