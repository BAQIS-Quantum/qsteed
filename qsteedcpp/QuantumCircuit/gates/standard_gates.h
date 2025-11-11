#pragma once
#include <cmath>
#include "base_gate.h"

namespace qsteedcpp {

// Non-parameterized gates
class HGate : public ClonableGate<HGate> {
public:
    HGate() : ClonableGate(1) {}
    const char* name() const override { return "h"; }
};

class XGate : public ClonableGate<XGate> {
public:
    XGate() : ClonableGate(1) {}
    const char* name() const override { return "x"; }
};

class YGate : public ClonableGate<YGate> {
public:
    YGate() : ClonableGate(1) {}
    const char* name() const override { return "y"; }
};

class ZGate : public ClonableGate<ZGate> {
public:
    ZGate() : ClonableGate(1) {}
    const char* name() const override { return "z"; }
};

class SGate : public ClonableGate<SGate> {
public:
    SGate() : ClonableGate(1) {}
    const char* name() const override { return "s"; }
};

class SdgGate : public ClonableGate<SdgGate> {
public:
    SdgGate() : ClonableGate(1) {}
    const char* name() const override { return "sdg"; }
};

class TGate : public ClonableGate<TGate> {
public:
    TGate() : ClonableGate(1) {}
    const char* name() const override { return "t"; }
};

class TdgGate : public ClonableGate<TdgGate> {
public:
    TdgGate() : ClonableGate(1) {}
    const char* name() const override { return "tdg"; }
};

// Parameterized 1-qubit gates
class RXGate : public ClonableGate<RXGate> {
public:
    RXGate(const Expr& theta) : ClonableGate(1, {theta}) {}
    const char* name() const override { return "rx"; }
};

class RYGate : public ClonableGate<RYGate> {
public:
    RYGate(const Expr& phi) : ClonableGate(1, {phi}) {}
    const char* name() const override { return "ry"; }
};

class RZGate : public ClonableGate<RZGate> {
public:
    RZGate(const Expr& lambda) : ClonableGate(1, {lambda}) {}
    const char* name() const override { return "rz"; }
};

class PhaseGate : public ClonableGate<PhaseGate> {
public:
    PhaseGate(const Expr& lambda) : ClonableGate(1, {lambda}) {}
    const char* name() const override { return "p"; }
};

class U3Gate : public ClonableGate<U3Gate> {
public:
    U3Gate(const Expr& theta, const Expr& phi, const Expr& lambda)
        : ClonableGate(1, {theta, phi, lambda}) {}
    const char* name() const override { return "u3"; }
};

// 2-qubit gates
class CNOTGate : public ClonableGate<CNOTGate> {
public:
    CNOTGate() : ClonableGate(2) {}
    const char* name() const override { return "cnot"; }
};

class CZGate : public ClonableGate<CZGate> {
public:
    CZGate() : ClonableGate(2) {}
    const char* name() const override { return "cz"; }
};

class SwapGate : public ClonableGate<SwapGate> {
public:
    SwapGate() : ClonableGate(2) {}
    const char* name() const override { return "swap"; }
};

class iSwapGate : public ClonableGate<iSwapGate> {
public:
    iSwapGate() : ClonableGate(2) {}
    const char* name() const override { return "iswap"; }
};

// Parameterized 2-qubit gates
class RXXGate : public ClonableGate<RXXGate> {
public:
    RXXGate(const Expr& theta) : ClonableGate(2, {theta}) {}
    const char* name() const override { return "rxx"; }
};

class RYYGate : public ClonableGate<RYYGate> {
public:
    RYYGate(const Expr& theta) : ClonableGate(2, {theta}) {}
    const char* name() const override { return "ryy"; }
};

class RZZGate : public ClonableGate<RZZGate> {
public:
    RZZGate(const Expr& theta) : ClonableGate(2, {theta}) {}
    const char* name() const override { return "rzz"; }
};

class CPGate : public ClonableGate<CPGate> {
public:
    CPGate(const Expr& theta) : ClonableGate(2, {theta}) {}
    const char* name() const override { return "cp"; }
};

// 3-qubit gates
class ToffoliGate : public ClonableGate<ToffoliGate> {
public:
    ToffoliGate() : ClonableGate(3) {}
    const char* name() const override { return "ccx"; }
};


// Helper functions
inline HGate H() { return HGate(); }
inline XGate X() { return XGate(); }
inline YGate Y() { return YGate(); }
inline ZGate Z() { return ZGate(); }
inline RXGate RX(const Expr& theta) { return RXGate(theta); }
inline RYGate RY(const Expr& phi) { return RYGate(phi); }
inline RZGate RZ(const Expr& lambda) { return RZGate(lambda); }
inline CNOTGate CNOT() { return CNOTGate(); }
inline RXXGate RXX(const Expr& theta) { return RXXGate(theta); }
inline RYYGate RYY(const Expr& theta) { return RYYGate(theta); }
inline RZZGate RZZ(const Expr& theta) { return RZZGate(theta); }

inline SGate S() { return SGate(); }
inline SdgGate Sdg() { return SdgGate(); }
inline TGate T() { return TGate(); }
inline TdgGate Tdg() { return TdgGate(); }
inline CZGate CZ() { return CZGate(); }
inline SwapGate SWAP() { return SwapGate(); }
inline iSwapGate iSWAP() { return iSwapGate(); }
inline ToffoliGate CCX() { return ToffoliGate(); }
inline ToffoliGate Toffoli() { return ToffoliGate(); }
inline PhaseGate P(const Expr& lambda) { return PhaseGate(lambda); }
inline CPGate CP(const Expr& theta) { return CPGate(theta); }
inline U3Gate U3(const Expr& theta, const Expr& phi, const Expr& lambda) {
    return U3Gate(theta, phi, lambda);
}

// Aliases
using CCXGate = ToffoliGate;
using ISwapGate = iSwapGate;
using PGate = PhaseGate;

} // namespace qsteedcpp

 