#pragma once
#include "circuit_instruction.h"
#include "../gates/standard_gates.h"

namespace qsteedcpp {

// =============================================================================
// Gate Concepts for compile-time type checking
// =============================================================================

template<typename T>
concept SingleQubitNonParam = (T::qubit_count == 1 && T::param_count == 0);

template<typename T>
concept SingleQubitSingleParam = (T::qubit_count == 1 && T::param_count == 1);

template<typename T>
concept SingleQubitTripleParam = (T::qubit_count == 1 && T::param_count == 3);

template<typename T>
concept TwoQubitNonParam = (T::qubit_count == 2 && T::param_count == 0);

template<typename T>
concept TwoQubitSingleParam = (T::qubit_count == 2 && T::param_count == 1);

template<typename T>
concept ThreeQubitNonParam = (T::qubit_count == 3 && T::param_count == 0);

template<typename T>
concept MultiControlledNonParam = (T::qubit_count == -1 && T::param_count == 0);

template<typename T>
concept MultiControlledSingleParam = (T::qubit_count == -1 && T::param_count == 1);

// =============================================================================
// Unified Gate Factory Template
// =============================================================================

template<typename GateType>
class GateFactory {
public:
    // Single-qubit non-parametric gates: H(0), X(1), etc.
    CircuitInstruction operator()(int qubit) const
        requires SingleQubitNonParam<GateType>
    {
        return CircuitInstruction(std::make_unique<GateType>(), std::vector<int>{qubit});
    }

    // Single-qubit single-parameter gates: RX(theta, 0), RY(phi, 1), etc.
    CircuitInstruction operator()(const Expr& param, int qubit) const
        requires SingleQubitSingleParam<GateType>
    {
        return CircuitInstruction(std::make_unique<GateType>(param), std::vector<int>{qubit});
    }

    // U3 gate: U3(theta, phi, lambda, 0)
    CircuitInstruction operator()(const Expr& theta, const Expr& phi, const Expr& lambda, int qubit) const
        requires SingleQubitTripleParam<GateType>
    {
        return CircuitInstruction(std::make_unique<GateType>(theta, phi, lambda), std::vector<int>{qubit});
    }

    // Two-qubit non-parametric gates: CNOT(0, 1), CZ(0, 1), etc.
    CircuitInstruction operator()(int qubit1, int qubit2) const
        requires TwoQubitNonParam<GateType>
    {
        return CircuitInstruction(std::make_unique<GateType>(), std::vector<int>{qubit1, qubit2});
    }

    // Two-qubit single-parameter gates: RXX(theta, 0, 1), CP(theta, 0, 1), etc.
    CircuitInstruction operator()(const Expr& param, int qubit1, int qubit2) const
        requires TwoQubitSingleParam<GateType>
    {
        return CircuitInstruction(std::make_unique<GateType>(param), std::vector<int>{qubit1, qubit2});
    }

    // Three-qubit non-parametric gates: Toffoli(0, 1, 2), Fredkin(0, 1, 2)
    CircuitInstruction operator()(int qubit1, int qubit2, int qubit3) const
        requires ThreeQubitNonParam<GateType>
    {
        return CircuitInstruction(std::make_unique<GateType>(), std::vector<int>{qubit1, qubit2, qubit3});
    }

    // Multi-controlled non-parametric gates: MCX(2, {0,1,2}), MCY(2, {0,1,2}), etc.
    CircuitInstruction operator()(int num_controls, const std::vector<int>& qubits) const
        requires MultiControlledNonParam<GateType>
    {
        if (qubits.size() != static_cast<size_t>(num_controls + 1)) {
            throw std::invalid_argument(std::string(GateType::gate_name) + ": qubit count must equal num_controls + 1");
        }
        return CircuitInstruction(std::make_unique<GateType>(num_controls), qubits);
    }

    // Multi-controlled parametric gates: MCRX(2, theta, {0,1,2}), etc.
    CircuitInstruction operator()(int num_controls, const Expr& param, const std::vector<int>& qubits) const
        requires MultiControlledSingleParam<GateType>
    {
        if (qubits.size() != static_cast<size_t>(num_controls + 1)) {
            throw std::invalid_argument(std::string(GateType::gate_name) + ": qubit count must equal num_controls + 1");
        }
        return CircuitInstruction(std::make_unique<GateType>(num_controls, param), qubits);
    }

    // Static name attribute (not a method - name is a property, not an operation)
    static constexpr const char* name = GateType::gate_name;
};

// =============================================================================
// Gate Factory Instances
// =============================================================================

// Single-qubit non-parametric gates
inline GateFactory<HGate> H;
inline GateFactory<XGate> X;
inline GateFactory<YGate> Y;
inline GateFactory<ZGate> Z;
inline GateFactory<SGate> S;
inline GateFactory<SdgGate> Sdg;
inline GateFactory<TGate> T;
inline GateFactory<TdgGate> Tdg;
inline GateFactory<IdGate> Id;
inline GateFactory<SXGate> SX;
inline GateFactory<SXdgGate> SXdg;
inline GateFactory<SYGate> SY;
inline GateFactory<SYdgGate> SYdg;
inline GateFactory<WGate> W;
inline GateFactory<SWGate> SW;
inline GateFactory<SWdgGate> SWdg;

// Single-qubit parametric gates
inline GateFactory<RXGate> RX;
inline GateFactory<RYGate> RY;
inline GateFactory<RZGate> RZ;
inline GateFactory<PhaseGate> Phase;
inline GateFactory<U3Gate> U3;

// Two-qubit non-parametric gates
inline GateFactory<CNOTGate> CNOT;
inline GateFactory<CNOTGate> CX;  // Alias for CNOT
inline GateFactory<CZGate> CZ;
inline GateFactory<CYGate> CY;
inline GateFactory<CSGate> CS;
inline GateFactory<CTGate> CT;
inline GateFactory<SwapGate> Swap;
inline GateFactory<ISwapGate> ISwap;

// Two-qubit parametric gates
inline GateFactory<RXXGate> RXX;
inline GateFactory<RYYGate> RYY;
inline GateFactory<RZZGate> RZZ;
inline GateFactory<CPGate> CP;
inline GateFactory<CRXGate> CRX;
inline GateFactory<CRYGate> CRY;
inline GateFactory<CRZGate> CRZ;

// Three-qubit gates
inline GateFactory<ToffoliGate> Toffoli;
inline GateFactory<ToffoliGate> CCX;  // Alias for Toffoli
inline GateFactory<FredkinGate> Fredkin;
inline GateFactory<FredkinGate> CSWAP;  // Alias for Fredkin

// Multi-controlled gates
inline GateFactory<MCXGate> MCX;
inline GateFactory<MCYGate> MCY;
inline GateFactory<MCZGate> MCZ;
inline GateFactory<MCRXGate> MCRX;
inline GateFactory<MCRYGate> MCRY;
inline GateFactory<MCRZGate> MCRZ;
inline GateFactory<ControlledUGate> ControlledU;

// =============================================================================
// Non-gate operation instruction factories
// =============================================================================

class MeasureFactory {
public:
    // Single qubit measurement
    CircuitInstruction operator()(int qubit, int clbit) const {
        return CircuitInstruction(Measurement(qubit, clbit));
    }

    // Multi-qubit measurement
    CircuitInstruction operator()(const std::vector<int>& qubits, const std::vector<int>& clbits) const {
        return CircuitInstruction(Measurement(qubits, clbits));
    }

    static constexpr const char* name = "measure";
};
inline MeasureFactory Measure;

class BarrierFactory {
public:
    CircuitInstruction operator()(const std::vector<int>& qubits) const {
        return CircuitInstruction(Barrier(qubits));
    }

    static constexpr const char* name = "barrier";
};
inline BarrierFactory Barrier;

class ResetFactory {
public:
    CircuitInstruction operator()(int qubit) const {
        return CircuitInstruction(Reset(qubit));
    }

    static constexpr const char* name = "reset";
};
inline ResetFactory Reset;

class DelayFactory {
public:
    CircuitInstruction operator()(int qubit, int duration, const std::string& unit = "ns") const {
        return CircuitInstruction(Delay(qubit, duration, unit));
    }

    static constexpr const char* name = "delay";
};
inline DelayFactory Delay;

class XYResonanceFactory {
public:
    CircuitInstruction operator()(int qubit_start, int qubit_end, int duration, const std::string& unit = "ns") const {
        return CircuitInstruction(XYResonance(qubit_start, qubit_end, duration, unit));
    }

    static constexpr const char* name = "xyresonance";
};
inline XYResonanceFactory XYResonance;

} // namespace qsteedcpp
