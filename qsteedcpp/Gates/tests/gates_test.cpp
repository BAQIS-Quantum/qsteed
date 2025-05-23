#include <gtest/gtest.h>
#include <complex>
#include <vector>
#include "gates/standard_gates.h"

using namespace Gate;

Matrix to_column_matrix(const std::vector<std::complex<double>>& vec) {
    std::vector<std::vector<Complex>> matrix;
    for (const auto& val : vec) {
        matrix.push_back({Complex(val.real(), val.imag())});
    }
    return Matrix(matrix);
}

// 测试Hadamard门
TEST(GateTest, HadamardGate) {
    HadamardGate h;
    
    // 测试|0⟩态
    std::vector<std::complex<double>> state0 = {1.0, 0.0};
    auto result0 = h.apply(to_column_matrix(state0));
    EXPECT_NEAR(std::abs(result0(0, 0)), 1.0/std::sqrt(2.0), 1e-10);
    EXPECT_NEAR(std::abs(result0(1, 0)), 1.0/std::sqrt(2.0), 1e-10);
    
    // 测试|1⟩态
    std::vector<std::complex<double>> state1 = {0.0, 1.0};
    auto result1 = h.apply(to_column_matrix(state1));
    EXPECT_NEAR(std::abs(result1(0, 0)), 1.0/std::sqrt(2.0), 1e-10);
    EXPECT_NEAR(std::abs(result1(1, 0)), 1.0/std::sqrt(2.0), 1e-10);
}

// 测试Pauli-X门
TEST(GateTest, PauliXGate) {
    PauliXGate x;
    
    // 测试|0⟩态
    std::vector<std::complex<double>> state0 = {1.0, 0.0};
    auto result0 = x.apply(to_column_matrix(state0));
    EXPECT_NEAR(std::abs(result0(0, 0)), 0.0, 1e-10);
    EXPECT_NEAR(std::abs(result0(1, 0)), 1.0, 1e-10);
    
    // 测试|1⟩态
    std::vector<std::complex<double>> state1 = {0.0, 1.0};
    auto result1 = x.apply(to_column_matrix(state1));
    EXPECT_NEAR(std::abs(result1(0, 0)), 1.0, 1e-10);
    EXPECT_NEAR(std::abs(result1(1, 0)), 0.0, 1e-10);
}

// 测试Pauli-Y门
TEST(GateTest, PauliYGate) {
    PauliYGate y;
    
    // 测试|0⟩态
    std::vector<std::complex<double>> state0 = {1.0, 0.0};
    auto result0 = y.apply(to_column_matrix(state0));
    EXPECT_NEAR(std::abs(result0(0, 0)), 0.0, 1e-10);
    EXPECT_NEAR(std::abs(result0(1, 0)), 1.0, 1e-10);
    
    // 测试|1⟩态
    std::vector<std::complex<double>> state1 = {0.0, 1.0};
    auto result1 = y.apply(to_column_matrix(state1));
    EXPECT_NEAR(std::abs(result1(0, 0)), 1.0, 1e-10);
    EXPECT_NEAR(std::abs(result1(1, 0)), 0.0, 1e-10);
}

// 测试Pauli-Z门
TEST(GateTest, PauliZGate) {
    PauliZGate z;
    
    // 测试|0⟩态
    std::vector<std::complex<double>> state0 = {1.0, 0.0};
    auto result0 = z.apply(to_column_matrix(state0));
    EXPECT_NEAR(std::abs(result0(0, 0)), 1.0, 1e-10);
    EXPECT_NEAR(std::abs(result0(1, 0)), 0.0, 1e-10);
    
    // 测试|1⟩态
    std::vector<std::complex<double>> state1 = {0.0, 1.0};
    auto result1 = z.apply(to_column_matrix(state1));
    EXPECT_NEAR(std::abs(result1(0, 0)), 0.0, 1e-10);
    EXPECT_NEAR(std::abs(result1(1, 0)), 1.0, 1e-10);
}

// 测试S门
TEST(GateTest, SGate) {
    SGate s;
    
    // 测试|0⟩态
    std::vector<std::complex<double>> state0 = {1.0, 0.0};
    auto result0 = s.apply(to_column_matrix(state0));
    EXPECT_NEAR(std::abs(result0(0, 0)), 1.0, 1e-10);
    EXPECT_NEAR(std::abs(result0(1, 0)), 0.0, 1e-10);
    
    // 测试|1⟩态
    std::vector<std::complex<double>> state1 = {0.0, 1.0};
    auto result1 = s.apply(to_column_matrix(state1));
    EXPECT_NEAR(std::abs(result1(0, 0)), 0.0, 1e-10);
    EXPECT_NEAR(std::abs(result1(1, 0)), 1.0, 1e-10);
}

// 测试T门
TEST(GateTest, TGate) {
    TGate t;
    
    // 测试|0⟩态
    std::vector<std::complex<double>> state0 = {1.0, 0.0};
    auto result0 = t.apply(to_column_matrix(state0));
    EXPECT_NEAR(std::abs(result0(0, 0)), 1.0, 1e-10);
    EXPECT_NEAR(std::abs(result0(1, 0)), 0.0, 1e-10);
    
    // 测试|1⟩态
    std::vector<std::complex<double>> state1 = {0.0, 1.0};
    auto result1 = t.apply(to_column_matrix(state1));
    EXPECT_NEAR(std::abs(result1(0, 0)), 0.0, 1e-10);
    EXPECT_NEAR(std::abs(result1(1, 0)), 1.0, 1e-10);
}

// 测试CNOT门
TEST(GateTest, CNOTGate) {
    CNOTGate cnot;
    
    // 测试|00⟩态
    std::vector<std::complex<double>> state00 = {1.0, 0.0, 0.0, 0.0};
    auto result00 = cnot.apply(to_column_matrix(state00));
    EXPECT_NEAR(std::abs(result00(0, 0)), 1.0, 1e-10);
    EXPECT_NEAR(std::abs(result00(1, 0)), 0.0, 1e-10);
    EXPECT_NEAR(std::abs(result00(2, 0)), 0.0, 1e-10);
    EXPECT_NEAR(std::abs(result00(3, 0)), 0.0, 1e-10);
    
    // 测试|10⟩态
    std::vector<std::complex<double>> state10 = {0.0, 0.0, 1.0, 0.0};
    auto result10 = cnot.apply(to_column_matrix(state10));
    EXPECT_NEAR(std::abs(result10(0, 0)), 0.0, 1e-10);
    EXPECT_NEAR(std::abs(result10(1, 0)), 0.0, 1e-10);
    EXPECT_NEAR(std::abs(result10(2, 0)), 0.0, 1e-10);
    EXPECT_NEAR(std::abs(result10(3, 0)), 1.0, 1e-10);
}

// int main(int argc, char **argv) {
//     testing::InitGoogleTest(&argc, argv);
//     return RUN_ALL_TESTS();
// } 