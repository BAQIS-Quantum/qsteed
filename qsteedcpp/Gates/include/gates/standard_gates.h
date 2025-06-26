#pragma once
#include <cmath>
#include <complex>
#include "base_gate.h"
#include "matrix.h"


namespace qsteedcpp {

// 单量子比特门
class HadamardGate : public QuantumGate {
public:
    static constexpr const char* NAME = "H";
    static constexpr int QUBIT_COUNT = 1;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = []() {
            const double h = 1.0 / std::sqrt(2.0);
            return Matrix({
                {Complex(h, 0), Complex(h, 0)},
                {Complex(h, 0), Complex(-h, 0)}
            });
        }();
        return matrix;
    }
    
    Matrix get_matrix(const std::map<std::string, double>& parameters) const override {
        return get_matrix(); // Hadamard门没有参数
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
    
};

class PauliXGate : public QuantumGate {
public:
    static constexpr const char* NAME = "X";
    static constexpr int QUBIT_COUNT = 1;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = Matrix({
            {Complex(0, 0), Complex(1, 0)},
            {Complex(1, 0), Complex(0, 0)}
        });
        return matrix;
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }

};

} // namespace qsteedcpp 