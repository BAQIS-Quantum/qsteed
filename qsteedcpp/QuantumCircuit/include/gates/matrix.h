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
        Matrix() : data(0, 0) {}

        Matrix(size_t rows, size_t cols) : data(rows, cols) {
            data.setZero();
        }

        Matrix(const MatrixXcd& matrix) : data(matrix) {}

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

        size_t rows() const { return data.rows(); }
        size_t cols() const { return data.cols(); }
        size_t get_rows() const { return data.rows(); }
        size_t get_cols() const { return data.cols(); }

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

        Matrix transpose() const {
            return Matrix(data.transpose());
        }

        Matrix conjugate() const {
            return Matrix(data.conjugate());
        }

        Matrix conjugate_transpose() const {
            return Matrix(data.adjoint());
        }

        Matrix dagger() const {
            return conjugate_transpose();
        }

        Complex determinant() const {
            if (data.rows() != data.cols()) {
                throw std::runtime_error("Determinant only defined for square matrices");
            }
            return data.determinant();
        }

        Complex trace() const {
            if (data.rows() != data.cols()) {
                throw std::runtime_error("Trace only defined for square matrices");
            }
            return data.trace();
        }

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

        Matrix inverse() const {
            if (data.rows() != data.cols()) {
                throw std::runtime_error("Inverse only defined for square matrices");
            }
            return Matrix(data.inverse());
        }

        Matrix exp() const {
            if (data.rows() != data.cols()) {
                throw std::runtime_error("Matrix exponential only defined for square matrices");
            }

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

        bool is_unitary(double epsilon = 1e-10) const {
            if (data.rows() != data.cols()) return false;

            MatrixXcd product = data * data.adjoint();
            MatrixXcd identity = MatrixXcd::Identity(data.rows(), data.cols());

            return (product - identity).norm() < epsilon;
        }

        bool is_hermitian(double epsilon = 1e-10) const {
            if (data.rows() != data.cols()) return false;
            return (data - data.adjoint()).norm() < epsilon;
        }

        bool is_diagonal(double epsilon = 1e-10) const {
            if (data.rows() != data.cols()) return false;

            MatrixXcd diag_matrix = data.diagonal().asDiagonal();
            return (data - diag_matrix).norm() < epsilon;
        }

        VectorXcd diagonal() const {
            return data.diagonal();
        }

        void print() const {
            std::cout << data << std::endl;
        }


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

    // --- Kronecker Product ---
    // Computes the tensor product of two matrices A and B.
    inline Matrix kronecker_product(const Matrix& A, const Matrix& B) {
        size_t a_rows = A.rows();
        size_t a_cols = A.cols();
        size_t b_rows = B.rows();
        size_t b_cols = B.cols();

        Matrix C(a_rows * b_rows, a_cols * b_cols);
        const auto& a_eigen = A.eigen_matrix();
        const auto& b_eigen = B.eigen_matrix();
        auto& c_eigen = C.eigen_matrix();

        for (size_t i = 0; i < a_rows; ++i) {
            for (size_t j = 0; j < a_cols; ++j) {
                c_eigen.block(i * b_rows, j * b_cols, b_rows, b_cols) = a_eigen(i, j) * b_eigen;
            }
        }
        return C;
    }

} // namespace qsteedcpp