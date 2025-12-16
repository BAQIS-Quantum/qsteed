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
};

class XGate : public ClonableGate<XGate> {
public:
    static constexpr const char* gate_name = "x";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    XGate() : ClonableGate() {}
    GateType type() const override { return GateType::X; }
    const char* name() const override { return gate_name; }
};

class YGate : public ClonableGate<YGate> {
public:
    static constexpr const char* gate_name = "y";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    YGate() : ClonableGate() {}
    GateType type() const override { return GateType::Y; }
    const char* name() const override { return gate_name; }
};

class ZGate : public ClonableGate<ZGate> {
public:
    static constexpr const char* gate_name = "z";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    ZGate() : ClonableGate() {}
    GateType type() const override { return GateType::Z; }
    const char* name() const override { return gate_name; }
};

class SGate : public ClonableGate<SGate> {
public:
    static constexpr const char* gate_name = "s";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    SGate() : ClonableGate() {}
    GateType type() const override { return GateType::S; }
    const char* name() const override { return gate_name; }
};

class SdgGate : public ClonableGate<SdgGate> {
public:
    static constexpr const char* gate_name = "sdg";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    SdgGate() : ClonableGate() {}
    GateType type() const override { return GateType::SDG; }
    const char* name() const override { return gate_name; }
};

class TGate : public ClonableGate<TGate> {
public:
    static constexpr const char* gate_name = "t";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    TGate() : ClonableGate() {}
    GateType type() const override { return GateType::T; }
    const char* name() const override { return gate_name; }
};

class TdgGate : public ClonableGate<TdgGate> {
public:
    static constexpr const char* gate_name = "tdg";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    TdgGate() : ClonableGate() {}
    GateType type() const override { return GateType::TDG; }
    const char* name() const override { return gate_name; }
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
};

class SXdgGate : public ClonableGate<SXdgGate> {
public:
    static constexpr const char* gate_name = "sxdg";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    SXdgGate() : ClonableGate() {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
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
};

class SYdgGate : public ClonableGate<SYdgGate> {
public:
    static constexpr const char* gate_name = "sydg";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    SYdgGate() : ClonableGate() {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
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
};

class SWGate : public ClonableGate<SWGate> {
public:
    static constexpr const char* gate_name = "sw";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    SWGate() : ClonableGate() {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
};

class SWdgGate : public ClonableGate<SWdgGate> {
public:
    static constexpr const char* gate_name = "swdg";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 0;

    SWdgGate() : ClonableGate() {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
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
};

class RYGate : public ClonableGate<RYGate> {
public:
    static constexpr const char* gate_name = "ry";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 1;

    RYGate(const Expr& phi) : ClonableGate({phi}) {}
    GateType type() const override { return GateType::RY; }
    const char* name() const override { return gate_name; }
};

class RZGate : public ClonableGate<RZGate> {
public:
    static constexpr const char* gate_name = "rz";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 1;

    RZGate(const Expr& lambda) : ClonableGate({lambda}) {}
    GateType type() const override { return GateType::RZ; }
    const char* name() const override { return gate_name; }
};

class PhaseGate : public ClonableGate<PhaseGate> {
public:
    static constexpr const char* gate_name = "p";
    static constexpr int qubit_count = 1;
    static constexpr int param_count = 1;

    PhaseGate(const Expr& lambda) : ClonableGate({lambda}) {}
    GateType type() const override { return GateType::P; }
    const char* name() const override { return gate_name; }
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
};

class CZGate : public ClonableGate<CZGate> {
public:
    static constexpr const char* gate_name = "cz";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 0;

    CZGate() : ClonableGate() {}
    GateType type() const override { return GateType::CZ; }
    const char* name() const override { return gate_name; }
};

class SwapGate : public ClonableGate<SwapGate> {
public:
    static constexpr const char* gate_name = "swap";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 0;

    SwapGate() : ClonableGate() {}
    GateType type() const override { return GateType::SWAP; }
    const char* name() const override { return gate_name; }
};

class ISwapGate : public ClonableGate<ISwapGate> {
public:
    static constexpr const char* gate_name = "iswap";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 0;

    ISwapGate() : ClonableGate() {}
    GateType type() const override { return GateType::ISWAP; }
    const char* name() const override { return gate_name; }
};

// Controlled Pauli gates
class CYGate : public ClonableGate<CYGate> {
public:
    static constexpr const char* gate_name = "cy";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 0;

    CYGate() : ClonableGate() {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
};

class CSGate : public ClonableGate<CSGate> {
public:
    static constexpr const char* gate_name = "cs";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 0;

    CSGate() : ClonableGate() {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
};

class CTGate : public ClonableGate<CTGate> {
public:
    static constexpr const char* gate_name = "ct";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 0;

    CTGate() : ClonableGate() {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
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
};

class RYYGate : public ClonableGate<RYYGate> {
public:
    static constexpr const char* gate_name = "ryy";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 1;

    RYYGate(const Expr& theta) : ClonableGate({theta}) {}
    GateType type() const override { return GateType::RYY; }
    const char* name() const override { return gate_name; }
};

class RZZGate : public ClonableGate<RZZGate> {
public:
    static constexpr const char* gate_name = "rzz";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 1;

    RZZGate(const Expr& theta) : ClonableGate({theta}) {}
    GateType type() const override { return GateType::RZZ; }
    const char* name() const override { return gate_name; }
};

class CPGate : public ClonableGate<CPGate> {
public:
    static constexpr const char* gate_name = "cp";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 1;

    CPGate(const Expr& theta) : ClonableGate({theta}) {}
    GateType type() const override { return GateType::CP; }
    const char* name() const override { return gate_name; }
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
};

class CRYGate : public ClonableGate<CRYGate> {
public:
    static constexpr const char* gate_name = "cry";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 1;

    CRYGate(const Expr& theta) : ClonableGate({theta}) {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
};

class CRZGate : public ClonableGate<CRZGate> {
public:
    static constexpr const char* gate_name = "crz";
    static constexpr int qubit_count = 2;
    static constexpr int param_count = 1;

    CRZGate(const Expr& theta) : ClonableGate({theta}) {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
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
};

class FredkinGate : public ClonableGate<FredkinGate> {
public:
    static constexpr const char* gate_name = "cswap";
    static constexpr int qubit_count = 3;
    static constexpr int param_count = 0;

    FredkinGate() : ClonableGate() {}
    GateType type() const override { return GateType::CUSTOM; }
    const char* name() const override { return gate_name; }
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
};


// Aliases
using CCXGate = ToffoliGate;

} // namespace qsteedcpp

 