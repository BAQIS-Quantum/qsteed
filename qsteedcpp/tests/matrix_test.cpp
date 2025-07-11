#include <gtest/gtest.h>
#include <cmath>
#include "Gates/include/matrix.h"

namespace qsteedcpp {
namespace Test {

class MatrixTest : public ::testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }

};


// 测试单位矩阵
TEST_F(MatrixTest, Identity) {
    Matrix id = Matrix::identity(2);
    EXPECT_EQ(id.get_rows(), 2);
    EXPECT_EQ(id.get_cols(), 2);
    EXPECT_DOUBLE_EQ(id(0, 0).real(), 1.0);
    EXPECT_DOUBLE_EQ(id(0, 1).real(), 0.0);
    EXPECT_DOUBLE_EQ(id(1, 0).real(), 0.0);
    EXPECT_DOUBLE_EQ(id(1, 1).real(), 1.0);
}

// 测试零矩阵
TEST_F(MatrixTest, Zeros) {
    Matrix zeros = Matrix::zeros(2, 2);
    EXPECT_EQ(zeros.get_rows(), 2);
    EXPECT_EQ(zeros.get_cols(), 2);
    EXPECT_DOUBLE_EQ(zeros(0, 0).real(), 0.0);
    EXPECT_DOUBLE_EQ(zeros(0, 1).real(), 0.0);
    EXPECT_DOUBLE_EQ(zeros(1, 0).real(), 0.0);
    EXPECT_DOUBLE_EQ(zeros(1, 1).real(), 0.0);
}

// 测试矩阵乘法
TEST_F(MatrixTest, Multiplication) {
    // 创建 X 门和 Y 门
    Matrix x_gate(std::vector<std::vector<Complex>>{
        {{0.0, 0.0}, {1.0, 0.0}},
        {{1.0, 0.0}, {0.0, 0.0}}
    });
    
    Matrix y_gate(std::vector<std::vector<Complex>>{
        {{0.0, 0.0}, {0.0, -1.0}},
        {{0.0, 1.0}, {0.0, 0.0}}
    });

    // 测试 X * Y = iZ
    Matrix result = x_gate * y_gate;
    EXPECT_NEAR(result(0, 0).real(), 0.0, 1e-10);
    EXPECT_NEAR(result(0, 0).imag(), 1.0, 1e-10);
    EXPECT_NEAR(result(0, 1).real(), 0.0, 1e-10);
    EXPECT_NEAR(result(0, 1).imag(), 0.0, 1e-10);
    EXPECT_NEAR(result(1, 0).real(), 0.0, 1e-10);
    EXPECT_NEAR(result(1, 0).imag(), 0.0, 1e-10);
    EXPECT_NEAR(result(1, 1).real(), 0.0, 1e-10);
    EXPECT_NEAR(result(1, 1).imag(), -1.0, 1e-10);
}

// 测试矩阵加法
TEST_F(MatrixTest, Addition) {
    Matrix m1(std::vector<std::vector<Complex>>{
        {{1.0, 0.0}, {2.0, 0.0}},
        {{3.0, 0.0}, {4.0, 0.0}}
    });
    
    Matrix m2(std::vector<std::vector<Complex>>{
        {{5.0, 0.0}, {6.0, 0.0}},
        {{7.0, 0.0}, {8.0, 0.0}}
    });

    Matrix result = m1 + m2;
    EXPECT_DOUBLE_EQ(result(0, 0).real(), 6.0);
    EXPECT_DOUBLE_EQ(result(0, 1).real(), 8.0);
    EXPECT_DOUBLE_EQ(result(1, 0).real(), 10.0);
    EXPECT_DOUBLE_EQ(result(1, 1).real(), 12.0);
}

// 测试标量乘法
TEST_F(MatrixTest, ScalarMultiplication) {
    Matrix m(std::vector<std::vector<Complex>>{
        {{1.0, 0.0}, {2.0, 0.0}},
        {{3.0, 0.0}, {4.0, 0.0}}
    });

    std::complex<double> scalar(2.0, 0.0);
    Matrix result = m * scalar;
    EXPECT_DOUBLE_EQ(result(0, 0).real(), 2.0);
    EXPECT_DOUBLE_EQ(result(0, 1).real(), 4.0);
    EXPECT_DOUBLE_EQ(result(1, 0).real(), 6.0);
    EXPECT_DOUBLE_EQ(result(1, 1).real(), 8.0);
}

// 测试共轭转置
TEST_F(MatrixTest, ConjugateTranspose) {
    Matrix m(std::vector<std::vector<Complex>>{
        {{1.0, 2.0}, {3.0, 4.0}},
        {{5.0, 6.0}, {7.0, 8.0}}
    });

    Matrix result = m.conjugate_transpose();
    EXPECT_DOUBLE_EQ(result(0, 0).real(), 1.0);
    EXPECT_DOUBLE_EQ(result(0, 0).imag(), -2.0);
    EXPECT_DOUBLE_EQ(result(0, 1).real(), 5.0);
    EXPECT_DOUBLE_EQ(result(0, 1).imag(), -6.0);
    EXPECT_DOUBLE_EQ(result(1, 0).real(), 3.0);
    EXPECT_DOUBLE_EQ(result(1, 0).imag(), -4.0);
    EXPECT_DOUBLE_EQ(result(1, 1).real(), 7.0);
    EXPECT_DOUBLE_EQ(result(1, 1).imag(), -8.0);
}

// 测试酉性
TEST_F(MatrixTest, Unitarity) {
    // 测试单位矩阵（是酉矩阵）
    Matrix id = Matrix::identity(2);
    EXPECT_TRUE(id.is_unitary());

    // 测试 Hadamard 门（是酉矩阵）
    Matrix h_gate(std::vector<std::vector<Complex>>{
        {{1.0/sqrt(2), 0.0}, {1.0/sqrt(2), 0.0}},
        {{1.0/sqrt(2), 0.0}, {-1.0/sqrt(2), 0.0}}
    });
    EXPECT_TRUE(h_gate.is_unitary());

    // 测试非酉矩阵
    Matrix non_unitary(std::vector<std::vector<Complex>>{
        {{1.0, 0.0}, {2.0, 0.0}},
        {{3.0, 0.0}, {4.0, 0.0}}
    });
    EXPECT_FALSE(non_unitary.is_unitary());
}

// 测试异常处理
TEST_F(MatrixTest, ExceptionHandling) {
    // 测试不规则矩阵
    std::vector<std::vector<std::complex<double>>> irregular = {
        {{1.0, 0.0}, {2.0, 0.0}},
        {{3.0, 0.0}}  // 缺少一个元素
    };
    EXPECT_THROW(Matrix m(irregular), std::runtime_error);

}

} // namespace Test
} // namespace qsteedcpp 