#pragma once
#include <pybind11/pybind11.h>

namespace py = pybind11;

// 声明门绑定函数
void bind_gates(py::module& m); 