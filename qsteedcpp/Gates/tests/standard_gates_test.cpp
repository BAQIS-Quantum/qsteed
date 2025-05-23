#include <gtest/gtest.h>
#include <complex>
#include <vector>
#include "../include/gates/standard_gates.h"
#include <cmath>

using namespace Gate;
using Complex = std::complex<double>;

bool is_close(const Complex& a, const Complex& b, double tolerance = 1e-10) {
    return std::abs(a.real() - b.real()) < tolerance && 
           std::abs(a.imag() - b.imag()) < tolerance;
}

bool matrices_are_close(const Matrix& a, const Matrix& b, double tolerance = 1e-10) {
    if (a.rows() != b.rows() || a.cols() != b.cols()) return false;
    for (size_t i = 0; i < a.rows(); ++i) {
        for (size_t j = 0; j < a.cols(); ++j) {
            if (!is_close(a(i, j), b(i, j), tolerance)) return false;
        }
    }
    return true;
}

// 测试Hadamard门
TEST(StandardGatesTest, HadamardGate) {
    HadamardGate h;
    EXPECT_EQ(h.get_name(), "H");
    EXPECT_EQ(h.get_param_count(), 0);
    EXPECT_EQ(h.get_qubit_count(), 1);
    
    Matrix matrix = h.get_matrix();
    EXPECT_TRUE(matrix.is_unitary());
    EXPECT_NEAR(std::abs(matrix(0, 0)), 1.0/std::sqrt(2.0), 1e-10);
    EXPECT_NEAR(std::abs(matrix(0, 1)), 1.0/std::sqrt(2.0), 1e-10);
    EXPECT_NEAR(std::abs(matrix(1, 0)), 1.0/std::sqrt(2.0), 1e-10);
    EXPECT_NEAR(std::abs(matrix(1, 1)), 1.0/std::sqrt(2.0), 1e-10);
}

// 测试Pauli门
TEST(StandardGatesTest, PauliGates) {
    // X门
    PauliXGate x;
    EXPECT_EQ(x.get_name(), "X");
    Matrix x_matrix = x.get_matrix();
    EXPECT_TRUE(x_matrix.is_unitary());
    EXPECT_EQ(x_matrix(0, 0), Complex(0, 0));
    EXPECT_EQ(x_matrix(0, 1), Complex(1, 0));
    EXPECT_EQ(x_matrix(1, 0), Complex(1, 0));
    EXPECT_EQ(x_matrix(1, 1), Complex(0, 0));
    
    // Y门
    PauliYGate y;
    EXPECT_EQ(y.get_name(), "Y");
    Matrix y_matrix = y.get_matrix();
    EXPECT_TRUE(y_matrix.is_unitary());
    EXPECT_EQ(y_matrix(0, 0), Complex(0, 0));
    EXPECT_EQ(y_matrix(0, 1), Complex(0, -1));
    EXPECT_EQ(y_matrix(1, 0), Complex(0, 1));
    EXPECT_EQ(y_matrix(1, 1), Complex(0, 0));
    
    // Z门
    PauliZGate z;
    EXPECT_EQ(z.get_name(), "Z");
    Matrix z_matrix = z.get_matrix();
    EXPECT_TRUE(z_matrix.is_unitary());
    EXPECT_EQ(z_matrix(0, 0), Complex(1, 0));
    EXPECT_EQ(z_matrix(0, 1), Complex(0, 0));
    EXPECT_EQ(z_matrix(1, 0), Complex(0, 0));
    EXPECT_EQ(z_matrix(1, 1), Complex(-1, 0));
}

// 测试相位门
TEST(StandardGatesTest, PhaseGates) {
    // S门
    SGate s;
    EXPECT_EQ(s.get_name(), "S");
    Matrix s_matrix = s.get_matrix();
    EXPECT_TRUE(s_matrix.is_unitary());
    EXPECT_EQ(s_matrix(0, 0), Complex(1, 0));
    EXPECT_EQ(s_matrix(0, 1), Complex(0, 0));
    EXPECT_EQ(s_matrix(1, 0), Complex(0, 0));
    EXPECT_EQ(s_matrix(1, 1), Complex(0, 1));
    
    // T门
    TGate t;
    EXPECT_EQ(t.get_name(), "T");
    Matrix t_matrix = t.get_matrix();
    EXPECT_TRUE(t_matrix.is_unitary());
    EXPECT_EQ(t_matrix(0, 0), Complex(1, 0));
    EXPECT_EQ(t_matrix(0, 1), Complex(0, 0));
    EXPECT_EQ(t_matrix(1, 0), Complex(0, 0));
    EXPECT_NEAR(std::abs(t_matrix(1, 1)), std::abs(std::exp(Complex(0, M_PI/4))), 1e-10);
}

// 测试旋转门
TEST(StandardGatesTest, RotationGates) {
    const double theta = M_PI/4;
    
    // Rx门
    RotationGate rx(RotationGate::Axis::X);
    EXPECT_EQ(rx.get_name(), "Rx");
    EXPECT_EQ(rx.get_param_count(), 1);
    Matrix rx_matrix = rx.get_matrix({theta});
    EXPECT_TRUE(rx_matrix.is_unitary());
    
    // Ry门
    RotationGate ry(RotationGate::Axis::Y);
    EXPECT_EQ(ry.get_name(), "Ry");
    EXPECT_EQ(ry.get_param_count(), 1);
    Matrix ry_matrix = ry.get_matrix({theta});
    EXPECT_TRUE(ry_matrix.is_unitary());
    
    // Rz门
    RotationGate rz(RotationGate::Axis::Z);
    EXPECT_EQ(rz.get_name(), "Rz");
    EXPECT_EQ(rz.get_param_count(), 1);
    Matrix rz_matrix = rz.get_matrix({theta});
    EXPECT_TRUE(rz_matrix.is_unitary());
    
    // 测试参数缺失
    EXPECT_THROW(rx.get_matrix(), std::runtime_error);
}

// 测试双量子比特门
TEST(StandardGatesTest, TwoQubitGates) {
    // CNOT门
    CNOTGate cnot;
    EXPECT_EQ(cnot.get_name(), "CNOT");
    EXPECT_EQ(cnot.get_qubit_count(), 2);
    Matrix cnot_matrix = cnot.get_matrix();
    EXPECT_TRUE(cnot_matrix.is_unitary());
    
    // SWAP门
    SWAPGate swap;
    EXPECT_EQ(swap.get_name(), "SWAP");
    EXPECT_EQ(swap.get_qubit_count(), 2);
    Matrix swap_matrix = swap.get_matrix();
    EXPECT_TRUE(swap_matrix.is_unitary());
    
    // CZ门
    CZGate cz;
    EXPECT_EQ(cz.get_name(), "CZ");
    EXPECT_EQ(cz.get_qubit_count(), 2);
    Matrix cz_matrix = cz.get_matrix();
    EXPECT_TRUE(cz_matrix.is_unitary());
}

// 测试三量子比特门
TEST(StandardGatesTest, ThreeQubitGates) {
    // Toffoli门
    ToffoliGate toffoli;
    EXPECT_EQ(toffoli.get_name(), "CCNOT");
    EXPECT_EQ(toffoli.get_qubit_count(), 3);
    Matrix toffoli_matrix = toffoli.get_matrix();
    EXPECT_TRUE(toffoli_matrix.is_unitary());
}

// 测试Hadamard门的矩阵
TEST(GateTest, HadamardMatrix) {
    HadamardGate h;
    Matrix m = h.get_matrix();
    
    const double h_val = 1.0 / std::sqrt(2.0);
    Matrix expected({
        {Complex(h_val, 0), Complex(h_val, 0)},
        {Complex(h_val, 0), Complex(-h_val, 0)}
    });
    
    EXPECT_TRUE(matrices_are_close(m, expected));
}

// 测试Pauli-X门的矩阵
TEST(GateTest, PauliXMatrix) {
    PauliXGate x;
    Matrix m = x.get_matrix();
    
    Matrix expected({
        {Complex(0, 0), Complex(1, 0)},
        {Complex(1, 0), Complex(0, 0)}
    });
    
    EXPECT_TRUE(matrices_are_close(m, expected));
}

// 测试Pauli-X门的自反性（X² = I）
TEST(GateTest, PauliXSelfInverse) {
    PauliXGate x;
    Matrix m = x.get_matrix();
    Matrix result = m * m;  // X²
    
    Matrix identity({
        {Complex(1, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(1, 0)}
    });
    
    EXPECT_TRUE(matrices_are_close(result, identity));
}

// 测试CNOT门的矩阵
TEST(GateTest, CNOTMatrix) {
    CNOTGate cnot;
    Matrix m = cnot.get_matrix();
    
    Matrix expected({
        {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(1, 0)},
        {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)}
    });
    
    EXPECT_TRUE(matrices_are_close(m, expected));
}

// 测试旋转门Rx
TEST(GateTest, RxGate) {
    // 测试0度旋转
    RxGate rx0(0.0);
    Matrix m0 = rx0.get_matrix();
    Matrix identity({
        {Complex(1, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(1, 0)}
    });
    EXPECT_TRUE(matrices_are_close(m0, identity));
    
    // 测试π/2旋转
    RxGate rx_pi2(M_PI/2);
    Matrix m_pi2 = rx_pi2.get_matrix();
    Matrix expected_pi2({
        {Complex(0.7071067811865476, 0), Complex(0, -0.7071067811865476)},
        {Complex(0, -0.7071067811865476), Complex(0.7071067811865476, 0)}
    });
    EXPECT_TRUE(matrices_are_close(m_pi2, expected_pi2));
}

// 测试旋转门Ry
TEST(GateTest, RyGate) {
    // 测试π旋转
    RyGate ry_pi(M_PI);
    Matrix m_pi = ry_pi.get_matrix();
    Matrix expected_pi({
        {Complex(0, 0), Complex(-1, 0)},
        {Complex(1, 0), Complex(0, 0)}
    });
    EXPECT_TRUE(matrices_are_close(m_pi, expected_pi));
}

// 测试旋转门Rz
TEST(GateTest, RzGate) {
    // 测试π/4旋转
    RzGate rz_pi4(M_PI/4);
    Matrix m_pi4 = rz_pi4.get_matrix();
    Complex phase = std::exp(Complex(0, -M_PI/8));
    Matrix expected_pi4({
        {phase, Complex(0, 0)},
        {Complex(0, 0), std::conj(phase)}
    });
    EXPECT_TRUE(matrices_are_close(m_pi4, expected_pi4));
}

// 测试门的幺正性
TEST(GateTest, Unitarity) {
    // 测试Hadamard门的幺正性
    HadamardGate h;
    Matrix m = h.get_matrix();
    Matrix m_dagger = m.adjoint();
    Matrix result = m * m_dagger;
    
    Matrix identity({
        {Complex(1, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(1, 0)}
    });
    
    EXPECT_TRUE(matrices_are_close(result, identity));
}

// 测试Toffoli门
TEST(GateTest, ToffoliGate) {
    ToffoliGate toffoli;
    Matrix m = toffoli.get_matrix();
    
    // 验证矩阵大小
    EXPECT_EQ(m.rows(), 8);
    EXPECT_EQ(m.cols(), 8);
    
    // 验证特定元素
    EXPECT_TRUE(is_close(m(6, 6), Complex(0, 0)));
    EXPECT_TRUE(is_close(m(6, 7), Complex(1, 0)));
    EXPECT_TRUE(is_close(m(7, 6), Complex(1, 0)));
    EXPECT_TRUE(is_close(m(7, 7), Complex(0, 0)));
}

// 测试SWAP门
TEST(GateTest, SWAPGate) {
    SWAPGate swap;
    Matrix m = swap.get_matrix();
    
    Matrix result = m * m;
    Matrix identity = Matrix::identity(4);
    EXPECT_TRUE(matrices_are_close(result, identity));
}

// 测试CZ门
TEST(GateTest, CZGate) {
    CZGate cz;
    Matrix m = cz.get_matrix();
    
    Matrix result = m * m;
    Matrix identity = Matrix::identity(4);
    EXPECT_TRUE(matrices_are_close(result, identity));
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 