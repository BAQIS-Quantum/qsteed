#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
#include <pybind11/complex.h>
#include "gates/base_gate.h"
#include "gates/standard_gates.h"
#include "circuit/parameter.h"
#include "gates/matrix.h"

namespace py = pybind11;
using namespace qsteedcpp;

void bind_gates(py::module& m) {
    // 绑定 std::complex<double> 作为 Complex
    py::class_<std::complex<double>>(m, "Complex")
        .def(py::init<double, double>())
        .def(py::init<double>())
        .def_property("real", 
                     [](const std::complex<double>& c) { return c.real(); },
                     [](std::complex<double>& c, double r) { c = std::complex<double>(r, c.imag()); })
        .def_property("imag", 
                     [](const std::complex<double>& c) { return c.imag(); },
                     [](std::complex<double>& c, double i) { c = std::complex<double>(c.real(), i); })
        .def("__repr__", [](const std::complex<double>& c) {
            return "Complex(" + std::to_string(c.real()) + ", " + std::to_string(c.imag()) + ")";
        });

    // 绑定 Matrix 类
    py::class_<Matrix>(m, "Matrix")
        .def(py::init<>())
        .def(py::init<size_t, size_t>())
        .def(py::init<const std::vector<std::vector<Complex>>&>())
        .def("get_element", &Matrix::get_element)
        .def("set_element", &Matrix::set_element)
        .def("rows", &Matrix::rows)
        .def("cols", &Matrix::cols)
        .def("get_rows", &Matrix::get_rows)
        .def("get_cols", &Matrix::get_cols)
        .def("get_data", &Matrix::get_data)
        .def("transpose", &Matrix::transpose)
        .def("dagger", &Matrix::dagger)
        .def("conjugate_transpose", &Matrix::conjugate_transpose)
        .def("conjugate", &Matrix::conjugate)
        .def("determinant", &Matrix::determinant)
        .def("trace", &Matrix::trace)
        .def("norm", &Matrix::norm)
        .def("frobenius_norm", &Matrix::frobenius_norm)
        .def("inverse", &Matrix::inverse)
        .def("exp", &Matrix::exp)
        .def("log", &Matrix::log)
        .def("is_unitary", &Matrix::is_unitary)
        .def("is_hermitian", &Matrix::is_hermitian)
        .def("is_diagonal", &Matrix::is_diagonal)
        .def("print", &Matrix::print)
        .def("print_matlab", &Matrix::print_matlab)
        .def_static("identity", &Matrix::identity)
        .def_static("zeros", &Matrix::zeros)
        .def_static("random", &Matrix::random)
        .def("__mul__", [](const Matrix& a, const Matrix& b) { return a * b; })
        .def("__add__", [](const Matrix& a, const Matrix& b) { return a + b; })
        .def("__sub__", [](const Matrix& a, const Matrix& b) { return a - b; })
        .def("__repr__", [](const Matrix& m) {
            std::string result = "Matrix(" + std::to_string(m.rows()) + "x" + std::to_string(m.cols()) + ")";
            return result;
        });

    // 绑定 Parameter 相关类
    py::class_<Parameter>(m, "GateParameter")
        .def(py::init<double>())
        .def(py::init<const std::string&>())
        .def_static("variable", &Parameter::variable)
        .def("value", &Parameter::value)
        .def("set_value", &Parameter::set_value)
        .def("get_name", &Parameter::get_name)
        .def("get_variables", &Parameter::get_variables)
        .def("to_string", &Parameter::to_string)
        .def("compute_gradients", &Parameter::compute_gradients)
        // 运算符重载
        .def("__add__", [](const Parameter& a, const Parameter& b) { return a + b; })
        .def("__add__", [](const Parameter& a, double b) { return a + b; })
        .def("__radd__", [](const Parameter& a, double b) { return b + a; })
        .def("__sub__", [](const Parameter& a, const Parameter& b) { return a - b; })
        .def("__sub__", [](const Parameter& a, double b) { return a - b; })
        .def("__rsub__", [](const Parameter& a, double b) { return b - a; })
        .def("__mul__", [](const Parameter& a, const Parameter& b) { return a * b; })
        .def("__mul__", [](const Parameter& a, double b) { return a * b; })
        .def("__rmul__", [](const Parameter& a, double b) { return b * a; })
        .def("__truediv__", [](const Parameter& a, const Parameter& b) { return a / b; })
        .def("__truediv__", [](const Parameter& a, double b) { return a / b; })
        .def("__rtruediv__", [](const Parameter& a, double b) { return b / a; })
        .def("__neg__", [](const Parameter& a) { return -a; })
        .def("__repr__", [](const Parameter& p) {
            return "Parameter(" + p.to_string() + ")";
        });

    // 数学函数
    m.def("sin", [](const Parameter& p) { return sin(p); });
    m.def("cos", [](const Parameter& p) { return cos(p); });
    m.def("tan", [](const Parameter& p) { return tan(p); });
    m.def("exp", [](const Parameter& p) { return exp(p); });
    m.def("log", [](const Parameter& p) { return log(p); });
    m.def("sqrt", [](const Parameter& p) { return sqrt(p); });
    m.def("pow", [](const Parameter& p, double exponent) { return pow(p, exponent); });

    // 绑定 Gate 基类
    py::class_<Gate>(m, "Gate")
        .def("get_name", &Gate::get_name)
        .def("get_qubit_count", &Gate::get_qubit_count)
        .def("has_parameters", &Gate::has_parameters)
        .def("parameter_count", &Gate::parameter_count)
        .def("get_parameter", &Gate::get_parameter, py::return_value_policy::reference)
        .def("set_parameter", &Gate::set_parameter)
        .def("get_parameter_values", &Gate::get_parameter_values)
        .def("get_matrix", &Gate::get_matrix)
        .def("apply", &Gate::apply)
        .def("__repr__", [](const Gate& g) {
            return g.get_name() + "Gate(" + std::to_string(g.get_qubit_count()) + " qubits)";
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