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

// 有参数门
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

// 两量子比特门
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

// 便捷的工厂函数
inline HGate H() { return HGate(); }
inline XGate X() { return XGate(); }
inline YGate Y() { return YGate(); }
inline ZGate Z() { return ZGate(); }
inline RXGate RX(const Parameter& theta) { return RXGate(theta); }
inline RYGate RY(const Parameter& phi) { return RYGate(phi); }
inline RZGate RZ(const Parameter& lambda) { return RZGate(lambda); }
inline CNOTGate CNOT() { return CNOTGate(); }

} // namespace qsteedcpp 