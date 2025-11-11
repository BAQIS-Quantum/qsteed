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

        // Add the new binding here
       
        // Method to get all instructions
        .def("get_instructions", &QuantumCircuit::get_instructions, 
             "Get the list of all instructions in the circuit.",
             py::return_value_policy::reference_internal)
       .def("print", &QuantumCircuit::print)
        
        .def("__repr__", [](const QuantumCircuit& circuit) {
            return "QuantumCircuit(num_qubits=" + std::to_string(circuit.num_qubits()) + 
                   ", num_clbits=" + std::to_string(circuit.num_clbits()) +
                   ", instructions=" + std::to_string(circuit.size()) + ")";
        })
        
        .def("__len__", &QuantumCircuit::size);
}
