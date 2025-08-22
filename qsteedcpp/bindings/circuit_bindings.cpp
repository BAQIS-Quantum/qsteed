#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include <pybind11/operators.h>
#include <pybind11/stl_bind.h>
#include <sstream>

#include "../QuantumCircuit/include/circuit/quantum_circuit.h"
#include "../QuantumCircuit/include/circuit/circuit_instruction.h"

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
    py::class_<ParameterGradInfo>(m, "ParameterGradInfo")
        .def(py::init<size_t, size_t, double>(), 
             py::arg("gate_index"), py::arg("param_index"), py::arg("grad_value"))
        .def_readwrite("gate_index", &ParameterGradInfo::gate_index)
        .def_readwrite("param_index", &ParameterGradInfo::param_index)
        .def_readwrite("grad_value", &ParameterGradInfo::grad_value)
        .def("__repr__", [](const ParameterGradInfo& info) {
            return "ParameterGradInfo(gate_index=" + std::to_string(info.gate_index) +
                   ", param_index=" + std::to_string(info.param_index) +
                   ", grad_value=" + std::to_string(info.grad_value) + ")";
        });

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
        .def("get_unitary_matrix", &QuantumCircuit::get_unitary_matrix,
             "Calculates and returns the unitary matrix representation of the circuit.\n\n" 
             "Warning: This can be very memory-intensive for circuits with a large number of qubits.")
        
        // Method to get all instructions
        .def("get_instructions", &QuantumCircuit::get_instructions, 
             "Get the list of all instructions in the circuit.",
             py::return_value_policy::reference_internal)

        // Single-qubit gates
        .def("h", &QuantumCircuit::h, py::arg("qubit"))
        .def("x", &QuantumCircuit::x, py::arg("qubit"))
        .def("y", &QuantumCircuit::y, py::arg("qubit"))
        .def("z", &QuantumCircuit::z, py::arg("qubit"))
        .def("s", &QuantumCircuit::s, py::arg("qubit"))
        .def("sdg", &QuantumCircuit::sdg, py::arg("qubit"))
        .def("t", &QuantumCircuit::t, py::arg("qubit"))
        .def("tdg", &QuantumCircuit::tdg, py::arg("qubit"))
        
        // Single-qubit parameterized gates
        .def("rx", &QuantumCircuit::rx, py::arg("theta"), py::arg("qubit"))
        .def("ry", &QuantumCircuit::ry, py::arg("phi"), py::arg("qubit"))
        .def("rz", &QuantumCircuit::rz, py::arg("lambda"), py::arg("qubit"))
        .def("p", &QuantumCircuit::p, py::arg("lambda"), py::arg("qubit"))
        .def("u3", &QuantumCircuit::u3, py::arg("theta"), py::arg("phi"), py::arg("lambda"), py::arg("qubit"))

        // Two-qubit gates
        .def("cnot", &QuantumCircuit::cnot, py::arg("control"), py::arg("target"))
        .def("cz", &QuantumCircuit::cz, py::arg("control"), py::arg("target"))
        .def("swap", &QuantumCircuit::swap, py::arg("qubit1"), py::arg("qubit2"))
        .def("iswap", &QuantumCircuit::iswap, py::arg("qubit1"), py::arg("qubit2"))

        // Two-qubit parameterized gates
        .def("rxx", &QuantumCircuit::rxx, py::arg("theta"), py::arg("qubit1"), py::arg("qubit2"))
        .def("ryy", &QuantumCircuit::ryy, py::arg("phi"), py::arg("qubit1"), py::arg("qubit2"))
        .def("rzz", &QuantumCircuit::rzz, py::arg("lambda"), py::arg("qubit1"), py::arg("qubit2"))
        
        // Three-qubit gates
        .def("ccx", &QuantumCircuit::ccx, py::arg("control1"), py::arg("control2"), py::arg("target"))
        .def("toffoli", &QuantumCircuit::toffoli, py::arg("control1"), py::arg("control2"), py::arg("target"))

        // Circuit operations
        .def("measure", py::overload_cast<int, int>(&QuantumCircuit::measure), 
             py::arg("qubit"), py::arg("clbit"))
        .def("measure", py::overload_cast<const std::vector<int>&, const std::vector<int>&>(&QuantumCircuit::measure), 
             py::arg("qubits"), py::arg("clbits"))
        .def("measure_all", &QuantumCircuit::measure_all)
        .def("reset", &QuantumCircuit::reset, py::arg("qubit"))
        .def("barrier", &QuantumCircuit::barrier, py::arg("qubits") = std::vector<int>{})

        // Parameter related methods
        .def("get_parameters", &QuantumCircuit::get_parameters)
        .def("get_parameter_grads", &QuantumCircuit::get_parameter_grads,
             "Get parameter gradients information",
             py::return_value_policy::reference_internal)
        .def("get_variables", &QuantumCircuit::get_variables,
             "Get all variables (unique parameters) in the circuit",
             py::return_value_policy::reference_internal)
        .def("update_parameters", &QuantumCircuit::update_parameters,
             py::arg("param_values"),
             "Update parameter values")
        .def("compute_gradients_for_parameter", &QuantumCircuit::compute_gradients_for_parameter,
             py::arg("gate_index"), py::arg("param_index"), py::arg("param_values"),
             "Compute gradients for a specific parameter using autodiff")
        .def("print_parameter_grads", &QuantumCircuit::print_parameter_grads,
             "Print parameter gradients information for debugging")
        
        // Convenient method: Return parameter gradients in a Python-friendly format
        .def("get_parameter_grads_dict", [](const QuantumCircuit& circuit) {
            const auto& grads = circuit.get_parameter_grads();
            py::dict result;
            
            for (const auto& pair : grads) {
                const std::string& var_name = pair.first;
                const std::vector<ParameterGradInfo>& grad_infos = pair.second;
                
                py::list grad_list;
                for (const auto& info : grad_infos) {
                    py::dict grad_dict;
                    grad_dict["gate_index"] = info.gate_index;
                    grad_dict["param_index"] = info.param_index;
                    grad_dict["grad_value"] = info.grad_value;
                    grad_list.append(grad_dict);
                }
                result[var_name.c_str()] = grad_list;
            }
            return result;
        }, "Get parameter gradients as Python dictionary")
        
        .def("has_parameters", [](const QuantumCircuit& circuit) {
            return !circuit.get_variables().empty();
        }, "Check if the circuit has parameters")
        
        .def("print", &QuantumCircuit::print)
        
        .def("__repr__", [](const QuantumCircuit& circuit) {
            return "QuantumCircuit(num_qubits=" + std::to_string(circuit.num_qubits()) + 
                   ", num_clbits=" + std::to_string(circuit.num_clbits()) +
                   ", instructions=" + std::to_string(circuit.size()) + ")";
        })
        
        .def("__len__", &QuantumCircuit::size);
}
