#include <gtest/gtest.h>
#include <complex>
#include <vector>
#include "matrix.h"


using namespace Gate;
using Complex = std::complex<double>;

// 测试矩阵构造函数
TEST(MatrixTest, Constructor) {
    // 测试空矩阵
    Matrix m1;
    EXPECT_EQ(m1.get_rows(), 0);
    EXPECT_EQ(m1.get_cols(), 0);

    // 测试复数矩阵
    std::vector<std::vector<Complex>> data = {
        {Complex(1, 0), Complex(0, 1)},
        {Complex(0, -1), Complex(1, 0)}
    };
    Matrix m2(data);
    EXPECT_EQ(m2.get_rows(), 2);
    EXPECT_EQ(m2.get_cols(), 2);
    EXPECT_EQ(m2(0, 0), Complex(1, 0));
    EXPECT_EQ(m2(0, 1), Complex(0, 1));

    // 测试实数矩阵
    std::vector<std::vector<double>> real_data = {
        {1.0, 0.0},
        {0.0, 1.0}
    };
    Matrix m3(real_data);
    EXPECT_EQ(m3.get_rows(), 2);
    EXPECT_EQ(m3.get_cols(), 2);
    EXPECT_EQ(m3(0, 0), Complex(1, 0));
    EXPECT_EQ(m3(1, 1), Complex(1, 0));
}

// 测试单位矩阵
TEST(MatrixTest, Identity) {
    Matrix id = Matrix::identity(2);
    EXPECT_EQ(id.get_rows(), 2);
    EXPECT_EQ(id.get_cols(), 2);
    EXPECT_EQ(id(0, 0), Complex(1, 0));
    EXPECT_EQ(id(0, 1), Complex(0, 0));
    EXPECT_EQ(id(1, 0), Complex(0, 0));
    EXPECT_EQ(id(1, 1), Complex(1, 0));
}

// 测试零矩阵
TEST(MatrixTest, Zeros) {
    Matrix zeros = Matrix::zeros(2, 3);
    EXPECT_EQ(zeros.get_rows(), 2);
    EXPECT_EQ(zeros.get_cols(), 3);
    for (size_t i = 0; i < 2; ++i) {
        for (size_t j = 0; j < 3; ++j) {
            EXPECT_EQ(zeros(i, j), Complex(0, 0));
        }
    }
}

// 测试矩阵乘法
TEST(MatrixTest, Multiplication) {
    Matrix m1({{Complex(1, 0), Complex(2, 0)},
               {Complex(3, 0), Complex(4, 0)}});
    Matrix m2({{Complex(5, 0), Complex(6, 0)},
               {Complex(7, 0), Complex(8, 0)}});
    Matrix result = m1 * m2;
    EXPECT_EQ(result(0, 0), Complex(19, 0));
    EXPECT_EQ(result(0, 1), Complex(22, 0));
    EXPECT_EQ(result(1, 0), Complex(43, 0));
    EXPECT_EQ(result(1, 1), Complex(50, 0));
}

// 测试矩阵加法
TEST(MatrixTest, Addition) {
    Matrix m1({{Complex(1, 0), Complex(2, 0)},
               {Complex(3, 0), Complex(4, 0)}});
    Matrix m2({{Complex(5, 0), Complex(6, 0)},
               {Complex(7, 0), Complex(8, 0)}});
    Matrix result = m1 + m2;
    EXPECT_EQ(result(0, 0), Complex(6, 0));
    EXPECT_EQ(result(0, 1), Complex(8, 0));
    EXPECT_EQ(result(1, 0), Complex(10, 0));
    EXPECT_EQ(result(1, 1), Complex(12, 0));
}

// 测试标量乘法
TEST(MatrixTest, ScalarMultiplication) {
    Matrix m({{Complex(1, 0), Complex(2, 0)},
              {Complex(3, 0), Complex(4, 0)}});
    Matrix result = m * Complex(2, 0);
    EXPECT_EQ(result(0, 0), Complex(2, 0));
    EXPECT_EQ(result(0, 1), Complex(4, 0));
    EXPECT_EQ(result(1, 0), Complex(6, 0));
    EXPECT_EQ(result(1, 1), Complex(8, 0));
}

// 测试共轭转置
TEST(MatrixTest, ConjugateTranspose) {
    Matrix m({{Complex(1, 1), Complex(2, 2)},
              {Complex(3, 3), Complex(4, 4)}});
    Matrix result = m.conjugate_transpose();
    EXPECT_EQ(result(0, 0), Complex(1, -1));
    EXPECT_EQ(result(0, 1), Complex(3, -3));
    EXPECT_EQ(result(1, 0), Complex(2, -2));
    EXPECT_EQ(result(1, 1), Complex(4, -4));
}

// 测试酉矩阵检查
TEST(MatrixTest, UnitaryCheck) {
    // 测试单位矩阵（是酉矩阵）
    Matrix id = Matrix::identity(2);
    EXPECT_TRUE(id.is_unitary());

    // 测试 Hadamard 门（是酉矩阵）
    const double h = 1.0 / std::sqrt(2.0);
    Matrix hadamard({{Complex(h, 0), Complex(h, 0)},
                     {Complex(h, 0), Complex(-h, 0)}});
    EXPECT_TRUE(hadamard.is_unitary());

    // 测试非酉矩阵
    Matrix non_unitary({{Complex(1, 0), Complex(2, 0)},
                        {Complex(3, 0), Complex(4, 0)}});
    EXPECT_FALSE(non_unitary.is_unitary());
}

// 测试异常处理
TEST(MatrixTest, ExceptionHandling) {
    // 测试不规则矩阵
    std::vector<std::vector<Complex>> irregular = {
        {Complex(1, 0), Complex(2, 0)},
        {Complex(3, 0)}
    };
    EXPECT_THROW(Matrix m(irregular), std::runtime_error);

    // 测试维度不匹配的矩阵乘法
    Matrix m1({{Complex(1, 0), Complex(2, 0)}});
    Matrix m2({{Complex(1, 0)},
               {Complex(2, 0)},
               {Complex(3, 0)}});
    EXPECT_THROW(m1 * m2, std::runtime_error);

    // 测试维度不匹配的矩阵加法
    Matrix m3({{Complex(1, 0), Complex(2, 0)}});
    Matrix m4({{Complex(1, 0), Complex(2, 0), Complex(3, 0)}});
    EXPECT_THROW(m3 + m4, std::runtime_error);

    // 测试越界访问
    Matrix m5({{Complex(1, 0), Complex(2, 0)}});
    EXPECT_THROW(m5(0, 2), std::out_of_range);
    EXPECT_THROW(m5(1, 0), std::out_of_range);
} 