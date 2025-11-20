#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include <pybind11/operators.h>
#include <pybind11/stl_bind.h>
#include <sstream>

#include "../QuantumCircuit/circuit/quantum_circuit.h"
#include "../QuantumCircuit/circuit/circuit_instruction.h"

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


void bind_quantum_circuit(py::module& m) {
    // Bind instruction components
    py::class_<Measurement>(m, "Measurement")
        .def_readonly("qubits", &Measurement::qubit_indices)
        .def_readonly("clbits", &Measurement::clbit_indices)
        .def("__repr__", [](const Measurement& meas) {
            return "Measurement(qubits=" + vec_to_string(meas.qubit_indices) + 
                   ", clbits=" + vec_to_string(meas.clbit_indices) + ")";
        });

    py::class_<Barrier>(m, "Barrier")
        .def_readonly("qubits", &Barrier::qubits)
        .def("__repr__", [](const Barrier& barrier) {
            return "Barrier(qubits=" + vec_to_string(barrier.qubits) + ")";
        });

    py::class_<Reset>(m, "Reset")
        .def_readonly("qubit", &Reset::qubit_index)
        .def("__repr__", [](const Reset& reset) {
            return "Reset(qubit=" + std::to_string(reset.qubit_index) + ")";
        });
        
    py::class_<Condition>(m, "Condition")
        .def_readonly("clbit", &Condition::clbit_index)
        .def_readonly("value", &Condition::value)
        .def("__repr__", [](const Condition& cond) {
            return "Condition(clbit=" + std::to_string(cond.clbit_index) + 
                   ", value=" + std::to_string(cond.value) + ")";
        });


    // Bind the main instruction class
    py::class_<CircuitInstruction>(m, "CircuitInstruction")
        .def_property_readonly("name", &CircuitInstruction::name)
        .def_property_readonly("qubits", [](const CircuitInstruction& inst) { return inst.qubits; })
        .def_property_readonly("clbits", [](const CircuitInstruction& inst) { return inst.clbits; })
        .def_property_readonly("condition", [](const CircuitInstruction& inst) { return inst.condition; })
        .def("is_gate", &CircuitInstruction::is_gate)
        .def("is_measurement", &CircuitInstruction::is_measurement)
        .def("is_barrier", &CircuitInstruction::is_barrier)
        .def("is_reset", &CircuitInstruction::is_reset)
        .def("__repr__", [](const CircuitInstruction& inst) {
            std::string repr = "Instruction(name='" + inst.name() + 
                               "', qubits=" + vec_to_string(inst.qubits);
            if (!inst.clbits.empty()) {
                repr += ", clbits=" + vec_to_string(inst.clbits);
            }
            if (inst.condition) {
                repr += ", condition=if(c" + std::to_string(inst.condition->clbit_index) + 
                        "==" + std::to_string(inst.condition->value) + ")";
            }
            repr += ")";
            return repr;
        });
    
    // Bind the vector of instructions to allow iteration in Python
    py::bind_vector<std::vector<CircuitInstruction>>(m, "InstructionVec");


    py::class_<QuantumCircuit>(m, "QuantumCircuit")
        // Constructors
        .def(py::init<int, int>(), py::arg("num_qubits"), py::arg("num_clbits") = 0)

        // Basic properties
        .def("num_qubits", &QuantumCircuit::num_qubits)
        .def("num_clbits", &QuantumCircuit::num_clbits)
        .def("num_gates", &QuantumCircuit::num_gates)
        .def("size", &QuantumCircuit::size, "Returns the total number of instructions in the circuit.")
       
        // Method to get all instructions
        .def("get_instructions", &QuantumCircuit::get_instructions,
             "Get the list of all instructions in the circuit.",
             py::return_value_policy::reference_internal)
        .def("print", &QuantumCircuit::print)

        // Single-qubit non-parametric gates
        .def("h", &QuantumCircuit::h, py::arg("qubit"), "Apply Hadamard gate")
        .def("x", &QuantumCircuit::x, py::arg("qubit"), "Apply Pauli-X gate")
        .def("y", &QuantumCircuit::y, py::arg("qubit"), "Apply Pauli-Y gate")
        .def("z", &QuantumCircuit::z, py::arg("qubit"), "Apply Pauli-Z gate")
        .def("s", &QuantumCircuit::s, py::arg("qubit"), "Apply S gate")
        .def("sdg", &QuantumCircuit::sdg, py::arg("qubit"), "Apply S-dagger gate")
        .def("t", &QuantumCircuit::t, py::arg("qubit"), "Apply T gate")
        .def("tdg", &QuantumCircuit::tdg, py::arg("qubit"), "Apply T-dagger gate")

        // Single-qubit parametric gates (Expr version)
        .def("rx", py::overload_cast<const Expr&, int>(&QuantumCircuit::rx),
             py::arg("theta"), py::arg("qubit"), "Apply RX rotation gate")
        .def("ry", py::overload_cast<const Expr&, int>(&QuantumCircuit::ry),
             py::arg("phi"), py::arg("qubit"), "Apply RY rotation gate")
        .def("rz", py::overload_cast<const Expr&, int>(&QuantumCircuit::rz),
             py::arg("lambda"), py::arg("qubit"), "Apply RZ rotation gate")
        .def("p", py::overload_cast<const Expr&, int>(&QuantumCircuit::p),
             py::arg("lambda"), py::arg("qubit"), "Apply phase gate")

        // Single-qubit parametric gates (Parameter version - direct, no implicit conversion)
        .def("rx", py::overload_cast<const Parameter&, int>(&QuantumCircuit::rx),
             py::arg("theta"), py::arg("qubit"), "Apply RX rotation gate with Parameter")
        .def("ry", py::overload_cast<const Parameter&, int>(&QuantumCircuit::ry),
             py::arg("phi"), py::arg("qubit"), "Apply RY rotation gate with Parameter")
        .def("rz", py::overload_cast<const Parameter&, int>(&QuantumCircuit::rz),
             py::arg("lambda"), py::arg("qubit"), "Apply RZ rotation gate with Parameter")
        .def("p", py::overload_cast<const Parameter&, int>(&QuantumCircuit::p),
             py::arg("lambda"), py::arg("qubit"), "Apply phase gate with Parameter")

        // Single-qubit parametric gates (double version)
        .def("rx", py::overload_cast<double, int>(&QuantumCircuit::rx),
             py::arg("theta"), py::arg("qubit"), "Apply RX rotation gate")
        .def("ry", py::overload_cast<double, int>(&QuantumCircuit::ry),
             py::arg("phi"), py::arg("qubit"), "Apply RY rotation gate")
        .def("rz", py::overload_cast<double, int>(&QuantumCircuit::rz),
             py::arg("lambda"), py::arg("qubit"), "Apply RZ rotation gate")
        .def("p", py::overload_cast<double, int>(&QuantumCircuit::p),
             py::arg("lambda"), py::arg("qubit"), "Apply phase gate")

        // Two-qubit non-parametric gates
        .def("cnot", &QuantumCircuit::cnot,
             py::arg("control"), py::arg("target"), "Apply CNOT gate")
        .def("cz", &QuantumCircuit::cz,
             py::arg("control"), py::arg("target"), "Apply CZ gate")
        .def("swap", &QuantumCircuit::swap,
             py::arg("qubit1"), py::arg("qubit2"), "Apply SWAP gate")
        .def("iswap", &QuantumCircuit::iswap,
             py::arg("qubit1"), py::arg("qubit2"), "Apply iSWAP gate")

        // Measurements and other operations
        .def("measure", py::overload_cast<int, int>(&QuantumCircuit::measure),
             py::arg("qubit"), py::arg("clbit"), "Measure a qubit")
        .def("measure", py::overload_cast<const std::vector<int>&, const std::vector<int>&>(&QuantumCircuit::measure),
             py::arg("qubits"), py::arg("clbits"), "Measure multiple qubits")
        .def("measure_all", &QuantumCircuit::measure_all, "Measure all qubits")
        .def("barrier", &QuantumCircuit::barrier,
             py::arg("qubits") = std::vector<int>{}, "Add a barrier")
        .def("reset", &QuantumCircuit::reset,
             py::arg("qubit"), "Reset a qubit to |0>")

        .def("__repr__", [](const QuantumCircuit& circuit) {
            return "QuantumCircuit(num_qubits=" + std::to_string(circuit.num_qubits()) +
                   ", num_clbits=" + std::to_string(circuit.num_clbits()) +
                   ", instructions=" + std::to_string(circuit.size()) + ")";
        })

        .def("__len__", &QuantumCircuit::size);
}
