#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include "circuit_bindings.h"
#include "Gates/include/gates/quantum_circuit.h"
#include "Gates/include/gates/parameter.h"
#include "Gates/include/gates/standard_gates.h"

namespace py = pybind11;
using namespace qsteedcpp;

void bind_quantum_circuit(py::module& m) {
    // 绑定 ParameterGradInfo 结构体
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

    // 绑定 QuantumCircuit 类
    py::class_<QuantumCircuit>(m, "QuantumCircuit")
        .def(py::init<int>(), py::arg("num_qubits"))
        .def("num_qubits", &QuantumCircuit::num_qubits)
        .def("num_gates", &QuantumCircuit::num_gates)
        
        // 单量子比特门方法
        .def("h", &QuantumCircuit::h, py::arg("qubit"))
        .def("x", &QuantumCircuit::x, py::arg("qubit"))
        .def("y", &QuantumCircuit::y, py::arg("qubit"))
        .def("z", &QuantumCircuit::z, py::arg("qubit"))
        
        // 参数化旋转门方法
        .def("rx", &QuantumCircuit::rx, py::arg("theta"), py::arg("qubit"))
        .def("ry", &QuantumCircuit::ry, py::arg("phi"), py::arg("qubit"))
        .def("rz", &QuantumCircuit::rz, py::arg("lambda"), py::arg("qubit"))
        
        // 两量子比特门方法
        .def("cnot", &QuantumCircuit::cnot, py::arg("control"), py::arg("target"))
        
        // 通用门添加方法
        .def("add_gate", [](QuantumCircuit& circuit, Gate& gate, const std::vector<int>& qubits) {
            std::unique_ptr<Gate> gate_copy;
            
            // 根据门的类型创建副本
            if (gate.get_name() == "H") {
                gate_copy = std::make_unique<HGate>();
            } else if (gate.get_name() == "X") {
                gate_copy = std::make_unique<XGate>();
            } else if (gate.get_name() == "Y") {
                gate_copy = std::make_unique<YGate>();
            } else if (gate.get_name() == "Z") {
                gate_copy = std::make_unique<ZGate>();
            } else if (gate.get_name() == "RX") {
                gate_copy = std::make_unique<RXGate>(gate.get_parameter(0));
            } else if (gate.get_name() == "RY") {
                gate_copy = std::make_unique<RYGate>(gate.get_parameter(0));
            } else if (gate.get_name() == "RZ") {
                gate_copy = std::make_unique<RZGate>(gate.get_parameter(0));
            } else if (gate.get_name() == "CNOT") {
                gate_copy = std::make_unique<CNOTGate>();
            } else {
                throw std::runtime_error("Unknown gate type: " + gate.get_name());
            }
            
            circuit.add_gate(std::move(gate_copy), qubits);
        }, py::arg("gate"), py::arg("qubits"))
        
        // 获取参数列表
        .def("get_parameters", &QuantumCircuit::get_parameters)
        
        // 参数梯度相关方法
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
        
        // 便利方法：以Python友好的格式返回参数梯度
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
        
        // 便利方法：检查电路是否有参数
        .def("has_parameters", [](const QuantumCircuit& circuit) {
            return !circuit.get_variables().empty();
        }, "Check if the circuit has parameters")
        
        // 打印电路
        .def("print", &QuantumCircuit::print)
        
        .def("__repr__", [](const QuantumCircuit& circuit) {
            return "QuantumCircuit(" + std::to_string(circuit.num_qubits()) + 
                   " qubits, " + std::to_string(circuit.num_gates()) + " gates)";
        })
        
        // 支持 len() 函数
        .def("__len__", &QuantumCircuit::num_gates);
}