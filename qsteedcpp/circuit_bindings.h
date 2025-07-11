#pragma once
#include <pybind11/pybind11.h>

namespace py = pybind11;

void bind_quantum_circuit(py::module& m);