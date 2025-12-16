#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
#include <pybind11/complex.h>
#include <pybind11/numpy.h>
#include "gates/base_gate.h"


namespace py = pybind11;
using namespace qsteedcpp;

void bind_gates(py::module& m) {
    // Base Gate class (for type system - users should not construct directly)
    auto gate_class = py::class_<Gate>(m, "Gate")
        .def("name", &Gate::name)
        .def("get_qubit_count", &Gate::get_qubit_count)
        .def("has_parameters", &Gate::has_parameters)
        .def("parameter_count", &Gate::parameter_count)
        .def("__repr__", [](const Gate& g) {
            return std::string(g.name()) + "Gate(" + std::to_string(g.get_qubit_count()) + " qubits)";
        });

}

    // m.attr("Instruction") = gate_class;
    // =============================================================================
    // Gate classes (for type checking only - no constructors exposed)
    // Users should use factory functions like H(0), X(1), CNOT(0,1) instead
    // =============================================================================

    // Single-qubit non-parametric gates
//     py::class_<HGate, Gate>(m, "HGate")
//         .def_property_readonly_static("name", [](py::object) { return "h"; });

//     py::class_<XGate, Gate>(m, "XGate")
//         .def_property_readonly_static("name", [](py::object) { return "x"; });

//     py::class_<YGate, Gate>(m, "YGate")
//         .def_property_readonly_static("name", [](py::object) { return "y"; });

//     py::class_<ZGate, Gate>(m, "ZGate")
//         .def_property_readonly_static("name", [](py::object) { return "z"; });

//     py::class_<SGate, Gate>(m, "SGate")
//         .def_property_readonly_static("name", [](py::object) { return "s"; });

//     py::class_<SdgGate, Gate>(m, "SdgGate")
//         .def_property_readonly_static("name", [](py::object) { return "sdg"; });

//     py::class_<TGate, Gate>(m, "TGate")
//         .def_property_readonly_static("name", [](py::object) { return "t"; });

//     py::class_<TdgGate, Gate>(m, "TdgGate")
//         .def_property_readonly_static("name", [](py::object) { return "tdg"; });

//     py::class_<IdGate, Gate>(m, "IdGate")
//         .def_property_readonly_static("name", [](py::object) { return "id"; });

//     py::class_<SXGate, Gate>(m, "SXGate")
//         .def_property_readonly_static("name", [](py::object) { return "sx"; });

//     py::class_<SXdgGate, Gate>(m, "SXdgGate")
//         .def_property_readonly_static("name", [](py::object) { return "sxdg"; });

//     py::class_<SYGate, Gate>(m, "SYGate")
//         .def_property_readonly_static("name", [](py::object) { return "sy"; });

//     py::class_<SYdgGate, Gate>(m, "SYdgGate")
//         .def_property_readonly_static("name", [](py::object) { return "sydg"; });

//     py::class_<WGate, Gate>(m, "WGate")
//         .def_property_readonly_static("name", [](py::object) { return "w"; });

//     py::class_<SWGate, Gate>(m, "SWGate")
//         .def_property_readonly_static("name", [](py::object) { return "sw"; });

//     py::class_<SWdgGate, Gate>(m, "SWdgGate")
//         .def_property_readonly_static("name", [](py::object) { return "swdg"; });

//     // Single-qubit parametric gates
//     py::class_<RXGate, Gate>(m, "RXGate")
//         .def_property_readonly_static("name", [](py::object) { return "rx"; });

//     py::class_<RYGate, Gate>(m, "RYGate")
//         .def_property_readonly_static("name", [](py::object) { return "ry"; });

//     py::class_<RZGate, Gate>(m, "RZGate")
//         .def_property_readonly_static("name", [](py::object) { return "rz"; });

//     py::class_<PhaseGate, Gate>(m, "PhaseGate")
//         .def_property_readonly_static("name", [](py::object) { return "p"; });

//     py::class_<U3Gate, Gate>(m, "U3Gate")
//         .def_property_readonly_static("name", [](py::object) { return "u3"; });

//     // Two-qubit non-parametric gates
//     py::class_<CNOTGate, Gate>(m, "CNOTGate")
//         .def_property_readonly_static("name", [](py::object) { return "cnot"; });

//     py::class_<CZGate, Gate>(m, "CZGate")
//         .def_property_readonly_static("name", [](py::object) { return "cz"; });

//     py::class_<SwapGate, Gate>(m, "SwapGate")
//         .def_property_readonly_static("name", [](py::object) { return "swap"; });

//     py::class_<iSwapGate, Gate>(m, "iSwapGate")
//         .def_property_readonly_static("name", [](py::object) { return "iswap"; });

//     py::class_<CYGate, Gate>(m, "CYGate")
//         .def_property_readonly_static("name", [](py::object) { return "cy"; });

//     py::class_<CSGate, Gate>(m, "CSGate")
//         .def_property_readonly_static("name", [](py::object) { return "cs"; });

//     py::class_<CTGate, Gate>(m, "CTGate")
//         .def_property_readonly_static("name", [](py::object) { return "ct"; });

//     // Two-qubit parametric gates
//     py::class_<RXXGate, Gate>(m, "RXXGate")
//         .def_property_readonly_static("name", [](py::object) { return "rxx"; });

//     py::class_<RYYGate, Gate>(m, "RYYGate")
//         .def_property_readonly_static("name", [](py::object) { return "ryy"; });

//     py::class_<RZZGate, Gate>(m, "RZZGate")
//         .def_property_readonly_static("name", [](py::object) { return "rzz"; });

//     py::class_<CPGate, Gate>(m, "CPGate")
//         .def_property_readonly_static("name", [](py::object) { return "cp"; });

//     py::class_<CRXGate, Gate>(m, "CRXGate")
//         .def_property_readonly_static("name", [](py::object) { return "crx"; });

//     py::class_<CRYGate, Gate>(m, "CRYGate")
//         .def_property_readonly_static("name", [](py::object) { return "cry"; });

//     py::class_<CRZGate, Gate>(m, "CRZGate")
//         .def_property_readonly_static("name", [](py::object) { return "crz"; });

//     // Three-qubit gates
//     py::class_<ToffoliGate, Gate>(m, "ToffoliGate")
//         .def_property_readonly_static("name", [](py::object) { return "ccx"; });

//     py::class_<FredkinGate, Gate>(m, "FredkinGate")
//         .def_property_readonly_static("name", [](py::object) { return "cswap"; });

//     // Multi-controlled gates
//     py::class_<MCXGate, Gate>(m, "MCXGate")
//         .def_property_readonly_static("name", [](py::object) { return "mcx"; })
//         .def("num_controls", &MCXGate::num_controls);

//     py::class_<MCYGate, Gate>(m, "MCYGate")
//         .def_property_readonly_static("name", [](py::object) { return "mcy"; })
//         .def("num_controls", &MCYGate::num_controls);

//     py::class_<MCZGate, Gate>(m, "MCZGate")
//         .def_property_readonly_static("name", [](py::object) { return "mcz"; })
//         .def("num_controls", &MCZGate::num_controls);

//     // Multi-controlled rotation gates
//     py::class_<MCRXGate, Gate>(m, "MCRXGate")
//         .def_property_readonly_static("name", [](py::object) { return "mcrx"; })
//         .def("num_controls", &MCRXGate::num_controls);

//     py::class_<MCRYGate, Gate>(m, "MCRYGate")
//         .def_property_readonly_static("name", [](py::object) { return "mcry"; })
//         .def("num_controls", &MCRYGate::num_controls);

//     py::class_<MCRZGate, Gate>(m, "MCRZGate")
//         .def_property_readonly_static("name", [](py::object) { return "mcrz"; })
//         .def("num_controls", &MCRZGate::num_controls);

//     // Controlled-U gate
//     py::class_<ControlledUGate, Gate>(m, "ControlledUGate")
//         .def_property_readonly_static("name", [](py::object) { return "controlledu"; })
//         .def("num_controls", &ControlledUGate::num_controls);
// }