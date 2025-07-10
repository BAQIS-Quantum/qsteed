#pragma once
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include <complex>
#include <iostream>
#include <vector>
#include <stdexcept>

namespace qsteedcpp {

using Complex = std::complex<double>;
using MatrixXcd = Eigen::Matrix<Complex, Eigen::Dynamic, Eigen::Dynamic>;
using VectorXcd = Eigen::Vector<Complex, Eigen::Dynamic>;

class Matrix {
private:
    MatrixXcd data;

public:
    // 默认构造函数
    Matrix() : data(0, 0) {}

    // 从行列数构造
    Matrix(size_t rows, size_t cols) : data(rows, cols) {
        data.setZero();
    }

    // 从Eigen矩阵构造
    Matrix(const MatrixXcd& matrix) : data(matrix) {}

    // 从复数vector构造
    Matrix(const std::vector<std::vector<Complex>>& matrix) {
        if (matrix.empty()) {
            data = MatrixXcd(0, 0);
            return;
        }

        size_t rows = matrix.size();
        size_t cols = matrix[0].size();

        // 检查矩阵是否规整
        for (const auto& row : matrix) {
            if (row.size() != cols) {
                throw std::runtime_error("Irregular matrix");
            }
        }

        data = MatrixXcd(rows, cols);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                data(i, j) = matrix[i][j];
            }
        }
    }

    // 从实数vector构造
    Matrix(const std::vector<std::vector<double>>& matrix) {
        if (matrix.empty()) {
            data = MatrixXcd(0, 0);
            return;
        }

        size_t rows = matrix.size();
        size_t cols = matrix[0].size();

        data = MatrixXcd(rows, cols);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                data(i, j) = Complex(matrix[i][j], 0.0);
            }
        }
    }

    // 创建单位矩阵
    static Matrix identity(size_t size) {
        Matrix result;
        result.data = MatrixXcd::Identity(size, size);
        return result;
    }

    // 创建零矩阵
    static Matrix zeros(size_t rows, size_t cols) {
        Matrix result;
        result.data = MatrixXcd::Zero(rows, cols);
        return result;
    }

    // 创建随机矩阵
    static Matrix random(size_t rows, size_t cols) {
        Matrix result;
        result.data = MatrixXcd::Random(rows, cols);
        return result;
    }

    // 获取维度 - 为绑定系统提供的兼容接口
    size_t rows() const { return data.rows(); }
    size_t cols() const { return data.cols(); }
    size_t get_rows() const { return data.rows(); }
    size_t get_cols() const { return data.cols(); }

    // 元素访问 - 为绑定系统提供的兼容接口
    Complex get_element(size_t row, size_t col) const {
        if (row >= rows() || col >= cols()) {
            throw std::out_of_range("Matrix index out of range");
        }
        return data(row, col);
    }

    void set_element(size_t row, size_t col, const Complex& value) {
        if (row >= rows() || col >= cols()) {
            throw std::out_of_range("Matrix index out of range");
        }
        data(row, col) = value;
    }

    Complex& operator()(size_t i, size_t j) {
        return data(i, j);
    }

    const Complex& operator()(size_t i, size_t j) const {
        return data(i, j);
    }

    // 获取Eigen矩阵引用（用于高级操作）
    MatrixXcd& eigen_matrix() { return data; }
    const MatrixXcd& eigen_matrix() const { return data; }

    // 矩阵运算
    Matrix operator*(const Matrix& other) const {
        return Matrix(data * other.data);
    }

    Matrix operator+(const Matrix& other) const {
        return Matrix(data + other.data);
    }

    Matrix operator-(const Matrix& other) const {
        return Matrix(data - other.data);
    }

    Matrix operator*(const Complex& scalar) const {
        return Matrix(data * scalar);
    }

    // 共轭转置
    Matrix conjugate_transpose() const {
        return Matrix(data.adjoint());
    }

    // 转置
    Matrix transpose() const {
        return Matrix(data.transpose());
    }

    // 共轭转置（别名，为绑定系统提供）
    Matrix dagger() const {
        return conjugate_transpose();
    }

    // 共轭
    Matrix conjugate() const {
        return Matrix(data.conjugate());
    }

    // 行列式
    Complex determinant() const {
        if (data.rows() != data.cols()) {
            throw std::runtime_error("Determinant only defined for square matrices");
        }
        return data.determinant();
    }

    // 迹
    Complex trace() const {
        if (data.rows() != data.cols()) {
            throw std::runtime_error("Trace only defined for square matrices");
        }
        return data.trace();
    }

    // 范数
    double norm() const {
        return data.norm();
    }

    double frobenius_norm() const {
        return data.norm();
    }

    // 特征值分解
    struct EigenDecomposition {
        VectorXcd eigenvalues;
        MatrixXcd eigenvectors;
    };

    EigenDecomposition eigendecomposition() const {
        if (data.rows() != data.cols()) {
            throw std::runtime_error("Eigendecomposition only defined for square matrices");
        }

        Eigen::ComplexEigenSolver<MatrixXcd> solver(data);
        EigenDecomposition result;
        result.eigenvalues = solver.eigenvalues();
        result.eigenvectors = solver.eigenvectors();
        return result;
    }

    // SVD分解
    struct SVD {
        MatrixXcd U;
        Eigen::VectorXd singularValues;
        MatrixXcd V;
    };

    SVD svd() const {
        Eigen::JacobiSVD<MatrixXcd> svd(data, Eigen::ComputeFullU | Eigen::ComputeFullV);
        SVD result;
        result.U = svd.matrixU();
        result.singularValues = svd.singularValues();
        result.V = svd.matrixV();
        return result;
    }

    // 矩阵求逆
    Matrix inverse() const {
        if (data.rows() != data.cols()) {
            throw std::runtime_error("Inverse only defined for square matrices");
        }
        return Matrix(data.inverse());
    }

    // 矩阵指数
    Matrix exp() const {
        if (data.rows() != data.cols()) {
            throw std::runtime_error("Matrix exponential only defined for square matrices");
        }

        // 使用特征值分解计算矩阵指数
        auto eig = eigendecomposition();
        VectorXcd exp_eigenvals = eig.eigenvalues.array().exp();
        return Matrix(eig.eigenvectors * exp_eigenvals.asDiagonal() * eig.eigenvectors.inverse());
    }

    // 矩阵对数
    Matrix log() const {
        if (data.rows() != data.cols()) {
            throw std::runtime_error("Matrix logarithm only defined for square matrices");
        }

        auto eig = eigendecomposition();
        VectorXcd log_eigenvals = eig.eigenvalues.array().log();
        return Matrix(eig.eigenvectors * log_eigenvals.asDiagonal() * eig.eigenvectors.inverse());
    }

    // 检查是否是酉矩阵（改进版本）
    bool is_unitary(double epsilon = 1e-10) const {
        if (data.rows() != data.cols()) return false;

        MatrixXcd product = data * data.adjoint();
        MatrixXcd identity = MatrixXcd::Identity(data.rows(), data.cols());

        return (product - identity).norm() < epsilon;
    }

    // 检查是否是厄米矩阵
    bool is_hermitian(double epsilon = 1e-10) const {
        if (data.rows() != data.cols()) return false;
        return (data - data.adjoint()).norm() < epsilon;
    }

    // 检查是否是对角矩阵
    bool is_diagonal(double epsilon = 1e-10) const {
        if (data.rows() != data.cols()) return false;

        MatrixXcd diag_matrix = data.diagonal().asDiagonal();
        return (data - diag_matrix).norm() < epsilon;
    }

    // 获取对角元素
    VectorXcd diagonal() const {
        return data.diagonal();
    }

    // 打印矩阵
    void print() const {
        std::cout << data << std::endl;
    }

    // 保存为Matlab格式
    void print_matlab() const {
        std::cout << "[";
        for (int i = 0; i < data.rows(); ++i) {
            for (int j = 0; j < data.cols(); ++j) {
                std::cout << data(i, j);
                if (j < data.cols() - 1) std::cout << ", ";
            }
            if (i < data.rows() - 1) std::cout << "; ";
        }
        std::cout << "]" << std::endl;
    }

    // 为绑定系统提供的数据访问方法
    std::vector<std::vector<Complex>> get_data() const {
        std::vector<std::vector<Complex>> result(rows());
        for (size_t i = 0; i < rows(); ++i) {
            result[i].resize(cols());
            for (size_t j = 0; j < cols(); ++j) {
                result[i][j] = data(i, j);
            }
        }
        return result;
    }
};

} // namespace qsteedcpp