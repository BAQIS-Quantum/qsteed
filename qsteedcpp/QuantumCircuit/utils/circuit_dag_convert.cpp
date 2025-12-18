#include "circuit_dag_convert.h"

#include <boost/graph/topological_sort.hpp>
#include <algorithm>
#include <stdexcept>

#include "QuantumCircuit/circuit/instruction_factory.h"

namespace qsteedcpp {

// Forward declaration
static CircuitInstruction node_to_instruction(const InstructionNode& node);
static InstructionNode instruction_to_node(const CircuitInstruction& inst);

DAGCircuit circuit_to_dag(const QuantumCircuit& circuit) {
    DAGCircuit dag;

    std::set<qubit_t> used_qubits;

    for(const auto& inst : circuit.get_instructions()) {
        if (inst.is_measurement()) {
            const auto& meas = std::get<Measurement>(inst.operation);
            for (size_t i = 0; i < meas.qubit_indices.size(); ++i) {
                dag.measure[meas.qubit_indices[i]] = meas.clbit_indices[i];
            }
        } else {
            InstructionNode node = instruction_to_node(inst);
            dag.add_instruction_node_end(node);

            for (qubit_t q : node.qubit_pos) {
                used_qubits.insert(q);
            }
        }
    }

    // For qubits that are only measured but not used by any gates,
    // add an edge from start to end to ensure the DAG knows about them
    for (const auto& [qubit, clbit] : dag.measure) {
        if (used_qubits.find(qubit) == used_qubits.end()) {
            dag.add_edge(dag.start_node_pos, dag.end_node_pos, qubit);
        }
    }

    return dag;
}

QuantumCircuit dag_to_circuit(const DAGCircuit& dag, int num_qubits_param) {
    auto qubits_used = dag.get_qubits_used();
    int inferred_num_qubits = 0;
    if (!qubits_used.empty()) {
        inferred_num_qubits = *std::max_element(qubits_used.begin(), qubits_used.end()) + 1;
    }

    // Use the maximum of the inferred qubits and the provided parameter
    int actual_num_qubits = std::max(inferred_num_qubits, num_qubits_param);
    QuantumCircuit circuit(actual_num_qubits);

    // Step 1: Add all gate nodes from the DAG to the circuit
    std::vector<DagGraph::vertex_descriptor> sorted_vertices;
    boost::topological_sort(dag.graph, std::back_inserter(sorted_vertices));

    for (auto vd : sorted_vertices) {
        const InstructionNode& node = dag.graph[vd];
        if (node.name == "start" || node.name == "end") {
            continue;
        }
        // Since the DAG no longer contains measure nodes, this check is simplified
        circuit.append(node_to_instruction(node));
    }

    // Step 2: Add measure instructions at the end from the 'measure' attribute
    if (!dag.measure.empty()) {
        // To ensure a consistent output order, we can sort the measurements.
        // Here we sort by classical bit index.
        std::vector<std::pair<qubit_t, cbit_t>> sorted_measures(dag.measure.begin(), dag.measure.end());
        std::sort(sorted_measures.begin(), sorted_measures.end(), 
                  [](const auto& a, const auto& b) {
                      return a.second < b.second;
                  });

        for (const auto& pair : sorted_measures) {
            circuit.measure(pair.first, pair.second);
        }
    }

    return circuit;
}


static InstructionNode instruction_to_node(const CircuitInstruction& inst) {
    std::string name = inst.name();
    std::vector<qubit_t> pos = inst.qubits;
    std::vector<Expr> paras;
    if (inst.is_gate()) {
        const auto& gate = std::get<std::unique_ptr<Gate>>(inst.operation);
        paras = gate->get_parameter_expressions();
    }
    
    std::optional<int> duration = inst.get_duration();
    std::optional<std::string> unit = inst.get_unit();

    return InstructionNode(name, pos, paras, duration, unit);
}

// Helper for converting an InstructionNode from a DAG back to a CircuitInstruction
static CircuitInstruction node_to_instruction(const InstructionNode& node) {
    const auto& name = node.name;
    const auto& qubits = node.qubit_pos;
    const auto& paras = node.paras;

    if (name == "barrier") return Barrier(qubits);
    if (name == "delay") return Delay(qubits[0], *node.duration, *node.unit);
    if (name == "xy" || name == "xyresonance") return XYResonance(qubits[0], qubits[1], *node.duration, *node.unit);
    if (name == "reset") return Reset(qubits[0]);

    if (name == "h") return H(qubits[0]);
    if (name == "x") return X(qubits[0]);
    if (name == "y") return Y(qubits[0]);
    if (name == "z") return Z(qubits[0]);
    if (name == "s") return S(qubits[0]);
    if (name == "sdg") return Sdg(qubits[0]);
    if (name == "t") return T(qubits[0]);
    if (name == "tdg") return Tdg(qubits[0]);
    if (name == "id") return Id(qubits[0]);
    if (name == "sx") return SX(qubits[0]);
    if (name == "sxdg") return SXdg(qubits[0]);
    if (name == "sy") return SY(qubits[0]);
    if (name == "sydg") return SYdg(qubits[0]);
    if (name == "w") return W(qubits[0]);
    if (name == "sw") return SW(qubits[0]);
    if (name == "swdg") return SWdg(qubits[0]);

    if (name == "rx") return RX(paras[0], qubits[0]);
    if (name == "ry") return RY(paras[0], qubits[0]);
    if (name == "rz") return RZ(paras[0], qubits[0]);
    if (name == "p" || name == "phase") return Phase(paras[0], qubits[0]);
    if (name == "u3") return U3(paras[0], paras[1], paras[2], qubits[0]);

    if (name == "cnot" || name == "cx") return CNOT(qubits[0], qubits[1]);
    if (name == "cz") return CZ(qubits[0], qubits[1]);
    if (name == "cy") return CY(qubits[0], qubits[1]);
    if (name == "cs") return CS(qubits[0], qubits[1]);
    if (name == "ct") return CT(qubits[0], qubits[1]);
    if (name == "swap") return Swap(qubits[0], qubits[1]);
    if (name == "iswap") return ISwap(qubits[0], qubits[1]);

    if (name == "rxx") return RXX(paras[0], qubits[0], qubits[1]);
    if (name == "ryy") return RYY(paras[0], qubits[0], qubits[1]);
    if (name == "rzz") return RZZ(paras[0], qubits[0], qubits[1]);
    if (name == "cp") return CP(paras[0], qubits[0], qubits[1]);
    if (name == "crx") return CRX(paras[0], qubits[0], qubits[1]);
    if (name == "cry") return CRY(paras[0], qubits[0], qubits[1]);
    if (name == "crz") return CRZ(paras[0], qubits[0], qubits[1]);

    if (name == "ccx" || name == "toffoli") return Toffoli(qubits[0], qubits[1], qubits[2]);
    if (name == "cswap" || name == "fredkin") return Fredkin(qubits[0], qubits[1], qubits[2]);

    throw std::runtime_error("Failed to convert node to instruction: gate '" + name + "' is not supported.");
}

} // namespace qsteedcpp
