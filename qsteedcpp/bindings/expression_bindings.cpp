#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>

#include "../QuantumCircuit/expression/expr.h"
#include "../QuantumCircuit/expression/nodes/parameter.h"

namespace py = pybind11;
using namespace qsteedcpp;

void bind_expressions(py::module& m) {
    // Create expression submodule
    py::module expr_module = m.def_submodule("expression", "Expression system for parametric quantum circuits");

     // pre declaration of Parameter class & Expr class
     py::class_<Expr> expr_class(expr_module, "Expr",
     "An expression representing a mathematical formula.\n\n"
        "Expr can represent constants, parameters, or mathematical operations\n"
        "on them. It supports arithmetic operations and mathematical functions.\n\n"
        "Example:\n"
        "    theta = Parameter(0.5, trainable=True)\n"
        "    phi = Parameter(0.3, trainable=True)\n"
        "    angle = 2 * theta + phi  # Creates an Expr\n"
        "    qc.rx(angle, 0)");

     py::class_<Parameter, std::shared_ptr<Parameter>> parameter_class(expr_module, "Parameter", 
        "A trainable or fixed parameter in a quantum circuit.\n\n"
        "Parameters can be used in parametric gates (RX, RY, RZ, etc.) and\n"
        "can be trained using gradient-based optimization when used with\n"
        "TorchBackend.\n\n"
        "Example:\n"
        "    theta = Parameter(0.5, trainable=True)\n"
        "    qc.rx(theta, 0)\n"
        "    backend.compile(qc)\n"
        "    theta_tensor = backend.get_parameter(theta)"
     );

     // Bind Expr class
     expr_class
        .def(py::init<double>(),
             py::arg("value"),
             "Create an Expr from a constant value.\n\n"
             "Args:\n"
             "    value: Constant value")
        .def(py::init<const Parameter&>(),
             py::arg("param"),
             "Create an Expr from a Parameter.\n\n"
             "Args:\n"
             "    param: Parameter object")
        .def("eval", &Expr::eval,
             "Evaluate the expression to a numeric value.\n\n"
             "Returns:\n"
             "    float: Result of evaluation")
        .def("to_string", &Expr::to_string,
             "Get string representation of the expression.\n\n"
             "Returns:\n"
             "    str: String representation")
        // Arithmetic operators
        .def("__add__", [](const Expr& a, const Expr& b) { return a + b; })
        .def("__add__", [](const Expr& a, const Parameter& b) { return a + Expr(b); })
        .def("__add__", [](const Expr& a, double b) { return a + b; })
        .def("__radd__", [](const Expr& a, const Parameter& b) { return Expr(b) + a; })
        .def("__radd__", [](const Expr& a, double b) { return b + a; })
        .def("__sub__", [](const Expr& a, const Expr& b) { return a - b; })
        .def("__sub__", [](const Expr& a, const Parameter& b) { return a - Expr(b); })
        .def("__sub__", [](const Expr& a, double b) { return a - b; })
        .def("__rsub__", [](const Expr& a, const Parameter& b) { return Expr(b) - a; })
        .def("__rsub__", [](const Expr& a, double b) { return b - a; })
        .def("__mul__", [](const Expr& a, const Expr& b) { return a * b; })
        .def("__mul__", [](const Expr& a, const Parameter& b) { return a * Expr(b); })
        .def("__mul__", [](const Expr& a, double b) { return a * b; })
        .def("__rmul__", [](const Expr& a, const Parameter& b) { return Expr(b) * a; })
        .def("__rmul__", [](const Expr& a, double b) { return b * a; })
        .def("__truediv__", [](const Expr& a, const Expr& b) { return a / b; })
        .def("__truediv__", [](const Expr& a, const Parameter& b) { return a / Expr(b); })
        .def("__truediv__", [](const Expr& a, double b) { return a / b; })
        .def("__rtruediv__", [](const Expr& a, const Parameter& b) { return Expr(b) / a; })
        .def("__rtruediv__", [](const Expr& a, double b) { return b / a; })
        .def("__neg__", [](const Expr& a) { return -a; })
        // Copy support for deepcopy
        .def("__copy__", [](const Expr& self) {
            return Expr(self);
        })
        .def("__deepcopy__", [](const Expr& self, py::dict) {
            return Expr(self);
        }, py::arg("memo"))
        .def("__repr__", [](const Expr& e) {
            return "Expr(" + e.to_string() + ")";
        });


     parameter_class
       .def(py::init<double, bool>(),
             py::arg("value") = 0.0,
             py::arg("trainable") = false,
             "Create a new Parameter.\n\n"
             "Args:\n"
             "    value: Initial value (default: 0.0)\n"
             "    trainable: Whether this parameter should be trainable (default: False)")
        .def("get_uuid", &Parameter::get_uuid,
             "Get the unique identifier of this parameter.\n\n"
             "Returns:\n"
             "    str: UUID string")
        .def("get_value", &Parameter::get_value,
             "Get the current value of this parameter.\n\n"
             "Returns:\n"
             "    float: Current value")
        .def("set_value", &Parameter::set_value,
             py::arg("value"),
             "Set the value of this parameter.\n\n"
             "Args:\n"
             "    value: New value")
        // Add 'value' property for convenience (read-write)
        .def_property("value", &Parameter::get_value, &Parameter::set_value,
             "The current value of this parameter (read-write)")
        .def("is_trainable", &Parameter::is_trainable,
             "Check if this parameter is trainable.\n\n"
             "Returns:\n"
             "    bool: True if trainable")
        // Mathematical functions that return Expr
        .def("sin", [](const Parameter& self) { return sin(Expr(self)); },
             "Compute sine of this parameter.\n\n"
             "Returns:\n"
             "    Expr: Expression representing sin(parameter)")
        .def("cos", [](const Parameter& self) { return cos(Expr(self)); },
             "Compute cosine of this parameter.\n\n"
             "Returns:\n"
             "    Expr: Expression representing cos(parameter)")
        .def("__repr__", [](const Parameter& p) {
            return "Parameter(value=" + std::to_string(p.get_value()) +
                   ", trainable=" + (p.is_trainable() ? "True" : "False") +
                   ", uuid=" + p.get_uuid().substr(0, 8) + "...)";
        })
        // Arithmetic operators (promote to Expr)
        .def("__add__", [](const Parameter& self, const Parameter& other) { return Expr(self) + Expr(other); })
        .def("__add__", [](const Parameter& self, const Expr& other) { return Expr(self) + other; })
        .def("__add__", [](const Parameter& self, double other) { return Expr(self) + other; })
        .def("__radd__", [](const Parameter& self, double other) { return other + Expr(self); })
        .def("__sub__", [](const Parameter& self, const Parameter& other) { return Expr(self) - Expr(other); })
        .def("__sub__", [](const Parameter& self, const Expr& other) { return Expr(self) - other; })
        .def("__sub__", [](const Parameter& self, double other) { return Expr(self) - other; })
        .def("__rsub__", [](const Parameter& self, double other) { return other - Expr(self); })
        .def("__mul__", [](const Parameter& self, const Parameter& other) { return Expr(self) * Expr(other); })
        .def("__mul__", [](const Parameter& self, const Expr& other) { return Expr(self) * other; })
        .def("__mul__", [](const Parameter& self, double other) { return Expr(self) * other; })
        .def("__rmul__", [](const Parameter& self, double other) { return other * Expr(self); })
        .def("__truediv__", [](const Parameter& self, const Parameter& other) { return Expr(self) / Expr(other); })
        .def("__truediv__", [](const Parameter& self, const Expr& other) { return Expr(self) / other; })
        .def("__truediv__", [](const Parameter& self, double other) { return Expr(self) / other; })
        .def("__rtruediv__", [](const Parameter& self, double other) { return other / Expr(self); })
        .def("__neg__", [](const Parameter& self) { return -Expr(self); })
        // Copy support for deepcopy
        .def("__copy__", [](const Parameter& self) {
            return Parameter(self);
        })
        .def("__deepcopy__", [](const Parameter& self, py::dict) {
            return Parameter(self);
        }, py::arg("memo"));


    // Mathematical functions
    expr_module.def("sin", [](const Expr& e) { return sin(e); },
          py::arg("expr"),
          "Compute sine of an expression.\n\n"
          "Args:\n"
          "    expr: Input expression\n\n"
          "Returns:\n"
          "    Expr: sin(expr)");

    expr_module.def("cos", [](const Expr& e) { return cos(e); },
          py::arg("expr"),
          "Compute cosine of an expression.\n\n"
          "Args:\n"
          "    expr: Input expression\n\n"
          "Returns:\n"
          "    Expr: cos(expr)");

    expr_module.def("tan", [](const Expr& e) { return tan(e); },
          py::arg("expr"),
          "Compute tangent of an expression.\n\n"
          "Args:\n"
          "    expr: Input expression\n\n"
          "Returns:\n"
          "    Expr: tan(expr)");

    expr_module.def("exp", [](const Expr& e) { return exp(e); },
          py::arg("expr"),
          "Compute exponential of an expression.\n\n"
          "Args:\n"
          "    expr: Input expression\n\n"
          "Returns:\n"
          "    Expr: exp(expr)");

    expr_module.def("log", [](const Expr& e) { return log(e); },
          py::arg("expr"),
          "Compute natural logarithm of an expression.\n\n"
          "Args:\n"
          "    expr: Input expression\n\n"
          "Returns:\n"
          "    Expr: log(expr)");

    expr_module.def("sqrt", [](const Expr& e) { return sqrt(e); },
          py::arg("expr"),
          "Compute square root of an expression.\n\n"
          "Args:\n"
          "    expr: Input expression\n\n"
          "Returns:\n"
          "    Expr: sqrt(expr)");

    expr_module.def("pow", [](const Expr& base, const Expr& exp) { return pow(base, exp); },
          py::arg("base"),
          py::arg("exponent"),
          "Compute power of an expression.\n\n"
          "Args:\n"
          "    base: Base expression\n"
          "    exponent: Exponent expression\n\n"
          "Returns:\n"
          "    Expr: base^exponent");

    // Note: Implicit conversions for Expr are declared in binding.cpp main module
}
