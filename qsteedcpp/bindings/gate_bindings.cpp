#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
#include <pybind11/complex.h>
#include <pybind11/numpy.h>
#include "gates/base_gate.h"
#include "gates/standard_gates.h"


namespace py = pybind11;
using namespace qsteedcpp;

void bind_gates(py::module& m) {
    py::class_<Gate>(m, "Gate")
        .def("name", &Gate::name)
        .def("get_qubit_count", &Gate::get_qubit_count)
        .def("has_parameters", &Gate::has_parameters)
        .def("parameter_count", &Gate::parameter_count)
        .def("__repr__", [](const Gate& g) {
            return std::string(g.name()) + "Gate(" + std::to_string(g.get_qubit_count()) + " qubits)";
        });

    // 绑定具体的门类
    py::class_<HGate, Gate>(m, "HGate")
        .def(py::init<>());

    py::class_<XGate, Gate>(m, "XGate")
        .def(py::init<>());

    py::class_<YGate, Gate>(m, "YGate")
        .def(py::init<>());

    py::class_<ZGate, Gate>(m, "ZGate")
        .def(py::init<>());

    py::class_<RXGate, Gate>(m, "RXGate")
        .def(py::init<const Parameter&>());

    py::class_<RYGate, Gate>(m, "RYGate")
        .def(py::init<const Parameter&>());

    py::class_<RZGate, Gate>(m, "RZGate")
        .def(py::init<const Parameter&>());

    py::class_<CNOTGate, Gate>(m, "CNOTGate")
        .def(py::init<>());

    // 绑定便捷的工厂函数
    m.def("H", &H);
    m.def("X", &X);
    m.def("Y", &Y);
    m.def("Z", &Z);
    m.def("RX", &RX);
    m.def("RY", &RY);
    m.def("RZ", &RZ);
    m.def("CNOT", &CNOT);
}