#pragma once
#include <vector>
#include <complex>
#include <stdexcept>
#include <cmath>

namespace Gate {
using Complex = std::complex<double>;

class Matrix {
private:
    std::vector<std::vector<Complex>> data;
    size_t rows;
    size_t cols;

public:

    Matrix() : rows(0), cols(0) {}
    
    Matrix(const std::vector<std::vector<Complex>>& matrix) 
        : data(matrix), rows(matrix.size()), cols(matrix.empty() ? 0 : matrix[0].size()) {
        for (const auto& row : matrix) {
            if (row.size() != cols) {
                throw std::runtime_error("Irregular matrix");
            }
        }
    }
    
    Matrix(const std::vector<std::vector<double>>& matrix) 
        : rows(matrix.size()), cols(matrix.empty() ? 0 : matrix[0].size()) {
        data.resize(rows);
        for (size_t i = 0; i < rows; ++i) {
            data[i].resize(cols);
            for (size_t j = 0; j < cols; ++j) {
                data[i][j] = Complex(matrix[i][j], 0.0);
            }
        }
    }
    
    // 建单位矩阵
    static Matrix identity(size_t size) {
        Matrix result;
        result.rows = result.cols = size;
        result.data.resize(size, std::vector<Complex>(size, Complex(0.0, 0.0)));
        for (size_t i = 0; i < size; ++i) {
            result.data[i][i] = Complex(1.0, 0.0);
        }
        return result;
    }
    
    // 创建零矩阵
    static Matrix zeros(size_t rows, size_t cols) {
        Matrix result;
        result.rows = rows;
        result.cols = cols;
        result.data.resize(rows, std::vector<Complex>(cols, Complex(0.0, 0.0)));
        return result;
    }
    
    size_t get_rows() const { return rows; }
    size_t get_cols() const { return cols; }
    
    Complex& operator()(size_t i, size_t j) {
        if (i >= rows || j >= cols) {
            throw std::out_of_range("Matrix index out of range");
        }
        return data[i][j];
    }
    
    const Complex& operator()(size_t i, size_t j) const {
        if (i >= rows || j >= cols) {
            throw std::out_of_range("Matrix index out of range");
        }
        return data[i][j];
    }
    
    // 矩阵乘法
    Matrix operator*(const Matrix& other) const {
        if (cols != other.rows) {
            throw std::runtime_error("Matrix dimensions do not match for multiplication");
        }
        
        Matrix result = Matrix::zeros(rows, other.cols);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < other.cols; ++j) {
                Complex sum(0.0, 0.0);
                for (size_t k = 0; k < cols; ++k) {
                    sum += data[i][k] * other.data[k][j];
                }
                result.data[i][j] = sum;
            }
        }
        return result;
    }
    
    // 矩阵加法
    Matrix operator+(const Matrix& other) const {
        if (rows != other.rows || cols != other.cols) {
            throw std::runtime_error("Matrix dimensions do not match for addition");
        }
        
        Matrix result = Matrix::zeros(rows, cols);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                result.data[i][j] = data[i][j] + other.data[i][j];
            }
        }
        return result;
    }
    
    // 标量乘法
    Matrix operator*(const Complex& scalar) const {
        Matrix result = Matrix::zeros(rows, cols);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                result.data[i][j] = data[i][j] * scalar;
            }
        }
        return result;
    }
    
    // 共轭转置
    Matrix conjugate_transpose() const {
        Matrix result = Matrix::zeros(cols, rows);
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                result.data[j][i] = std::conj(data[i][j]);
            }
        }
        return result;
    }
    
    // 检查是否是酉矩阵（量子门必须是酉矩阵）
    bool is_unitary() const {
        if (rows != cols) return false;
        
        Matrix product = (*this) * this->conjugate_transpose();
        Matrix identity = Matrix::identity(rows);
        
        // 检查是否接近单位矩阵
        const double epsilon = 1e-10;
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                if (std::abs(product(i, j) - identity(i, j)) > epsilon) {
                    return false;
                }
            }
        }
        return true;
    }
    

    void print() const {
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                std::cout << data[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }
};


} // namespace Gate