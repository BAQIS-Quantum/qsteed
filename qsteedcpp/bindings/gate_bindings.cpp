#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
#include <pybind11/complex.h>
#include <pybind11/numpy.h>
#include "gates/base_gate.h"


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

}