#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <torch/extension.h>

#include "torch_backend.hpp"

namespace py = pybind11;

// Torch backend bindings - integrated into qsteedcpp module
void bind_torch_backend(py::module& m) {
    // CRITICAL: Import torch to register pybind11 type converters for torch::Tensor
    // Without this, returning torch::Tensor from C++ will cause segfault!
    // We try to import here, but if it fails (e.g., during stub generation),
    // we'll delay the import to TorchBackend constructor.
    try {
        py::module_::import("torch");
    } catch (py::error_already_set& e) {
        // torch not available during build/stub generation - that's OK
        // Will be imported on first TorchBackend instantiation
    }

    // Create a submodule for torch backend
    py::module torch_module = m.def_submodule("torch_backend", "Quantum Circuit Tensor Network Simulator with Autograd Support");

    // TorchBackend class
    py::class_<qsteedcpp::TorchBackend>(torch_module, "TorchBackend",
        "Torch-based quantum circuit backend with automatic differentiation support.\n\n"
        "Features:\n"
        "- Automatically manages parameters as torch.Tensors\n"
        "- Supports autograd for gradient-based optimization\n"
        "- Simple interface: compile once, run multiple times\n\n"
        "Example:\n"
        "    backend = qsteedcpp.torch_backend.TorchBackend()\n"
        "    backend.compile(qc)\n"
        "    result = backend.run()\n"
        "    loss = result.abs().pow(2).sum()\n"
        "    loss.backward()\n"
        "    # Access gradients via backend.parameters()")
        .def(py::init([]() {
            // Ensure torch is imported when creating TorchBackend instance
            // (in case it wasn't imported during module initialization)
            py::module_::import("torch");
            return new qsteedcpp::TorchBackend();
        }), "Create a new TorchBackend instance")

        .def("compile", &qsteedcpp::TorchBackend::compile, py::arg("circuit"),
             "Compile a quantum circuit.\n\n"
             "Analyzes the circuit, extracts parameters, and creates\n"
             "trainable torch.Tensors for each parameter.\n\n"
             "Args:\n"
             "    circuit: A qsteedcpp.QuantumCircuit to compile")

        .def("run", &qsteedcpp::TorchBackend::run,
             "Run the compiled circuit.\n\n"
             "Uses internally managed parameters to build and execute\n"
             "the tensor network. The result is connected to PyTorch's\n"
             "autograd graph, enabling gradient computation.\n\n"
             "Returns:\n"
             "    torch.Tensor: The output state vector")

        .def("parameters", &qsteedcpp::TorchBackend::parameters,
             "Get all trainable parameters.\n\n"
             "Returns a list of torch.Tensors that can be passed to\n"
             "PyTorch optimizers.\n\n"
             "Returns:\n"
             "    List[torch.Tensor]: All parameters")

        .def("parameter", &qsteedcpp::TorchBackend::parameter, py::arg("index"),
             "Get a single parameter by index.\n\n"
             "Args:\n"
             "    index: Parameter index\n\n"
             "Returns:\n"
             "    torch.Tensor: The parameter")

        .def("get_parameter", &qsteedcpp::TorchBackend::get_parameter, py::arg("param"),
             "Get a parameter by Parameter object.\n\n"
             "This is a user-friendly interface that avoids manual index lookup.\n\n"
             "Args:\n"
             "    param: A Parameter object used in the circuit\n\n"
             "Returns:\n"
             "    torch.Tensor: The corresponding parameter tensor\n\n"
             "Example:\n"
             "    theta = Parameter(0.5, trainable=True)\n"
             "    qc.rx(theta, 0)\n"
             "    backend.compile(qc)\n"
             "    theta_tensor = backend.get_parameter(theta)")

        .def("num_parameters", &qsteedcpp::TorchBackend::num_parameters,
             "Get the number of parameters.\n\n"
             "Returns:\n"
             "    int: Number of parameters")

        .def("set_parameter", &qsteedcpp::TorchBackend::set_parameter,
             py::arg("index"), py::arg("value"),
             "Set a parameter value.\n\n"
             "Args:\n"
             "    index: Parameter index\n"
             "    value: New torch.Tensor value")

        .def("is_compiled", &qsteedcpp::TorchBackend::is_compiled,
             "Check if a circuit has been compiled.\n\n"
             "Returns:\n"
             "    bool: True if compiled")

        .def("num_qubits", &qsteedcpp::TorchBackend::num_qubits,
             "Get the number of qubits in the compiled circuit.\n\n"
             "Returns:\n"
             "    int: Number of qubits")

        .def("reset", &qsteedcpp::TorchBackend::reset,
             "Reset the backend to initial state.\n\n"
             "Clears the compiled circuit and all parameters.");
}
