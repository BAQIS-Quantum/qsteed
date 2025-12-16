#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "sabre/model/backend.h"
#include "sabre/model/model.h"
#include "sabre/model/coupling.h"
#include "sabre/model/layout.h"
#include "sabre/sabre_core.h"
#include "sabre/sabre_layout.h"
#include "sabre/sabre_routing.h"
#include "DAG/dag.h"
#include "compiler.h"
#include "QuantumCircuit/expression/expr.h"
#include "QuantumCircuit/expression/nodes/parameter.h"

namespace py = pybind11;
using namespace sabre;
using namespace qsteedcpp;


void bind_quantum_circuit(py::module& m);
void bind_gates(py::module& m);
void bind_passes(py::module& m);
void bind_expressions(py::module& m);

#ifdef BUILD_TORCH_BACKEND
void bind_torch_backend(py::module& m);
#endif


PYBIND11_MODULE(qsteedcpp, m) {
    m.doc() = "QSteed C++ Extensions";

    bind_expressions(m);

    // Enable implicit conversions after expression types are bound
    py::implicitly_convertible<double, qsteedcpp::Expr>();
    py::implicitly_convertible<int, qsteedcpp::Expr>();
    py::implicitly_convertible<float, qsteedcpp::Expr>();
    py::implicitly_convertible<qsteedcpp::Parameter, qsteedcpp::Expr>();

    // Bind circuit and instruction FIRST, so CircuitInstruction is a known type
    bind_quantum_circuit(m);

    // Bind gates SECOND, as they will now be factories that return CircuitInstruction
    bind_gates(m);

    bind_passes(m);

#ifdef BUILD_TORCH_BACKEND
    bind_torch_backend(m);
#endif

    py::enum_<Heuristic>(m, "Heuristic")
        .value("FIDELITY", Heuristic::FIDELITY)
        .value("DISTANCE", Heuristic::DISTANCE)
        .value("MIXTURE", Heuristic::MIXTURE);

    py::class_<SabreLayout>(m, "SabreLayout")
        .def(py::init<const CouplingCircuit&>())
        .def(py::init<const CouplingCircuit&, int, const std::string&>())
        .def(py::init<const CouplingCircuit&, int, const std::string&, LayoutStructure>())
        .def("run", &SabreLayout::run)
        .def("get_model", &SabreLayout::get_model)
        .def("get_add_swap_count", &SabreLayout::get_add_swap_count)
        .def_readwrite("c_circuit", &SabreLayout::c_circuit)
        .def_readwrite("max_iterations", &SabreLayout::max_iterations)
        .def_readwrite("heuristic", &SabreLayout::heuristic);

     py::class_<SabreRouting>(m, "SabreRouting")
        .def(py::init<const CouplingCircuit&>())
        .def(py::init<const CouplingCircuit&, Heuristic>())
        .def("set_model", &SabreRouting::set_model)
        .def("get_model", &SabreRouting::get_model)
        .def("get_add_swap_count", &SabreRouting::get_add_swap_count)
        .def("run", &SabreRouting::run)
        .def_readwrite("modify_dag", &SabreRouting::modify_dag);

    py::class_<Backend>(m, "Backend")
        .def(py::init<CouplingList>())
        .def(py::init<>());

    py::class_<Model>(m, "Model")
        .def(py::init<>())
        // .def(py::init<Backend>())
        .def_readwrite("initial_layout", &Model::initial_layout)
        .def_readwrite("final_layout", &Model::final_layout);


    py::class_<CouplingCircuit>(m, "CouplingCircuit")
        .def(py::init<CouplingList>())
        .def("update_num_qubits", &CouplingCircuit::update_num_qubits)
        .def("get_distance_matrix", &CouplingCircuit::get_distance_matrix)
        .def("get_fidelity_dict", &CouplingCircuit::get_fidelity_dict)
        .def("get_fidelity", &CouplingCircuit::get_fidelity)
        .def("get_mini_path", &CouplingCircuit::get_mini_path)
        .def("print", &CouplingCircuit::print)
        .def_readwrite("num_qubits", &CouplingCircuit::num_qubits);

    py::class_<Layout>(m, "Layout")
        .def(py::init<>())
        .def(py::init<LayoutStructure>())
        .def("get_v2p", &Layout::get_v2p)
        .def("__getitem__", &Layout::operator[]);

    py::class_<InstructionNode>(m, "InstructionNode")
        .def(py::init<>())
        .def(py::init<const std::string>())
        .def(py::init<const std::string, const std::vector<int>>())
        .def(py::init<const std::string, const int>())
        // New constructor using Expr
        .def(py::init<const std::string, const std::vector<int>, const std::vector<Expr>, int, const std::string>())
        .def_readwrite("name", &InstructionNode::name)
        .def_readwrite("qubit_pos", &InstructionNode::qubit_pos)
        .def_readwrite("paras", &InstructionNode::paras)
        .def_readwrite("duration", &InstructionNode::duration)
        .def_readwrite("unit", &InstructionNode::unit);

    py::class_<MeasureNode, InstructionNode>(m, "MeasureNode")
        .def(py::init<>())
        .def(py::init<std::vector<int>, std::vector<int>>())
        .def_readwrite("classic_pos", &MeasureNode::classic_pos);

    py::class_<EdgeProperties>(m, "EdgeProperties")
        .def(py::init<>())
        .def(py::init<int>())
        .def_readwrite("id", &EdgeProperties::qubit_id);

    py::class_<DagGraph>(m, "DagGraph")
        .def("__getitem__", [](const DagGraph &g, node_pos_t i) {
            if (i<0 || i>= boost::num_vertices(g))
                throw py::index_error("Out of index range!");
            else
                return g[i];
        });


    py::class_<DAGCircuit>(m, "DAGCircuit")
        .def(py::init<>())
        .def("add_node", &DAGCircuit::add_node)
        .def("add_instruction_node_end", &DAGCircuit::add_instruction_node_end)
        .def("add_edge", py::overload_cast<const int, const int, const qubit_t>(&DAGCircuit::add_edge))
        .def("add_edge", py::overload_cast<const int, const int, const EdgeProperties&>(&DAGCircuit::add_edge))
        .def("get_num_nodes", &DAGCircuit::get_num_nodes)
        .def("get_qubits_used", &DAGCircuit::get_qubits_used)
        .def("num_qubits", &DAGCircuit::num_qubits)
        .def("vertices", [](DAGCircuit &s) {return py::make_iterator(s.vertex_begin(), s.vertex_end());})
        .def("reverse", &DAGCircuit::reverse)
        .def_readwrite("graph", &DAGCircuit::graph)
        .def_readwrite("measure", &DAGCircuit::measure);

    py::class_<qarser::QasmCompiler>(m, "QasmCompiler")
        .def(py::init<const std::string&, bool>())
        .def("parse", &qarser::QasmCompiler::parse)
        .def("analyze", &qarser::QasmCompiler::analyze)
        .def("convert_to_dag", &qarser::QasmCompiler::convert_to_dag)
        .def("load_from_file", &qarser::QasmCompiler::load_from_file)
        .def("print_source", &qarser::QasmCompiler::print_source)
        .def("get_dag", &qarser::QasmCompiler::get_dag);


    m.def("qasm_to_dag", &qarser::qasm_to_dag);

#ifdef WITH_GRAPHVIZ
    m.def("hello", []() { return "Hello, Graphviz!"; });
#endif

}