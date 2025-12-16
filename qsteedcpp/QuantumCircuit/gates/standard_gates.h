#pragma once
#include <cmath>
#include "base_gate.h"

namespace qsteedcpp {

// Non-parameterized gates
class HGate : public ClonableGate<HGate> {
public:
    static constexpr const char* gate_name = "h";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    HGate() : ClonableGate() {}
    GateType type() const override { return GateType::H; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        const double h = 1.0 / std::sqrt(2.0);
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(h, 0), Complex(h, 0)},
            {Complex(h, 0), Complex(-h, 0)}
        });
    }
};

class XGate : public ClonableGate<XGate> {
public:
    static constexpr const char* gate_name = "x";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    XGate() : ClonableGate() {}
    GateType type() const override { return GateType::X; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(0, 0), Complex(1, 0)},
            {Complex(1, 0), Complex(0, 0)}
        });
    }
};

class YGate : public ClonableGate<YGate> {
public:
    static constexpr const char* gate_name = "y";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    YGate() : ClonableGate() {}
    GateType type() const override { return GateType::Y; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(0, 0), Complex(0, -1)},
            {Complex(0, 1), Complex(0, 0)}
        });
    }
};

class ZGate : public ClonableGate<ZGate> {
public:
    static constexpr const char* gate_name = "z";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    ZGate() : ClonableGate() {}
    GateType type() const override { return GateType::Z; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(-1, 0)}
        });
    }
};

class SGate : public ClonableGate<SGate> {
public:
    static constexpr const char* gate_name = "s";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    SGate() : ClonableGate() {}
    GateType type() const override { return GateType::S; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 1)}  // e^(iπ/2) = i
        });
    }
};

class SdgGate : public ClonableGate<SdgGate> {
public:
    static constexpr const char* gate_name = "sdg";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    SdgGate() : ClonableGate() {}
    GateType type() const override { return GateType::SDG; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, -1)}  // e^(-iπ/2) = -i
        });
    }
};

class TGate : public ClonableGate<TGate> {
public:
    static constexpr const char* gate_name = "t";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    TGate() : ClonableGate() {}
    GateType type() const override { return GateType::T; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        double val = M_PI / 4.0;
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(std::cos(val), std::sin(val))}  // e^(iπ/4)
        });
    }
};

class TdgGate : public ClonableGate<TdgGate> {
public:
    static constexpr const char* gate_name = "tdg";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    TdgGate() : ClonableGate() {}
    GateType type() const override { return GateType::TDG; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        double val = -M_PI / 4.0;
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(std::cos(val), std::sin(val))}  // e^(-iπ/4)
        });
    }
};

// Identity gate
class IdGate : public ClonableGate<IdGate> {
public:
    static constexpr const char* gate_name = "id";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    IdGate() : ClonableGate() {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        return Matrix::identity(2);
    }
};

// SX and SXdg gates (sqrt(X))
class SXGate : public ClonableGate<SXGate> {
public:
    static constexpr const char* gate_name = "sx";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    SXGate() : ClonableGate() {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        // SX = sqrt(X) = [[1+i, 1-i], [1-i, 1+i]] / 2
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(0.5, 0.5), Complex(0.5, -0.5)},
            {Complex(0.5, -0.5), Complex(0.5, 0.5)}
        });
    }
};

class SXdgGate : public ClonableGate<SXdgGate> {
public:
    static constexpr const char* gate_name = "sxdg";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    SXdgGate() : ClonableGate() {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        // SXdg = sqrt(X)^† = [[1-i, 1+i], [1+i, 1-i]] / 2
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(0.5, -0.5), Complex(0.5, 0.5)},
            {Complex(0.5, 0.5), Complex(0.5, -0.5)}
        });
    }
};

// SY and SYdg gates (sqrt(Y))
class SYGate : public ClonableGate<SYGate> {
public:
    static constexpr const char* gate_name = "sy";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    SYGate() : ClonableGate() {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        // SY = sqrt(Y) = [[1+i, -1-i], [1+i, 1+i]] / 2
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(0.5, 0.5), Complex(-0.5, -0.5)},
            {Complex(0.5, 0.5), Complex(0.5, 0.5)}
        });
    }
};

class SYdgGate : public ClonableGate<SYdgGate> {
public:
    static constexpr const char* gate_name = "sydg";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    SYdgGate() : ClonableGate() {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        // SYdg = sqrt(Y)^† = [[1-i, 1-i], [-1+i, 1-i]] / 2
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(0.5, -0.5), Complex(0.5, -0.5)},
            {Complex(-0.5, 0.5), Complex(0.5, -0.5)}
        });
    }
};

// W gate family
class WGate : public ClonableGate<WGate> {
public:
    static constexpr const char* gate_name = "w";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    WGate() : ClonableGate() {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        // W gate (custom definition may vary)
        const double inv_sqrt2 = 1.0 / std::sqrt(2.0);
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(inv_sqrt2, 0), Complex(0, -inv_sqrt2)},
            {Complex(0, inv_sqrt2), Complex(inv_sqrt2, 0)}
        });
    }
};

class SWGate : public ClonableGate<SWGate> {
public:
    static constexpr const char* gate_name = "sw";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    SWGate() : ClonableGate() {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        // SW = sqrt(W) where W = [[0, -i], [i, 0]]
        // SW = (1 + W) / sqrt(2(1 + sqrt(2)))
        const double c = (1.0 + std::sqrt(2.0)) / 2.0;  // (1 + sqrt(2)) / 2
        const double s = (std::sqrt(2.0) - 1.0) / 2.0;   // (sqrt(2) - 1) / 2
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(c, -s), Complex(-s, -s)},
            {Complex(s, s), Complex(c, s)}
        });
    }
};

class SWdgGate : public ClonableGate<SWdgGate> {
public:
    static constexpr const char* gate_name = "swdg";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    SWdgGate() : ClonableGate() {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        // SWdg = sqrt(W)^† (conjugate transpose of SW)
        const double c = (1.0 + std::sqrt(2.0)) / 2.0;  // (1 + sqrt(2)) / 2
        const double s = (std::sqrt(2.0) - 1.0) / 2.0;   // (sqrt(2) - 1) / 2
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(c, s), Complex(s, -s)},
            {Complex(-s, -s), Complex(c, -s)}
        });
    }
};

// Parameterized 1-qubit gates
class RXGate : public ClonableGate<RXGate> {
public:
    static constexpr const char* gate_name = "rx";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 1;

    RXGate(const Expr& theta) : ClonableGate({theta}) {}
    GateType type() const override { return GateType::RX; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        double theta_val = get_parameter_expression(0).eval();
        double cos_half = std::cos(theta_val / 2.0);
        double sin_half = std::sin(theta_val / 2.0);
        
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(cos_half, 0), Complex(0, -sin_half)},
            {Complex(0, -sin_half), Complex(cos_half, 0)}
        });
    }
};

class RYGate : public ClonableGate<RYGate> {
public:
    static constexpr const char* gate_name = "ry";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 1;

    RYGate(const Expr& phi) : ClonableGate({phi}) {}
    GateType type() const override { return GateType::RY; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        double phi_val = get_parameter_expression(0).eval();
        double cos_half = std::cos(phi_val / 2.0);
        double sin_half = std::sin(phi_val / 2.0);
        
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(cos_half, 0), Complex(-sin_half, 0)},
            {Complex(sin_half, 0), Complex(cos_half, 0)}
        });
    }
};

class RZGate : public ClonableGate<RZGate> {
public:
    static constexpr const char* gate_name = "rz";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 1;

    RZGate(const Expr& lambda) : ClonableGate({lambda}) {}
    GateType type() const override { return GateType::RZ; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        double lambda_val = get_parameter_expression(0).eval();
        double cos_half = std::cos(lambda_val / 2.0);
        double sin_half = std::sin(lambda_val / 2.0);
        
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(cos_half, -sin_half), Complex(0, 0)},
            {Complex(0, 0), Complex(cos_half, sin_half)}
        });
    }
};

class PhaseGate : public ClonableGate<PhaseGate> {
public:
    static constexpr const char* gate_name = "p";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 1;

    PhaseGate(const Expr& lambda) : ClonableGate({lambda}) {}
    GateType type() const override { return GateType::P; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        double lambda_val = get_parameter_expression(0).eval();
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(std::cos(lambda_val), std::sin(lambda_val))}
        });
    }
};

class U3Gate : public ClonableGate<U3Gate> {
public:
    static constexpr const char* gate_name = "u3";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 3;

    U3Gate(const Expr& theta, const Expr& phi, const Expr& lambda)
        : ClonableGate({theta, phi, lambda}) {}
    GateType type() const override { return GateType::U3; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        double theta_val = get_parameter_expression(0).eval();
        double phi_val = get_parameter_expression(1).eval();
        double lambda_val = get_parameter_expression(2).eval();
        
        double cos_half = std::cos(theta_val / 2.0);
        double sin_half = std::sin(theta_val / 2.0);
        
        Complex exp_phi(std::cos(phi_val), std::sin(phi_val));
        Complex exp_lambda(std::cos(lambda_val), std::sin(lambda_val));
        Complex exp_sum(std::cos(phi_val + lambda_val), std::sin(phi_val + lambda_val));
        
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(cos_half, 0), -exp_lambda * Complex(sin_half, 0)},
            {exp_phi * Complex(sin_half, 0), exp_sum * Complex(cos_half, 0)}
        });
    }
};

// 2-qubit gates
class CNOTGate : public ClonableGate<CNOTGate> {
public:
    static constexpr const char* gate_name = "cx";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 0;

    CNOTGate() : ClonableGate() {}
    GateType type() const override { return GateType::CNOT; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(1, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)}
        });
    }
};

class CZGate : public ClonableGate<CZGate> {
public:
    static constexpr const char* gate_name = "cz";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 0;

    CZGate() : ClonableGate() {}
    GateType type() const override { return GateType::CZ; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(-1, 0)}
        });
    }
};

class SwapGate : public ClonableGate<SwapGate> {
public:
    static constexpr const char* gate_name = "swap";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 0;

    SwapGate() : ClonableGate() {}
    GateType type() const override { return GateType::SWAP; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(1, 0)}
        });
    }
};

class ISwapGate : public ClonableGate<ISwapGate> {
public:
    static constexpr const char* gate_name = "iswap";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 0;

    ISwapGate() : ClonableGate() {}
    GateType type() const override { return GateType::ISWAP; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 1), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 1), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(1, 0)}
        });
    }
};

// Controlled Pauli gates
class CYGate : public ClonableGate<CYGate> {
public:
    static constexpr const char* gate_name = "cy";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 0;

    CYGate() : ClonableGate() {}
    GateType type() const override { return GateType::CY; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(0, -1)},
            {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 1), Complex(0, 0), Complex(0, 0)}
        });
    }
};

class CSGate : public ClonableGate<CSGate> {
public:
    static constexpr const char* gate_name = "cs";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 0;

    CSGate() : ClonableGate() {}
    GateType type() const override { return GateType::CS; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(0, 1)}
        });
    }
};

class CTGate : public ClonableGate<CTGate> {
public:
    static constexpr const char* gate_name = "ct";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 0;

    CTGate() : ClonableGate() {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        double val = M_PI / 4.0;
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(std::cos(val), std::sin(val))}
        });
    }
};

// Parameterized 2-qubit gates
class RXXGate : public ClonableGate<RXXGate> {
public:
    static constexpr const char* gate_name = "rxx";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 1;

    RXXGate(const Expr& theta) : ClonableGate({theta}) {}
    GateType type() const override { return GateType::RXX; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        double theta_val = get_parameter_expression(0).eval();
        double cos_val = std::cos(theta_val / 2.0);
        double sin_val = std::sin(theta_val / 2.0);
        
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(cos_val, 0), Complex(0, 0), Complex(0, 0), Complex(0, -sin_val)},
            {Complex(0, 0), Complex(cos_val, 0), Complex(0, -sin_val), Complex(0, 0)},
            {Complex(0, 0), Complex(0, -sin_val), Complex(cos_val, 0), Complex(0, 0)},
            {Complex(0, -sin_val), Complex(0, 0), Complex(0, 0), Complex(cos_val, 0)}
        });
    }
};

class RYYGate : public ClonableGate<RYYGate> {
public:
    static constexpr const char* gate_name = "ryy";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 1;

    RYYGate(const Expr& theta) : ClonableGate({theta}) {}
    GateType type() const override { return GateType::RYY; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        double theta_val = get_parameter_expression(0).eval();
        double cos_val = std::cos(theta_val / 2.0);
        double sin_val = std::sin(theta_val / 2.0);
        
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(cos_val, 0), Complex(0, 0), Complex(0, 0), Complex(0, sin_val)},
            {Complex(0, 0), Complex(cos_val, 0), Complex(0, -sin_val), Complex(0, 0)},
            {Complex(0, 0), Complex(0, -sin_val), Complex(cos_val, 0), Complex(0, 0)},
            {Complex(0, sin_val), Complex(0, 0), Complex(0, 0), Complex(cos_val, 0)}
        });
    }
};

class RZZGate : public ClonableGate<RZZGate> {
public:
    static constexpr const char* gate_name = "rzz";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 1;

    RZZGate(const Expr& theta) : ClonableGate({theta}) {}
    GateType type() const override { return GateType::RZZ; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        double theta_val = get_parameter_expression(0).eval();
        double cos_val = std::cos(theta_val / 2.0);
        double sin_val = std::sin(theta_val / 2.0);
        
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(cos_val, -sin_val), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(cos_val, sin_val), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(cos_val, sin_val), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(cos_val, -sin_val)}
        });
    }
};

class CPGate : public ClonableGate<CPGate> {
public:
    static constexpr const char* gate_name = "cp";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 1;

    CPGate(const Expr& theta) : ClonableGate({theta}) {}
    GateType type() const override { return GateType::CP; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        double theta_val = get_parameter_expression(0).eval();
        Matrix m = Matrix::identity(4);
        m.set_element(3, 3, Complex(std::cos(theta_val), std::sin(theta_val)));
        return m;
    }
};

// Controlled rotation gates
class CRXGate : public ClonableGate<CRXGate> {
public:
    static constexpr const char* gate_name = "crx";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 1;

    CRXGate(const Expr& theta) : ClonableGate({theta}) {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        double theta_val = get_parameter_expression(0).eval();
        double cos_half = std::cos(theta_val / 2.0);
        double sin_half = std::sin(theta_val / 2.0);
        
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(cos_half, 0), Complex(0, -sin_half)},
            {Complex(0, 0), Complex(0, 0), Complex(0, -sin_half), Complex(cos_half, 0)}
        });
    }
};

class CRYGate : public ClonableGate<CRYGate> {
public:
    static constexpr const char* gate_name = "cry";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 1;

    CRYGate(const Expr& theta) : ClonableGate({theta}) {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        double theta_val = get_parameter_expression(0).eval();
        double cos_half = std::cos(theta_val / 2.0);
        double sin_half = std::sin(theta_val / 2.0);
        
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(cos_half, 0), Complex(-sin_half, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(sin_half, 0), Complex(cos_half, 0)}
        });
    }
};

class CRZGate : public ClonableGate<CRZGate> {
public:
    static constexpr const char* gate_name = "crz";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 1;

    CRZGate(const Expr& theta) : ClonableGate({theta}) {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        double theta_val = get_parameter_expression(0).eval();
        double cos_half = std::cos(theta_val / 2.0);
        double sin_half = std::sin(theta_val / 2.0);
        
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(cos_half, -sin_half), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(cos_half, sin_half)}
        });
    }
};

// 3-qubit gates
class ToffoliGate : public ClonableGate<ToffoliGate> {
public:
    static constexpr const char* gate_name = "ccx";
    static constexpr int qubit_count = 3;
    static constexpr int param_count = 0;

    ToffoliGate() : ClonableGate() {}
    GateType type() const override { return GateType::TOFFOLI; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        Matrix result = Matrix::identity(8);
        // Flips target qubit (q2) if control qubits (q0, q1) are 1.
        // This swaps the basis states |011> (3) and |111> (7).
        result.set_element(3, 3, Complex(0, 0));
        result.set_element(3, 7, Complex(1, 0));
        result.set_element(7, 3, Complex(1, 0));
        result.set_element(7, 7, Complex(0, 0));
        return result;
    }
};

class FredkinGate : public ClonableGate<FredkinGate> {
public:
    static constexpr const char* gate_name = "cswap";
    static constexpr int qubit_count = 3;
    static constexpr int param_count = 0;

    FredkinGate() : ClonableGate() {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    
    Matrix get_matrix() const override {
        // Fredkin (CSWAP) gate: controlled swap
        Matrix result = Matrix::identity(8);
        // Swaps |101> (5) and |110> (6) when control is 1
        result.set_element(5, 5, Complex(0, 0));
        result.set_element(5, 6, Complex(1, 0));
        result.set_element(6, 5, Complex(1, 0));
        result.set_element(6, 6, Complex(0, 0));
        return result;
    }
};

// Multi-controlled gates (variable qubit count)
class MCXGate : public ClonableGate<MCXGate> {
private:
    int num_controls_;
public:
    static constexpr const char* gate_name = "mcx";
    static constexpr int qubit_count = -1;  // Variable qubit count
    static constexpr int param_count = 0;

    MCXGate(int num_controls)
        : ClonableGate(), num_controls_(num_controls) {}

    // Override to return runtime qubit count
    int get_qubit_count() const override { return num_controls_ + 1; }

    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    int num_controls() const { return num_controls_; }
    
    Matrix get_matrix() const override {
        int n = get_qubit_count();
        int dim = 1 << n;  // 2^n
        Matrix result = Matrix::identity(dim);
        // Multi-controlled X: flip last two entries
        result.set_element(dim - 2, dim - 2, Complex(0, 0));
        result.set_element(dim - 2, dim - 1, Complex(1, 0));
        result.set_element(dim - 1, dim - 2, Complex(1, 0));
        result.set_element(dim - 1, dim - 1, Complex(0, 0));
        return result;
    }
};

class MCYGate : public ClonableGate<MCYGate> {
private:
    int num_controls_;
public:
    static constexpr const char* gate_name = "mcy";
    static constexpr int qubit_count = -1;  // Variable qubit count
    static constexpr int param_count = 0;

    MCYGate(int num_controls)
        : ClonableGate(), num_controls_(num_controls) {}

    // Override to return runtime qubit count
    int get_qubit_count() const override { return num_controls_ + 1; }

    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    int num_controls() const { return num_controls_; }
    
    Matrix get_matrix() const override {
        int n = get_qubit_count();
        int dim = 1 << n;  // 2^n
        Matrix result = Matrix::identity(dim);
        // Multi-controlled Y: apply Y matrix to last two entries
        result.set_element(dim - 2, dim - 2, Complex(0, 0));
        result.set_element(dim - 2, dim - 1, Complex(0, -1));
        result.set_element(dim - 1, dim - 2, Complex(0, 1));
        result.set_element(dim - 1, dim - 1, Complex(0, 0));
        return result;
    }
};

class MCZGate : public ClonableGate<MCZGate> {
private:
    int num_controls_;
public:
    static constexpr const char* gate_name = "mcz";
    static constexpr int qubit_count = -1;  // Variable qubit count
    static constexpr int param_count = 0;

    MCZGate(int num_controls)
        : ClonableGate(), num_controls_(num_controls) {}

    // Override to return runtime qubit count
    int get_qubit_count() const override { return num_controls_ + 1; }

    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    int num_controls() const { return num_controls_; }
    
    Matrix get_matrix() const override {
        int n = get_qubit_count();
        int dim = 1 << n;  // 2^n
        Matrix result = Matrix::identity(dim);
        // Multi-controlled Z: flip sign of last entry
        result.set_element(dim - 1, dim - 1, Complex(-1, 0));
        return result;
    }
};

// Multi-controlled rotation gates
class MCRXGate : public ClonableGate<MCRXGate> {
private:
    int num_controls_;
public:
    static constexpr const char* gate_name = "mcrx";
    static constexpr int qubit_count = -1;  // Variable qubit count
    static constexpr int param_count = 1;

    MCRXGate(int num_controls, const Expr& theta)
        : ClonableGate({theta}), num_controls_(num_controls) {}

    // Override to return runtime qubit count
    int get_qubit_count() const override { return num_controls_ + 1; }

    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    int num_controls() const { return num_controls_; }
    
    Matrix get_matrix() const override {
        double theta_val = get_parameter_expression(0).eval();
        double cos_half = std::cos(theta_val / 2.0);
        double sin_half = std::sin(theta_val / 2.0);
        
        int n = get_qubit_count();
        int dim = 1 << n;  // 2^n
        Matrix result = Matrix::identity(dim);
        // Multi-controlled RX: apply RX to last 2x2 block
        result.set_element(dim - 2, dim - 2, Complex(cos_half, 0));
        result.set_element(dim - 2, dim - 1, Complex(0, -sin_half));
        result.set_element(dim - 1, dim - 2, Complex(0, -sin_half));
        result.set_element(dim - 1, dim - 1, Complex(cos_half, 0));
        return result;
    }
};

class MCRYGate : public ClonableGate<MCRYGate> {
private:
    int num_controls_;
public:
    static constexpr const char* gate_name = "mcry";
    static constexpr int qubit_count = -1;  // Variable qubit count
    static constexpr int param_count = 1;

    MCRYGate(int num_controls, const Expr& theta)
        : ClonableGate({theta}), num_controls_(num_controls) {}

    // Override to return runtime qubit count
    int get_qubit_count() const override { return num_controls_ + 1; }

    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    int num_controls() const { return num_controls_; }
    
    Matrix get_matrix() const override {
        double theta_val = get_parameter_expression(0).eval();
        double cos_half = std::cos(theta_val / 2.0);
        double sin_half = std::sin(theta_val / 2.0);
        
        int n = get_qubit_count();
        int dim = 1 << n;  // 2^n
        Matrix result = Matrix::identity(dim);
        // Multi-controlled RY: apply RY to last 2x2 block
        result.set_element(dim - 2, dim - 2, Complex(cos_half, 0));
        result.set_element(dim - 2, dim - 1, Complex(-sin_half, 0));
        result.set_element(dim - 1, dim - 2, Complex(sin_half, 0));
        result.set_element(dim - 1, dim - 1, Complex(cos_half, 0));
        return result;
    }
};

class MCRZGate : public ClonableGate<MCRZGate> {
private:
    int num_controls_;
public:
    static constexpr const char* gate_name = "mcrz";
    static constexpr int qubit_count = -1;  // Variable qubit count
    static constexpr int param_count = 1;

    MCRZGate(int num_controls, const Expr& theta)
        : ClonableGate({theta}), num_controls_(num_controls) {}

    // Override to return runtime qubit count
    int get_qubit_count() const override { return num_controls_ + 1; }

    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    int num_controls() const { return num_controls_; }
    
    Matrix get_matrix() const override {
        double theta_val = get_parameter_expression(0).eval();
        double cos_half = std::cos(theta_val / 2.0);
        double sin_half = std::sin(theta_val / 2.0);
        
        int n = get_qubit_count();
        int dim = 1 << n;  // 2^n
        Matrix result = Matrix::identity(dim);
        // Multi-controlled RZ: apply RZ to last 2x2 block
        result.set_element(dim - 2, dim - 2, Complex(cos_half, -sin_half));
        result.set_element(dim - 1, dim - 1, Complex(cos_half, sin_half));
        return result;
    }
};

// Controlled-U gate (generic controlled unitary)
// Note: In Python usage, the matrix will be passed as attribute, not stored in C++
class ControlledUGate : public ClonableGate<ControlledUGate> {
private:
    int num_controls_;
public:
    static constexpr const char* gate_name = "controlledu";
    static constexpr int qubit_count = -1;  // Variable qubit count
    static constexpr int param_count = 0;

    ControlledUGate(int num_controls)
        : ClonableGate(), num_controls_(num_controls) {}

    // Override to return runtime qubit count
    int get_qubit_count() const override { return num_controls_ + 1; }

    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
    int num_controls() const { return num_controls_; }
    
    Matrix get_matrix() const override {
        // Controlled-U gate matrix should be provided externally
        // This is a placeholder that returns identity
        int n = get_qubit_count();
        int dim = 1 << n;  // 2^n
        return Matrix::identity(dim);
    }
};


// Aliases
using CCXGate = ToffoliGate;

} // namespace qsteedcpp

 