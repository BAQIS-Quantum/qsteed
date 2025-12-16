#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include "Passes/base_pass.h"
#include "Passes/unroll/unroll_pass.h"
#include "Passes/unroll/rule_manager.h"
#include "Passes/unroll/decomposition_rules.h"

namespace py = pybind11;
using namespace qsteedcpp;

void bind_passes(py::module& m) {
    // Bind BasePass (abstract base class)
    py::class_<BasePass>(m, "BasePass")
        .def("run", &BasePass::run, py::arg("circuit"),
             "Run the pass on a quantum circuit")
        .def("name", &BasePass::name,
             "Get the name of the pass");
    
    // Bind UnrollPass
    py::class_<UnrollPass, BasePass>(m, "UnrollPass")
        .def(py::init<const std::set<std::string>&>(), 
             py::arg("basis_gates"),
             "Create an UnrollPass with specified basis gates")
        .def(py::init<const std::set<std::string>&, bool>(),
             py::arg("basis_gates"), py::arg("initialize_default_rules") = true,
             "Create an UnrollPass with specified basis gates")
        .def("try_run", &UnrollPass::try_run, py::arg("circuit"),
             "Try to run the pass and return True if all gates were successfully decomposed to basis gates")
        .def("get_basis_gates", &UnrollPass::get_basis_gates,
             "Get the list of basis gates",
             py::return_value_policy::reference_internal)
        .def("__repr__", [](const UnrollPass& pass) {
            std::string basis_str = "{";
            const auto& basis = pass.get_basis_gates();
            bool first = true;
            for (const auto& gate : basis) {
                if (!first) basis_str += ", ";
                basis_str += "'" + gate + "'";
                first = false;
            }
            basis_str += "}";
            return "<UnrollPass basis_gates=" + basis_str + ">";
        });
    
    // Bind DecompositionRule
    py::class_<RuleManager::DecompositionRule>(m, "DecompositionRule")
        .def(py::init<>())
        .def_readwrite("name", &RuleManager::DecompositionRule::name)
        .def_readwrite("target_gates", &RuleManager::DecompositionRule::target_gates)
        .def_readwrite("global_phase", &RuleManager::DecompositionRule::global_phase)
        .def_readwrite("priority", &RuleManager::DecompositionRule::priority)
        .def("__repr__", [](const RuleManager::DecompositionRule& rule) {
            std::string target_str = "{";
            bool first = true;
            for (const auto& gate : rule.target_gates) {
                if (!first) target_str += ", ";
                target_str += "'" + gate + "'";
                first = false;
            }
            target_str += "}";
            return "<DecompositionRule name='" + rule.name + 
                   "' targets=" + target_str + 
                   " priority=" + std::to_string(rule.priority) + ">";
        });
    
    // Bind RuleManager
    py::class_<RuleManager, std::shared_ptr<RuleManager>>(m, "RuleManager")
        .def(py::init<>())
        .def("register_rule", 
             static_cast<void (RuleManager::*)(const std::string&, const RuleManager::DecompositionRule&)>(&RuleManager::register_rule),
             py::arg("gate_name"), py::arg("rule"),
             "Register a decomposition rule for a gate")
        .def("register_rule", 
             static_cast<void (RuleManager::*)(const std::vector<std::string>&, const RuleManager::DecompositionRule&)>(&RuleManager::register_rule),
             py::arg("gate_names"), py::arg("rule"),
             "Register a decomposition rule for multiple gate names (aliases)")
        .def("get_rules", &RuleManager::get_rules,
             py::arg("gate_name"),
             "Get all rules registered for a gate")
        .def("has_rule", &RuleManager::has_rule,
             py::arg("gate_name"),
             "Check if there are any rules for a gate")
        .def("clear", &RuleManager::clear,
             "Clear all registered rules");
    
    // Helper function to create a RuleManager with standard rules
    m.def("create_standard_rule_manager", []() {
        auto manager = std::make_shared<RuleManager>();
        initialize_standard_rules(*manager);
        return manager;
    }, "Create a RuleManager with standard decomposition rules");
    
    // Common basis gate sets (as Python sets)
    m.attr("IBM_BASIS_GATES") = py::set(py::cast(std::set<std::string>{"u1", "u2", "u3", "cx"}));
    m.attr("RIGETTI_BASIS_GATES") = py::set(py::cast(std::set<std::string>{"rx", "rz", "cz"}));
    m.attr("DEFAULT_BASIS_GATES") = py::set(py::cast(std::set<std::string>{"rx", "ry", "rz", "cx", "h"}));
    
    // Example usage in docstring
    m.doc() = R"pbdoc(
        Quantum circuit transformation passes
        
        The UnrollPass recursively decomposes quantum gates until all gates 
        in the circuit are from the specified basis gate set.
        
        Example usage:
        
        >>> import qsteedcpp
        >>> circuit = qsteedcpp.QuantumCircuit(2)
        >>> circuit.h(0)
        >>> circuit.cnot(0, 1)
        >>> circuit.swap(0, 1)
        >>> 
        >>> # Create an UnrollPass with basis gates
        >>> basis_gates = {'rx', 'rz', 'cx'}  # Use set instead of list
        >>> unroll_pass = qsteedcpp.UnrollPass(basis_gates)
        >>> 
        >>> # Run the pass on the circuit
        >>> # The pass will recursively decompose gates like H and SWAP
        >>> # until everything is expressed in terms of RX, RZ, and CX
        >>> unroll_pass.run(circuit)
        >>> # Note: run() will NOT modify the circuit if any gate cannot be 
        >>> # decomposed to basis gates (all-or-nothing behavior)
        >>> 
        >>> # Use try_run for partial decomposition
        >>> # This will decompose what it can and keep non-decomposable gates
        >>> if unroll_pass.try_run(circuit):
        ...     print("All gates successfully decomposed to basis gates")
        ... else:
        ...     print("Some gates could not be decomposed but were kept")
        
        Note: 
        - run(): All-or-nothing - only modifies circuit if ALL gates can be decomposed
        - try_run(): Best-effort - decomposes what it can, keeps the rest
        
        Context-aware rule selection:
        The UnrollPass now intelligently selects decomposition rules based on your
        basis gates. For example, if 'cz' is in your basis_gates, it will prefer
        rules that decompose directly to CZ instead of going through CX first.
        
        >>> # Example: Context-aware decomposition
        >>> # With CZ in basis
        >>> basis_with_cz = {'cz', 'h', 'rx', 'ry', 'rz'}
        >>> pass_cz = qsteedcpp.UnrollPass(basis_with_cz)
        >>> # CNOT will be decomposed as: H(target) · CZ · H(target)
        >>> 
        >>> # Without CZ in basis
        >>> basis_no_cz = {'cx', 'rx', 'ry', 'rz'}
        >>> pass_cx = qsteedcpp.UnrollPass(basis_no_cz)
        >>> # CZ will be decomposed as: H(target) · CNOT · H(target)
    )pbdoc";
}