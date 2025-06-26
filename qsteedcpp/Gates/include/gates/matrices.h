#pragma once
#include "../matrix.h"
#include <cmath>

namespace qsteedcpp {

// 基础矩阵常量
namespace matrices {

// 单比特门矩阵
inline Matrix IdMatrix() {
    return Matrix::identity(2);
}

inline Matrix XMatrix() {
    return Matrix({
        {Complex(0, 0), Complex(1, 0)},
        {Complex(1, 0), Complex(0, 0)}
    });
}

inline Matrix YMatrix() {
    return Matrix({
        {Complex(0, 0), Complex(0, -1)},
        {Complex(0, 1), Complex(0, 0)}
    });
}

inline Matrix ZMatrix() {
    return Matrix({
        {Complex(1, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(-1, 0)}
    });
}

inline Matrix SMatrix() {
    return Matrix({
        {Complex(1, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(0, 1)}
    });
}

inline Matrix SXMatrix() {
    const double h = 1.0 / std::sqrt(2.0);
    return Matrix({
        {Complex(h, 0), Complex(0, h)},
        {Complex(0, h), Complex(h, 0)}
    });
}

inline Matrix SYMatrix() {
    const double h = 1.0 / std::sqrt(2.0);
    return Matrix({
        {Complex(h, 0), Complex(-h, 0)},
        {Complex(h, 0), Complex(h, 0)}
    });
}

inline Matrix TMatrix() {
    const Complex phase = std::exp(Complex(0, M_PI/4));
    return Matrix({
        {Complex(1, 0), Complex(0, 0)},
        {Complex(0, 0), phase}
    });
}

inline Matrix WMatrix() {
    const double h = 1.0 / std::sqrt(2.0);
    return Matrix({
        {Complex(h, 0), Complex(h, 0)},
        {Complex(h, 0), Complex(-h, 0)}
    });
}

inline Matrix SWMatrix() {
    const double h = 1.0 / std::sqrt(2.0);
    return Matrix({
        {Complex(0.5, 0.5), Complex(0, -h)},
        {Complex(h, 0), Complex(0.5, 0.5)}
    });
}

inline Matrix HMatrix() {
    const double h = 1.0 / std::sqrt(2.0);
    return Matrix({
        {Complex(h, 0), Complex(h, 0)},
        {Complex(h, 0), Complex(-h, 0)}
    });
}

// 双比特门矩阵
inline Matrix SwapMatrix() {
    return Matrix({
        {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(1, 0)}
    });
}

inline Matrix ISwapMatrix() {
    return Matrix({
        {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(0, 0), Complex(0, 1), Complex(0, 0)},
        {Complex(0, 0), Complex(0, 1), Complex(0, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(1, 0)}
    });
}

inline Matrix CXMatrix() {
    return Matrix({
        {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(1, 0)},
        {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)}
    });
}

inline Matrix CYMatrix() {
    return Matrix({
        {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(0, -1)},
        {Complex(0, 0), Complex(0, 0), Complex(0, 1), Complex(0, 0)}
    });
}

inline Matrix CZMatrix() {
    return Matrix({
        {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(-1, 0)}
    });
}

inline Matrix CTMatrix() {
    const Complex phase = std::exp(Complex(0, M_PI/4));
    return Matrix({
        {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(0, 0), Complex(0, 0), phase}
    });
}

// 三比特门矩阵
inline Matrix ToffoliMatrix() {
    Matrix result = Matrix::identity(8);
    result(6, 6) = Complex(0, 0);
    result(6, 7) = Complex(1, 0);
    result(7, 6) = Complex(1, 0);
    result(7, 7) = Complex(0, 0);
    return result;
}

inline Matrix FredkinMatrix() {
    Matrix result = Matrix::identity(8);
    result(5, 5) = Complex(0, 0);
    result(5, 6) = Complex(1, 0);
    result(6, 5) = Complex(1, 0);
    result(6, 6) = Complex(0, 0);
    return result;
}

// 参数化矩阵函数
inline Matrix rx_mat(double theta) {
    double cos_half = std::cos(theta / 2.0);
    double sin_half = std::sin(theta / 2.0);
    
    return Matrix({
        {Complex(cos_half, 0), Complex(0, -sin_half)},
        {Complex(0, -sin_half), Complex(cos_half, 0)}
    });
}

inline Matrix ry_mat(double theta) {
    double cos_half = std::cos(theta / 2.0);
    double sin_half = std::sin(theta / 2.0);
    
    return Matrix({
        {Complex(cos_half, 0), Complex(-sin_half, 0)},
        {Complex(sin_half, 0), Complex(cos_half, 0)}
    });
}

inline Matrix rz_mat(double theta) {
    Complex phase1 = std::exp(Complex(0, -theta / 2.0));
    Complex phase2 = std::exp(Complex(0, theta / 2.0));
    
    return Matrix({
        {phase1, Complex(0, 0)},
        {Complex(0, 0), phase2}
    });
}

inline Matrix pmatrix(double lambda) {
    Complex phase = std::exp(Complex(0, lambda));
    return Matrix({
        {Complex(1, 0), Complex(0, 0)},
        {Complex(0, 0), phase}
    });
}

inline Matrix rxx_mat(double theta) {
    double cos_half = std::cos(theta / 2.0);
    double sin_half = std::sin(theta / 2.0);
    
    return Matrix({
        {Complex(cos_half, 0), Complex(0, 0), Complex(0, 0), Complex(0, -sin_half)},
        {Complex(0, 0), Complex(cos_half, 0), Complex(0, -sin_half), Complex(0, 0)},
        {Complex(0, 0), Complex(0, -sin_half), Complex(cos_half, 0), Complex(0, 0)},
        {Complex(0, -sin_half), Complex(0, 0), Complex(0, 0), Complex(cos_half, 0)}
    });
}

inline Matrix ryy_mat(double theta) {
    double cos_half = std::cos(theta / 2.0);
    double sin_half = std::sin(theta / 2.0);
    
    return Matrix({
        {Complex(cos_half, 0), Complex(0, 0), Complex(0, 0), Complex(0, sin_half)},
        {Complex(0, 0), Complex(cos_half, 0), Complex(0, -sin_half), Complex(0, 0)},
        {Complex(0, 0), Complex(0, -sin_half), Complex(cos_half, 0), Complex(0, 0)},
        {Complex(0, sin_half), Complex(0, 0), Complex(0, 0), Complex(cos_half, 0)}
    });
}

inline Matrix rzz_mat(double theta) {
    Complex phase1 = std::exp(Complex(0, -theta / 2.0));
    Complex phase2 = std::exp(Complex(0, theta / 2.0));
    
    return Matrix({
        {phase1, Complex(0, 0), Complex(0, 0), Complex(0, 0)},
        {Complex(0, 0), phase2, Complex(0, 0), Complex(0, 0)},
        {Complex(0, 0), Complex(0, 0), phase2, Complex(0, 0)},
        {Complex(0, 0), Complex(0, 0), Complex(0, 0), phase1}
    });
}

inline Matrix u2matrix(double phi = 0.0, double lambda = 0.0) {
    Complex exp_phi = std::exp(Complex(0, phi));
    Complex exp_lambda = std::exp(Complex(0, lambda));
    Complex exp_sum = std::exp(Complex(0, phi + lambda));
    
    return Matrix({
        {Complex(1, 0), std::conj(exp_lambda)},
        {exp_phi, exp_sum}
    });
}

inline Matrix u3matrix(double theta = 0.0, double phi = 0.0, double lambda = 0.0) {
    double cos_half = std::cos(theta / 2.0);
    double sin_half = std::sin(theta / 2.0);
    Complex exp_phi = std::exp(Complex(0, phi));
    Complex exp_lambda = std::exp(Complex(0, lambda));
    Complex exp_sum = std::exp(Complex(0, phi + lambda));
    
    return Matrix({
        {Complex(cos_half, 0), -exp_lambda * Complex(sin_half, 0)},
        {exp_phi * Complex(sin_half, 0), exp_sum * Complex(cos_half, 0)}
    });
}

} // namespace matrices

} // namespace qsteedcpp 