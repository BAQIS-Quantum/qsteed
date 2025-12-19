
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include <pybind11/operators.h>
#include <pybind11/stl_bind.h>
#include <pybind11/eigen.h>
#include <sstream>
#include <variant>
#include <type_traits>

#include "../QuantumCircuit/circuit/quantum_circuit.h"
#include "../QuantumCircuit/circuit/circuit_instruction.h"
#include "../QuantumCircuit/circuit/instruction_factory.h"
#include "../QuantumCircuit/utils/circuit_dag_convert.h"

namespace py = pybind11;
using namespace qsteedcpp;

// Helper to convert a vector to a string like "[0, 1, 2]"
template<typename T>
std::string vec_to_string(const std::vector<T>& vec) {
    std::stringstream ss;
    ss << "[";
    for (size_t i = 0; i < vec.size(); ++i) {
        ss << vec[i];
        if (i < vec.size() - 1) {
            ss << ", ";
        }
    }
    ss << "]";
    return ss.str();
}


// Helper function to create measures dictionary from circuit instructions
py::dict get_measures_dict(const QuantumCircuit& qc) {
    py::dict measures_dict;
    const auto& instructions = qc.get_instructions();

    for (const auto& inst : instructions) {
        if (std::holds_alternative<Measurement>(inst.operation)) {
            const auto& meas = std::get<Measurement>(inst.operation);
            // Use py::int_() wrapper to convert int keys properly for Python dict
            py::int_ qubit_key(inst.qubits[0]);
            py::int_ clbit_value(inst.clbits[0]);
            measures_dict[qubit_key] = clbit_value;
        }
    }

    return measures_dict;
}

// =============================================================================
// Gate Factory Binding Helper Templates - Modern C++ (Type-safe)
// =============================================================================

// Single-qubit non-parametric gates: H(0), X(1), etc.
template<typename GateClass>
void bind_gate_1q_nonparam(py::module& m, const char* name, const GateFactory<GateClass>& instance) {
    using FactoryType = GateFactory<GateClass>;
    std::string py_class_name = std::string(name) + "Factory";

    py::class_<FactoryType>(m, py_class_name.c_str())
        .def("__call__", [](const FactoryType& self, int qubit) {
            return self(qubit);
        }, py::arg("qubit"))
        .def_readonly_static("name", &FactoryType::name);  // Read-only static attribute

    m.attr(name) = instance;
}

// Single-qubit parametric gates: RX(theta, 0), RY(phi, 1), etc.
template<typename GateClass>
void bind_gate_1q_param(py::module& m, const char* name, const GateFactory<GateClass>& instance) {
    using FactoryType = GateFactory<GateClass>;
    std::string py_class_name = std::string(name) + "Factory";

    py::class_<FactoryType>(m, py_class_name.c_str())
        .def("__call__", [](const FactoryType& self, const Expr& param, int qubit) {
            return self(param, qubit);
        }, py::arg("param"), py::arg("qubit"))
        .def_readonly_static("name", &FactoryType::name);  // Read-only static attribute

    m.attr(name) = instance;
}

// Two-qubit non-parametric gates: CNOT(0, 1), CZ(0, 1), etc.
template<typename GateClass>
void bind_gate_2q_nonparam(py::module& m, const char* name, const GateFactory<GateClass>& instance) {
    using FactoryType = GateFactory<GateClass>;
    std::string py_class_name = std::string(name) + "Factory";

    py::class_<FactoryType>(m, py_class_name.c_str())
        .def("__call__", [](const FactoryType& self, int qubit1, int qubit2) {
            return self(qubit1, qubit2);
        }, py::arg("qubit1"), py::arg("qubit2"))
        .def_readonly_static("name", &FactoryType::name);  // Read-only static attribute

    m.attr(name) = instance;
}

// Two-qubit parametric gates: RXX(theta, 0, 1), CP(theta, 0, 1), etc.
template<typename GateClass>
void bind_gate_2q_param(py::module& m, const char* name, const GateFactory<GateClass>& instance) {
    using FactoryType = GateFactory<GateClass>;
    std::string py_class_name = std::string(name) + "Factory";

    py::class_<FactoryType>(m, py_class_name.c_str())
        .def("__call__", [](const FactoryType& self, const Expr& param, int qubit1, int qubit2) {
            return self(param, qubit1, qubit2);
        }, py::arg("param"), py::arg("qubit1"), py::arg("qubit2"))
        .def_readonly_static("name", &FactoryType::name);  // Read-only static attribute

    m.attr(name) = instance;
}

// Three-qubit non-parametric gates: Toffoli(0, 1, 2), Fredkin(0, 1, 2)
template<typename GateClass>
void bind_gate_3q_nonparam(py::module& m, const char* name, const GateFactory<GateClass>& instance) {
    using FactoryType = GateFactory<GateClass>;
    std::string py_class_name = std::string(name) + "Factory";

    py::class_<FactoryType>(m, py_class_name.c_str())
        .def("__call__", [](const FactoryType& self, int qubit1, int qubit2, int qubit3) {
            return self(qubit1, qubit2, qubit3);
        }, py::arg("qubit1"), py::arg("qubit2"), py::arg("qubit3"))
        .def_readonly_static("name", &FactoryType::name);  // Read-only static attribute

    m.attr(name) = instance;
}

// Multi-controlled non-parametric gates: MCX(2, {0,1,2}), MCY(2, {0,1,2}), etc.
template<typename GateClass>
void bind_gate_mc_nonparam(py::module& m, const char* name, const GateFactory<GateClass>& instance) {
    using FactoryType = GateFactory<GateClass>;
    std::string py_class_name = std::string(name) + "Factory";

    py::class_<FactoryType>(m, py_class_name.c_str())
        .def("__call__", [](const FactoryType& self, int num_controls, const std::vector<int>& qubits) {
            return self(num_controls, qubits);
        }, py::arg("num_controls"), py::arg("qubits"))
        .def_readonly_static("name", &FactoryType::name);  // Read-only static attribute

    m.attr(name) = instance;
}

// Multi-controlled parametric gates: MCRX(2, theta, {0,1,2}), etc.
template<typename GateClass>
void bind_gate_mc_param(py::module& m, const char* name, const GateFactory<GateClass>& instance) {
    using FactoryType = GateFactory<GateClass>;
    std::string py_class_name = std::string(name) + "Factory";

    py::class_<FactoryType>(m, py_class_name.c_str())
        .def("__call__", [](const FactoryType& self, int num_controls, const Expr& param, const std::vector<int>& qubits) {
            return self(num_controls, param, qubits);
        }, py::arg("num_controls"), py::arg("param"), py::arg("qubits"))
        .def_readonly_static("name", &FactoryType::name);  // Read-only static attribute

    m.attr(name) = instance;
}

void bind_quantum_circuit(py::module& m) {

    py::class_<CircuitInstruction>(m, "CircuitInstruction")
        .def_readonly("qubits", &CircuitInstruction::qubits)
        .def_readonly("clbits", &CircuitInstruction::clbits)

        // Backward compatibility: 'pos' as alias for 'qubits'
        // For measure: returns dict {qubit: clbit}
        // For other gates: returns list [qubits]
        .def_property_readonly("pos", [](const CircuitInstruction& inst) -> py::object {
            if (inst.is_measurement()) {
                // Measure: return {qubit: clbit} dict for DAG compatibility
                py::dict result;
                for (size_t i = 0; i < inst.qubits.size(); ++i) {
                    result[py::int_(inst.qubits[i])] = py::int_(inst.clbits[i]);
                }
                return result;
            } else {
                // Other gates: return list of qubits
                return py::cast(inst.qubits);
            }
        })

        .def_property_readonly("name", &CircuitInstruction::name)

        // Parameters (extract from Gate if it's a parameterized gate)
        .def_property_readonly("paras", [](const CircuitInstruction& inst) -> py::object {
            if (!inst.is_gate()) {
                return py::none();
            }
            const auto& gate_ptr = std::get<std::unique_ptr<Gate>>(inst.operation);
            if (!gate_ptr->has_parameters()) {
                return py::none();
            }
            // Return parameters as a list
            return py::cast(gate_ptr->get_parameter_expressions());
        })

        // Duration (only for Delay and XYResonance operations)
        .def_property_readonly("duration", [](const CircuitInstruction& inst) -> py::object {
            if (auto duration = inst.get_duration()) {
                return py::cast(*duration);
            }
            return py::none();
        })

        // Unit (only for Delay and XYResonance operations)
        .def_property_readonly("unit", [](const CircuitInstruction& inst) -> py::object {
            if (auto unit = inst.get_unit()) {
                return py::cast(*unit);
            }
            return py::none();
        })

        // Type checking methods
        .def("is_gate", &CircuitInstruction::is_gate)
        .def("is_measurement", &CircuitInstruction::is_measurement)
        .def("is_barrier", &CircuitInstruction::is_barrier)

        .def("get_matrix", [](const CircuitInstruction& self) -> Eigen::MatrixXcd {
            return self.get_matrix().eigen_matrix();
        }, "Get the unitary matrix representation of this instruction")
        .def_property_readonly("matrix", [](const CircuitInstruction& self) -> Eigen::MatrixXcd {
            return self.get_matrix().eigen_matrix();
        }, "The unitary matrix representation of this instruction")


        .def("clone", &CircuitInstruction::clone, "Create a deep copy of this instruction")

        .def("__copy__", [](const CircuitInstruction& self) {
            return self.clone();
        })
        .def("__deepcopy__", [](const CircuitInstruction& self, py::dict) {
            return self.clone();
        }, py::arg("memo"))

        .def("__repr__", [](const CircuitInstruction& inst) {
            std::string repr = inst.name();
            repr += ":q" + vec_to_string(inst.qubits);
            if (!inst.clbits.empty()) {
                repr += "->c" + vec_to_string(inst.clbits);
            }
            return repr;
        });

    // Bind QuantumCircuit
    py::class_<QuantumCircuit>(m, "QuantumCircuit")
        .def(py::init<int, int>(), py::arg("num_qubits"), py::arg("num_clbits") = -1)
        .def_static("from_openqasm", &QuantumCircuit::from_openqasm, py::arg("qasm_str"),
            "Create a QuantumCircuit from an OpenQASM string")
        .def("num_qubits", &QuantumCircuit::num_qubits)
        .def("num_clbits", &QuantumCircuit::num_clbits)
        .def("num_gates", &QuantumCircuit::num_gates)
        .def("size", &QuantumCircuit::size)
        .def("print", &QuantumCircuit::print, "Print a human-readable representation of the circuit")

        .def_property_readonly("num", &QuantumCircuit::num_qubits)
        .def_property_readonly("gates", &QuantumCircuit::get_gates)
        .def_property_readonly("instructions", &QuantumCircuit::get_instructions)
        .def_property_readonly("variables", &QuantumCircuit::get_variables)
        .def_property_readonly("measures", &get_measures_dict)

        // Fluent single-qubit non-parametric gates
        .def("h", &QuantumCircuit::h, py::arg("qubit"))
        .def("x", &QuantumCircuit::x, py::arg("qubit"))
        .def("y", &QuantumCircuit::y, py::arg("qubit"))
        .def("z", &QuantumCircuit::z, py::arg("qubit"))
        .def("s", &QuantumCircuit::s, py::arg("qubit"))
        .def("sdg", &QuantumCircuit::sdg, py::arg("qubit"))
        .def("t", &QuantumCircuit::t, py::arg("qubit"))
        .def("tdg", &QuantumCircuit::tdg, py::arg("qubit"))

        // Fluent single-qubit parametric gates (only Expr version)
        .def("rx", &QuantumCircuit::rx, py::arg("theta"), py::arg("qubit"))
        .def("ry", &QuantumCircuit::ry, py::arg("phi"), py::arg("qubit"))
        .def("rz", &QuantumCircuit::rz, py::arg("lambda"), py::arg("qubit"))
        .def("p", &QuantumCircuit::p, py::arg("lambda"), py::arg("qubit"))

        // Fluent two-qubit parametric gates
        .def("rxx", &QuantumCircuit::rxx, py::arg("theta"), py::arg("qubit1"), py::arg("qubit2"))
        .def("ryy", &QuantumCircuit::ryy, py::arg("theta"), py::arg("qubit1"), py::arg("qubit2"))
        .def("rzz", &QuantumCircuit::rzz, py::arg("theta"), py::arg("qubit1"), py::arg("qubit2"))

        // Fluent two-qubit gates
        .def("cnot", &QuantumCircuit::cnot, py::arg("control"), py::arg("target"))
        .def("cz", &QuantumCircuit::cz, py::arg("control"), py::arg("target"))
        .def("swap", &QuantumCircuit::swap, py::arg("qubit1"), py::arg("qubit2"))
        .def("iswap", &QuantumCircuit::iswap, py::arg("qubit1"), py::arg("qubit2"))
        .def("toffoli", &QuantumCircuit::toffoli, py::arg("control1"), py::arg("control2"), py::arg("target"))

        // Common aliases
        .def("cx", &QuantumCircuit::cnot, py::arg("control"), py::arg("target"))
        .def("ccx", &QuantumCircuit::toffoli, py::arg("control1"), py::arg("control2"), py::arg("target"))


        // Append a generic instruction
        .def("append", &QuantumCircuit::append, py::arg("instruction"), "Append a CircuitInstruction to the circuit.")


        // Fluent other operations
        .def("measure", py::overload_cast<const std::vector<int>&, const std::vector<int>&>(&QuantumCircuit::measure), py::arg("qubits"), py::arg("clbits"))
        .def("measure", py::overload_cast<const std::map<int, int>&>(&QuantumCircuit::measure), py::arg("qubit_clbit_map"), "Measure qubits to classical bits using a dictionary mapping")
        .def("measure_all", &QuantumCircuit::measure_all)
        .def("barrier", &QuantumCircuit::barrier, py::arg("qubits") = std::vector<int>{})
        .def("to_openqasm", &QuantumCircuit::to_openqasm, py::arg("with_para") = false, "Exports the circuit to an OpenQASM 2.0 string.");




    // Single-qubit non-parametric gates
    bind_gate_1q_nonparam(m, "H", H);
    bind_gate_1q_nonparam(m, "X", X);
    bind_gate_1q_nonparam(m, "Y", Y);
    bind_gate_1q_nonparam(m, "Z", Z);
    bind_gate_1q_nonparam(m, "S", S);
    bind_gate_1q_nonparam(m, "Sdg", Sdg);
    bind_gate_1q_nonparam(m, "T", T);
    bind_gate_1q_nonparam(m, "Tdg", Tdg);
    bind_gate_1q_nonparam(m, "Id", Id);
    bind_gate_1q_nonparam(m, "SX", SX);
    bind_gate_1q_nonparam(m, "SXdg", SXdg);
    bind_gate_1q_nonparam(m, "SY", SY);
    bind_gate_1q_nonparam(m, "SYdg", SYdg);
    bind_gate_1q_nonparam(m, "W", W);
    bind_gate_1q_nonparam(m, "SW", SW);
    bind_gate_1q_nonparam(m, "SWdg", SWdg);

    // Single-qubit parametric gates
    bind_gate_1q_param(m, "RX", RX);
    bind_gate_1q_param(m, "RY", RY);
    bind_gate_1q_param(m, "RZ", RZ);
    bind_gate_1q_param(m, "Phase", Phase);

    // U3 gate (special case: 3 parameters)
    py::class_<GateFactory<U3Gate>>(m, "U3Factory")
        .def("__call__", [](const GateFactory<U3Gate>& self, const Expr& theta, const Expr& phi, const Expr& lambda, int qubit) {
            return self(theta, phi, lambda, qubit);
        }, py::arg("theta"), py::arg("phi"), py::arg("lambda"), py::arg("qubit"))
        .def_readonly_static("name", &GateFactory<U3Gate>::name);  // Read-only static attribute
    m.attr("U3") = &U3;

    // Two-qubit non-parametric gates
    bind_gate_2q_nonparam(m, "CNOT", CNOT);
    m.attr("CX") = m.attr("CNOT");  // CX is an alias for CNOT
    bind_gate_2q_nonparam(m, "CZ", CZ);
    bind_gate_2q_nonparam(m, "CY", CY);
    bind_gate_2q_nonparam(m, "CS", CS);
    bind_gate_2q_nonparam(m, "CT", CT);
    bind_gate_2q_nonparam(m, "Swap", Swap);
    bind_gate_2q_nonparam(m, "ISwap", ISwap);

    // Two-qubit parametric gates
    bind_gate_2q_param(m, "RXX", RXX);
    bind_gate_2q_param(m, "RYY", RYY);
    bind_gate_2q_param(m, "RZZ", RZZ);
    bind_gate_2q_param(m, "CP", CP);
    bind_gate_2q_param(m, "CRX", CRX);
    bind_gate_2q_param(m, "CRY", CRY);
    bind_gate_2q_param(m, "CRZ", CRZ);

    // Three-qubit gates
    bind_gate_3q_nonparam(m, "Toffoli", Toffoli);
    m.attr("CCX") = m.attr("Toffoli");
    bind_gate_3q_nonparam(m, "Fredkin", Fredkin);
    m.attr("CSWAP") = m.attr("Fredkin");

    // Multi-controlled gates
    bind_gate_mc_nonparam(m, "MCX", MCX);
    bind_gate_mc_nonparam(m, "MCY", MCY);
    bind_gate_mc_nonparam(m, "MCZ", MCZ);
    bind_gate_mc_param(m, "MCRX", MCRX);
    bind_gate_mc_param(m, "MCRY", MCRY);
    bind_gate_mc_param(m, "MCRZ", MCRZ);
    bind_gate_mc_nonparam(m, "ControlledU", ControlledU);


    // Non-gate operation factories
    py::class_<MeasureFactory>(m, "MeasureFactory")
        .def("__call__", py::overload_cast<int, int>(&MeasureFactory::operator(), py::const_),
             py::arg("qubit"), py::arg("clbit"))
        .def("__call__", py::overload_cast<const std::vector<int>&, const std::vector<int>&>(&MeasureFactory::operator(), py::const_),
             py::arg("qubits"), py::arg("clbits"))
        .def_readonly_static("name", &MeasureFactory::name);  // Read-only static attribute
    m.attr("Measure") = &Measure;

    py::class_<BarrierFactory>(m, "BarrierFactory")
        .def("__call__", &BarrierFactory::operator(), py::arg("qubits"))
        .def_readonly_static("name", &BarrierFactory::name);  // Read-only static attribute
    m.attr("Barrier") = &Barrier;

    py::class_<ResetFactory>(m, "ResetFactory")
        .def("__call__", &ResetFactory::operator(), py::arg("qubit"))
        .def_readonly_static("name", &ResetFactory::name);  // Read-only static attribute
    m.attr("Reset") = &Reset;

    py::class_<DelayFactory>(m, "DelayFactory")
        .def("__call__", &DelayFactory::operator(), py::arg("qubit"), py::arg("duration"), py::arg("unit") = "ns")
        .def_readonly_static("name", &DelayFactory::name);  // Read-only static attribute
    m.attr("Delay") = &Delay;

    py::class_<XYResonanceFactory>(m, "XYResonanceFactory")
        .def("__call__", &XYResonanceFactory::operator(),
             py::arg("qubit_start"), py::arg("qubit_end"), py::arg("duration"), py::arg("unit") = "ns")
        .def_readonly_static("name", &XYResonanceFactory::name);  // Read-only static attribute
    m.attr("XYResonance") = &XYResonance;

    // Add bindings for the new conversion functions
    m.def("circuit_to_dag", &circuit_to_dag,
            py::arg("circuit"),
            "Converts a QuantumCircuit to a DAGCircuit.");

            m.def("dag_to_circuit", &dag_to_circuit,
                  py::arg("dag"), py::arg("num_qubits"),
                  "Converts a DAGCircuit to a QuantumCircuit.");}

