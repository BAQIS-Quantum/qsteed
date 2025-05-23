#pragma once
#include <cmath>
#include <complex>
#include "base_gate.h"
#include "matrix.h"





namespace Gate {

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

class PauliYGate : public QuantumGate {
public:
    static constexpr const char* NAME = "Y";
    static constexpr int QUBIT_COUNT = 1;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = Matrix({
            {Complex(0, 0), Complex(0, -1)},
            {Complex(0, 1), Complex(0, 0)}
        });
        return matrix;
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
};

class PauliZGate : public QuantumGate {
public:
    static constexpr const char* NAME = "Z";
    static constexpr int QUBIT_COUNT = 1;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = Matrix({
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(-1, 0)}
        });
        return matrix;
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
};

class SGate : public QuantumGate {
public:
    static constexpr const char* NAME = "S";
    static constexpr int QUBIT_COUNT = 1;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = Matrix({
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 1)}
        });
        return matrix;
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
};


class TGate : public QuantumGate {
public:
    static constexpr const char* NAME = "T";
    static constexpr int QUBIT_COUNT = 1;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = []() {
            const Complex phase = std::exp(Complex(0, M_PI/4));
            return Matrix({
                {Complex(1, 0), Complex(0, 0)},
                {Complex(0, 0), phase}
            });
        }();
        return matrix;
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
};



class CNOTGate : public QuantumGate {
public:
    static constexpr const char* NAME = "CNOT";
    static constexpr int QUBIT_COUNT = 2;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = Matrix({
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(1, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)}
        });
        return matrix;
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
};


class SWAPGate : public QuantumGate {
public:
    static constexpr const char* NAME = "SWAP";
    static constexpr int QUBIT_COUNT = 2;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = Matrix({
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(1, 0)}
        });
        return matrix;
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
};


class CZGate : public QuantumGate {
public:
    static constexpr const char* NAME = "CZ";
    static constexpr int QUBIT_COUNT = 2;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = Matrix({
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(-1, 0)}
        });
        return matrix;
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
};


class ToffoliGate : public QuantumGate {
public:
    static constexpr const char* NAME = "CCNOT";
    static constexpr int QUBIT_COUNT = 3;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = []() {
            Matrix result = Matrix::identity(8);
            result(6, 6) = Complex(0, 0);
            result(6, 7) = Complex(1, 0);
            result(7, 6) = Complex(1, 0);
            result(7, 7) = Complex(0, 0);
            return result;
        }();
        return matrix;
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
};


// 旋转门
class RXGate : public QuantumGate {
public:
    static constexpr const char* NAME = "RX";  // 改为大写
    static constexpr int QUBIT_COUNT = 1;

    RXGate(double theta) : theta_(theta) {}

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        double cos_theta = std::cos(theta_ / 2);
        double sin_theta = std::sin(theta_ / 2);
        return Matrix({
            {Complex(cos_theta, 0), Complex(0, -sin_theta)},
            {Complex(0, -sin_theta), Complex(cos_theta, 0)}
        });
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }

private:
    double theta_;
};


class RYGate : public QuantumGate {
public:
    static constexpr const char* NAME = "RY";  // 改为大写
    static constexpr int QUBIT_COUNT = 1;

    RYGate(double theta) : theta_(theta) {}

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        double cos_theta = std::cos(theta_ / 2);
        double sin_theta = std::sin(theta_ / 2);
        return Matrix({
            {Complex(cos_theta, 0), Complex(-sin_theta, 0)},
            {Complex(sin_theta, 0), Complex(cos_theta, 0)}
        });
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }

private:
    double theta_;
};


class RZGate : public QuantumGate {
public:
    static constexpr const char* NAME = "RZ";  // 改为大写
    static constexpr int QUBIT_COUNT = 1;

    RZGate(double theta) : theta_(theta) {}

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        Complex phase = std::exp(Complex(0, -theta_ / 2));
        return Matrix({
            {phase, Complex(0, 0)},
            {Complex(0, 0), std::conj(phase)}
        });
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }

private:
    double theta_;
};

// 单位门
class IdGate : public QuantumGate {
public:
    static constexpr const char* NAME = "ID";
    static constexpr int QUBIT_COUNT = 1;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        return Matrix::identity(2);
    }

    Matrix apply(const Matrix& state) const override {
        return state;
    }
};

// S门的共轭转置
class SdgGate : public QuantumGate {
public:
    static constexpr const char* NAME = "Sdg";
    static constexpr int QUBIT_COUNT = 1;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = Matrix({
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, -1)}
        });
        return matrix;
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
};

// T门的共轭转置
class TdgGate : public QuantumGate {
public:
    static constexpr const char* NAME = "Tdg";
    static constexpr int QUBIT_COUNT = 1;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = []() {
            const Complex phase = std::exp(Complex(0, -M_PI/4));
            return Matrix({
                {Complex(1, 0), Complex(0, 0)},
                {Complex(0, 0), phase}
            });
        }();
        return matrix;
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
};

// SX门
class SXGate : public QuantumGate {
public:
    static constexpr const char* NAME = "SX";
    static constexpr int QUBIT_COUNT = 1;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = []() {
            const double h = 1.0 / std::sqrt(2.0);
            return Matrix({
                {Complex(h, 0), Complex(0, -h)},
                {Complex(0, -h), Complex(h, 0)}
            });
        }();
        return matrix;
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
};

// SX门的共轭转置
class SXdgGate : public QuantumGate {
public:
    static constexpr const char* NAME = "SXdg";
    static constexpr int QUBIT_COUNT = 1;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = []() {
            const double h = 1.0 / std::sqrt(2.0);
            return Matrix({
                {Complex(h, 0), Complex(0, h)},
                {Complex(0, h), Complex(h, 0)}
            });
        }();
        return matrix;
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
};

// SY门
class SYGate : public QuantumGate {
public:
    static constexpr const char* NAME = "SY";
    static constexpr int QUBIT_COUNT = 1;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = []() {
            const double h = 1.0 / std::sqrt(2.0);
            return Matrix({
                {Complex(h, 0), Complex(-h, 0)},
                {Complex(h, 0), Complex(h, 0)}
            });
        }();
        return matrix;
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
};

// SY门的共轭转置
class SYdgGate : public QuantumGate {
public:
    static constexpr const char* NAME = "SYdg";
    static constexpr int QUBIT_COUNT = 1;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = []() {
            const double h = 1.0 / std::sqrt(2.0);
            return Matrix({
                {Complex(h, 0), Complex(h, 0)},
                {Complex(-h, 0), Complex(h, 0)}
            });
        }();
        return matrix;
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
};

// W门
class WGate : public QuantumGate {
public:
    static constexpr const char* NAME = "W";
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

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
};

// SW门
class SWGate : public QuantumGate {
public:
    static constexpr const char* NAME = "SW";
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

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
};

// SW门的共轭转置
class SWdgGate : public QuantumGate {
public:
    static constexpr const char* NAME = "SWdg";
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

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
};

// U3门
class U3Gate : public QuantumGate {
public:
    static constexpr const char* NAME = "U3";
    static constexpr int QUBIT_COUNT = 1;

    U3Gate(double theta, double phi, double lambda) 
        : theta_(theta), phi_(phi), lambda_(lambda) {}

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        double cos_theta = std::cos(theta_ / 2);
        double sin_theta = std::sin(theta_ / 2);
        Complex exp_phi = std::exp(Complex(0, phi_));
        Complex exp_lambda = std::exp(Complex(0, lambda_));
        
        return Matrix({
            {Complex(cos_theta, 0), -exp_lambda * Complex(sin_theta, 0)},
            {exp_phi * Complex(sin_theta, 0), exp_phi * exp_lambda * Complex(cos_theta, 0)}
        });
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }

private:
    double theta_;
    double phi_;
    double lambda_;
};

// Phase门
class PhaseGate : public QuantumGate {
public:
    static constexpr const char* NAME = "P";
    static constexpr int QUBIT_COUNT = 1;

    PhaseGate(double lambda) : lambda_(lambda) {}

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        Complex phase = std::exp(Complex(0, lambda_));
        return Matrix({
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), phase}
        });
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }

private:
    double lambda_;
};

// RXX门
class RXXGate : public QuantumGate {
public:
    static constexpr const char* NAME = "RXX";
    static constexpr int QUBIT_COUNT = 2;

    RXXGate(double theta) : theta_(theta) {}

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        double cos_theta = std::cos(theta_ / 2);
        double sin_theta = std::sin(theta_ / 2);
        Complex i_sin = Complex(0, sin_theta);
        
        return Matrix({
            {Complex(cos_theta, 0), Complex(0, 0), Complex(0, 0), -i_sin},
            {Complex(0, 0), Complex(cos_theta, 0), -i_sin, Complex(0, 0)},
            {Complex(0, 0), -i_sin, Complex(cos_theta, 0), Complex(0, 0)},
            {-i_sin, Complex(0, 0), Complex(0, 0), Complex(cos_theta, 0)}
        });
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }

private:
    double theta_;
};

// RYY门
class RYYGate : public QuantumGate {
public:
    static constexpr const char* NAME = "RYY";
    static constexpr int QUBIT_COUNT = 2;

    RYYGate(double theta) : theta_(theta) {}

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        double cos_theta = std::cos(theta_ / 2);
        double sin_theta = std::sin(theta_ / 2);
        Complex i_sin = Complex(0, sin_theta);
        
        return Matrix({
            {Complex(cos_theta, 0), Complex(0, 0), Complex(0, 0), i_sin},
            {Complex(0, 0), Complex(cos_theta, 0), -i_sin, Complex(0, 0)},
            {Complex(0, 0), -i_sin, Complex(cos_theta, 0), Complex(0, 0)},
            {i_sin, Complex(0, 0), Complex(0, 0), Complex(cos_theta, 0)}
        });
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }

private:
    double theta_;
};

// RZZ门
class RZZGate : public QuantumGate {
public:
    static constexpr const char* NAME = "RZZ";
    static constexpr int QUBIT_COUNT = 2;

    RZZGate(double theta) : theta_(theta) {}

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        Complex phase = std::exp(Complex(0, theta_ / 2));
        Complex phase_conj = std::conj(phase);
        
        return Matrix({
            {phase_conj, Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), phase, Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), phase, Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), phase_conj}
        });
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }

private:
    double theta_;
};

// ISwap门
class ISwapGate : public QuantumGate {
public:
    static constexpr const char* NAME = "ISWAP";
    static constexpr int QUBIT_COUNT = 2;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        return Matrix({
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 1), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 1), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(1, 0)}
        });
    }

    Matrix apply(const Matrix& state) const override {
        return get_matrix() * state;
    }
};

// 添加缺少的受控门
class CYGate : public QuantumGate {
public:
    static constexpr const char* NAME = "CY";
    static constexpr int QUBIT_COUNT = 2;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = Matrix({
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(0, -1)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 1), Complex(0, 0)}
        });
        return matrix;
    }
};

class CSGate : public QuantumGate {
public:
    static constexpr const char* NAME = "CS";
    static constexpr int QUBIT_COUNT = 2;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = Matrix({
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(0, 1)}
        });
        return matrix;
    }
};

class CTGate : public QuantumGate {
public:
    static constexpr const char* NAME = "CT";
    static constexpr int QUBIT_COUNT = 2;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = []() {
            const Complex phase = std::exp(Complex(0, M_PI/4));
            return Matrix({
                {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
                {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
                {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)},
                {Complex(0, 0), Complex(0, 0), Complex(0, 0), phase}
            });
        }();
        return matrix;
    }
};

// 多控制门
class MCXGate : public QuantumGate {
public:
    static constexpr const char* NAME = "MCX";
    static constexpr int QUBIT_COUNT = 3;  // 2个控制位 + 1个目标位

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = []() {
            Matrix result = Matrix::identity(8);
            result(6, 6) = Complex(0, 0);
            result(6, 7) = Complex(1, 0);
            result(7, 6) = Complex(1, 0);
            result(7, 7) = Complex(0, 0);
            return result;
        }();
        return matrix;
    }
};

class MCYGate : public QuantumGate {
public:
    static constexpr const char* NAME = "MCY";
    static constexpr int QUBIT_COUNT = 3;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = []() {
            Matrix result = Matrix::identity(8);
            result(6, 6) = Complex(0, 0);
            result(6, 7) = Complex(0, -1);
            result(7, 6) = Complex(0, 1);
            result(7, 7) = Complex(0, 0);
            return result;
        }();
        return matrix;
    }
};

class MCZGate : public QuantumGate {
public:
    static constexpr const char* NAME = "MCZ";
    static constexpr int QUBIT_COUNT = 3;

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        static const Matrix matrix = []() {
            Matrix result = Matrix::identity(8);
            result(7, 7) = Complex(-1, 0);
            return result;
        }();
        return matrix;
    }
};

// 添加受控旋转门
class CRXGate : public QuantumGate {
public:
    static constexpr const char* NAME = "CRX";
    static constexpr int QUBIT_COUNT = 2;

    CRXGate(double theta) : theta_(theta) {}

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        double cos_theta = std::cos(theta_ / 2);
        double sin_theta = std::sin(theta_ / 2);
        return Matrix({
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(cos_theta, 0), Complex(0, -sin_theta)},
            {Complex(0, 0), Complex(0, 0), Complex(0, -sin_theta), Complex(cos_theta, 0)}
        });
    }

private:
    double theta_;
};

class CRYGate : public QuantumGate {
public:
    static constexpr const char* NAME = "CRY";
    static constexpr int QUBIT_COUNT = 2;

    CRYGate(double theta) : theta_(theta) {}

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        double cos_theta = std::cos(theta_ / 2);
        double sin_theta = std::sin(theta_ / 2);
        return Matrix({
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(cos_theta, 0), Complex(-sin_theta, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(sin_theta, 0), Complex(cos_theta, 0)}
        });
    }

private:
    double theta_;
};

class CRZGate : public QuantumGate {
public:
    static constexpr const char* NAME = "CRZ";
    static constexpr int QUBIT_COUNT = 2;

    CRZGate(double theta) : theta_(theta) {}

    std::string get_name() const override { return NAME; }
    int get_qubit_count() const override { return QUBIT_COUNT; }
    Matrix get_matrix() const override {
        Complex phase = std::exp(Complex(0, -theta_ / 2));
        return Matrix({
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), phase, Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), std::conj(phase)}
        });
    }

private:
    double theta_;
};

} // namespace Gate 