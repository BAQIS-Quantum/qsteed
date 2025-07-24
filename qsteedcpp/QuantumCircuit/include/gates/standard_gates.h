#pragma once
#include <cmath>
#include <complex>
#include "base_gate.h"
#include "matrix.h"

namespace qsteedcpp {

// 无参数门
class HGate : public Gate {
public:
    HGate() : Gate("H", 1) {}
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        const double h = 1.0 / std::sqrt(2.0);
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(h, 0), Complex(h, 0)},
            {Complex(h, 0), Complex(-h, 0)}
        });
    }
};

class XGate : public Gate {
public:
    XGate() : Gate("X", 1) {}
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(0, 0), Complex(1, 0)},
            {Complex(1, 0), Complex(0, 0)}
        });
    }
};

class YGate : public Gate {
public:
    YGate() : Gate("Y", 1) {}
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(0, 0), Complex(0, -1)},
            {Complex(0, 1), Complex(0, 0)}
        });
    }
};

class ZGate : public Gate {
public:
    ZGate() : Gate("Z", 1) {}
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(-1, 0)}
        });
    }
};


class RXGate : public Gate {
public:
    RXGate(const Parameter& theta) : Gate("RX", 1, {theta}) {}
    
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

class RYGate : public Gate {
public:
    RYGate(const Parameter& phi) : Gate("RY", 1, {phi}) {}
    
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

class RZGate : public Gate {
public:
    RZGate(const Parameter& lambda) : Gate("RZ", 1, {lambda}) {}
    
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

class CNOTGate : public Gate {
public:
    CNOTGate() : Gate("CNOT", 2) {}
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(1, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)}
        });
    }
};

class RXXGate : public Gate {
public:
    RXXGate(const Parameter& theta) : Gate("RXX", 2, {theta}) {}
    
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

class RYYGate : public Gate {
public:
    RYYGate(const Parameter& theta) : Gate("RYY", 2, {theta}) {}
    
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

class RZZGate : public Gate {
public:
    RZZGate(const Parameter& theta) : Gate("RZZ", 2, {theta}) {}
    
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
class SGate : public Gate {
public:
    SGate() : Gate("S", 1) {}
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 1)}  // e^(iπ/2) = i
        });
    }
};

// S† 门（S 门的共轭）
class SdgGate : public Gate {
public:
    SdgGate() : Gate("Sdg", 1) {}
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, -1)}  // e^(-iπ/2) = -i
        });
    }
};

// T 门（相位门 π/4）
class TGate : public Gate {
public:
    TGate() : Gate("T", 1) {}
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        double val = M_PI / 4.0;
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(std::cos(val), std::sin(val))}  // e^(iπ/4)
        });
    }
};


class TdgGate : public Gate {
public:
    TdgGate() : Gate("Tdg", 1) {}
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        double val = -M_PI / 4.0;
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(std::cos(val), std::sin(val))}  // e^(-iπ/4)
        });
    }
};

class CZGate : public Gate {
public:
    CZGate() : Gate("CZ", 2) {}
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(-1, 0)}
        });
    }
};

class SwapGate : public Gate {
public:
    SwapGate() : Gate("SWAP", 2) {}
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(1, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(1, 0)}
        });
    }
};

class iSwapGate : public Gate {
public:
    iSwapGate() : Gate("iSWAP", 2) {}
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 1), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 1), Complex(0, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(0, 0), Complex(0, 0), Complex(1, 0)}
        });
    }
};

class ToffoliGate : public Gate {
public:
    ToffoliGate() : Gate("CCX", 3) {}
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        Matrix result = Matrix::identity(8);
        result.set_element(6, 6, Complex(0, 0));
        result.set_element(6, 7, Complex(1, 0));
        result.set_element(7, 6, Complex(1, 0));
        result.set_element(7, 7, Complex(0, 0));
        return result;
    }
};

class PhaseGate : public Gate {
public:
    PhaseGate(const Parameter& lambda) : Gate("P", 1, {lambda}) {}
    
    Matrix get_matrix(const std::map<std::string, double>& param_map = {}) const override {
        double lambda_val = get_parameter(0).value(param_map);
        return Matrix(std::vector<std::vector<Complex>>{
            {Complex(1, 0), Complex(0, 0)},
            {Complex(0, 0), Complex(std::cos(lambda_val), std::sin(lambda_val))}
        });
    }
};

class U3Gate : public Gate {
public:
    U3Gate(const Parameter& theta, const Parameter& phi, const Parameter& lambda) 
        : Gate("U3", 1, {theta, phi, lambda}) {}
    
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
inline U3Gate U3(const Parameter& theta, const Parameter& phi, const Parameter& lambda) { 
    return U3Gate(theta, phi, lambda); 
}

} // namespace qsteedcpp 