#pragma once
#include <cmath>
#include <complex>
#include "base_gate.h"
#include "matrix.h"

namespace qsteedcpp {

// 无参数门
class HGate : public ClonableGate<HGate> {
public:
    HGate() : ClonableGate(1) {}
    
    const char* name() const override { return "h"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        const double h = 1.0 / std::sqrt(2.0);
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(h, 0), Complex(h, 0)},
            {Complex(h, 0), Complex(-h, 0)}
        });
    }
};

class XGate : public ClonableGate<XGate> {
public:
    XGate() : ClonableGate(1) {}
    
    const char* name() const override { return "x"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(0, 0), Complex(1, 0)},
            {Complex(1, 0), Complex(0, 0)}
        });
    }
};

class YGate : public ClonableGate<YGate> {
public:
    YGate() : ClonableGate(1) {}
    
    const char* name() const override { return "y"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(0, 0), Complex(0, -1)},
            {Complex(0, 1), Complex(0, 0)}
        });
    }
};

class ZGate : public ClonableGate<ZGate> {
public:
    ZGate() : ClonableGate(1) {}
    
    const char* name() const override { return "z"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(-1, 0)}
        });
    }
};


class RXGate : public ClonableGate<RXGate> {
public:
    RXGate(const Parameter& theta) : ClonableGate(1, {theta}) {}
    
    const char* name() const override { return "rx"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        double theta_val = get_parameter(0).value(param_map);
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
    RYGate(const Parameter& phi) : ClonableGate(1, {phi}) {}
    
    const char* name() const override { return "ry"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        double phi_val = get_parameter(0).value(param_map);
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
    RZGate(const Parameter& lambda) : ClonableGate(1, {lambda}) {}
    
    const char* name() const override { return "rz"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        double lambda_val = get_parameter(0).value(param_map);
        double cos_half = std::cos(lambda_val / 2.0);
        double sin_half = std::sin(lambda_val / 2.0);
        
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(cos_half, -sin_half), Complex(0, 0)},
            {Complex(0, 0), Complex(cos_half, sin_half)}
        });
    }
};

class CNOTGate : public ClonableGate<CNOTGate> {
public:
    CNOTGate() : ClonableGate(2) {}
    
    const char* name() const override { return "cnot"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        // Correct matrix for CNOT(0,1) assuming |q1q0> state vector
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(1, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)}
        });
    }
};

class RXXGate : public ClonableGate<RXXGate> {
public:
    RXXGate(const Parameter& theta) : ClonableGate(2, {theta}) {}
    
    const char* name() const override { return "rxx"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        double theta_val = get_parameter(0).value(param_map);
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
    RYYGate(const Parameter& theta) : ClonableGate(2, {theta}) {}
    
    const char* name() const override { return "ryy"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        double theta_val = get_parameter(0).value(param_map);
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
    RZZGate(const Parameter& theta) : ClonableGate(2, {theta}) {}
    
    const char* name() const override { return "rzz"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        double theta_val = get_parameter(0).value(param_map);
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

// S 门（相位门 π/2）
class SGate : public ClonableGate<SGate> {
public:
    SGate() : ClonableGate(1) {}
    
    const char* name() const override { return "s"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 1)}  // e^(iπ/2) = i
        });
    }
};

// S† 门（S 门的共轭）
class SdgGate : public ClonableGate<SdgGate> {
public:
    SdgGate() : ClonableGate(1) {}
    
    const char* name() const override { return "sdg"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, -1)}  // e^(-iπ/2) = -i
        });
    }
};

// T 门（相位门 π/4）
class TGate : public ClonableGate<TGate> {
public:
    TGate() : ClonableGate(1) {}
    
    const char* name() const override { return "t"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        double val = M_PI / 4.0;
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(std::cos(val), std::sin(val))}  // e^(iπ/4)
        });
    }
};


class TdgGate : public ClonableGate<TdgGate> {
public:
    TdgGate() : ClonableGate(1) {}
    
    const char* name() const override { return "tdg"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        double val = -M_PI / 4.0;
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(std::cos(val), std::sin(val))}  // e^(-iπ/4)
        });
    }
};

class CZGate : public ClonableGate<CZGate> {
public:
    CZGate() : ClonableGate(2) {}
    
    const char* name() const override { return "cz"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
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
    SwapGate() : ClonableGate(2) {}
    
    const char* name() const override { return "swap"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(1, 0)}
        });
    }
};

class iSwapGate : public ClonableGate<iSwapGate> {
public:
    iSwapGate() : ClonableGate(2) {}
    
    const char* name() const override { return "iswap"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 1), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 1), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(1, 0)}
        });
    }
};

class ToffoliGate : public ClonableGate<ToffoliGate> {
public:
    ToffoliGate() : ClonableGate(3) {}
    
    const char* name() const override { return "ccx"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
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

class PhaseGate : public ClonableGate<PhaseGate> {
public:
    PhaseGate(const Parameter& lambda) : ClonableGate(1, {lambda}) {}
    
    const char* name() const override { return "p"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        double lambda_val = get_parameter(0).value(param_map);
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(std::cos(lambda_val), std::sin(lambda_val))}
        });
    }
};

class CPGate : public ClonableGate<CPGate> {
public:
    CPGate(const Parameter& theta) : ClonableGate(2, {theta}) {}
    
    const char* name() const override { return "cp"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        double theta_val = get_parameter(0).value(param_map);
        Matrix m = Matrix::identity(4);
        m.set_element(3, 3, Complex(std::cos(theta_val), std::sin(theta_val)));
        return m;
    }
};


class U3Gate : public ClonableGate<U3Gate> {
public:
    U3Gate(const Parameter& theta, const Parameter& phi, const Parameter& lambda) 
        : ClonableGate(1, {theta, phi, lambda}) {}
    
    const char* name() const override { return "u3"; }
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        double theta_val = get_parameter(0).value(param_map);
        double phi_val = get_parameter(1).value(param_map);
        double lambda_val = get_parameter(2).value(param_map);
        
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



inline HGate H() { return HGate(); }
inline XGate X() { return XGate(); }
inline YGate Y() { return YGate(); }
inline ZGate Z() { return ZGate(); }
inline RXGate RX(const Parameter& theta) { return RXGate(theta); }
inline RYGate RY(const Parameter& phi) { return RYGate(phi); }
inline RZGate RZ(const Parameter& lambda) { return RZGate(lambda); }
inline CNOTGate CNOT() { return CNOTGate(); }
inline RXXGate RXX(const Parameter& theta) { return RXXGate(theta); }
inline RYYGate RYY(const Parameter& theta) { return RYYGate(theta); }
inline RZZGate RZZ(const Parameter& theta) { return RZZGate(theta); }

inline SGate S() { return SGate(); }
inline SdgGate Sdg() { return SdgGate(); }
inline TGate T() { return TGate(); }
inline TdgGate Tdg() { return TdgGate(); }
inline CZGate CZ() { return CZGate(); }
inline SwapGate SWAP() { return SwapGate(); }
inline iSwapGate iSWAP() { return iSwapGate(); }
inline ToffoliGate CCX() { return ToffoliGate(); }
inline ToffoliGate Toffoli() { return ToffoliGate(); }
inline PhaseGate P(const Parameter& lambda) { return PhaseGate(lambda); }
inline CPGate CP(const Parameter& theta) { return CPGate(theta); }
inline U3Gate U3(const Parameter& theta, const Parameter& phi, const Parameter& lambda) { 
    return U3Gate(theta, phi, lambda); 
}

// 别名
using CCXGate = ToffoliGate;
using ISwapGate = iSwapGate;
using PGate = PhaseGate;

} // namespace qsteedcpp 