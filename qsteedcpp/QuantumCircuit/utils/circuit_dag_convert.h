#pragma once

#include "QuantumCircuit/circuit/quantum_circuit.h"
#include "QuantumCircuit/DAG/dag.h"

namespace qsteedcpp {

/**
 * @brief Convert a QuantumCircuit to a DAGCircuit.
 *
 * This function converts a `qsteedcpp::QuantumCircuit` object into its Directed Acyclic Graph
 * (DAG) representation, `DAGCircuit`. Gates are converted to nodes, and qubit dependencies
 * are represented as edges. Measurements are collected and stored in the `DAGCircuit`.
 * This implementation is inspired by the Python version in `qsteed/dag/circuit_dag_convert.py`.
 *
 * @param circuit The input `QuantumCircuit` to convert.
 * @return A `DAGCircuit` representing the quantum circuit.
 */
DAGCircuit circuit_to_dag(const QuantumCircuit& circuit);

/**
 * @brief Convert a DAGCircuit back to a QuantumCircuit.
 *
 * This function reconstructs a `qsteedcpp::QuantumCircuit` from a `DAGCircuit`. It performs
 * a topological sort on the DAG nodes to determine the correct order of operations.
 * Gate information is extracted from `InstructionNode`s, and measurements are
 * reconstructed from the `measure` map in the `DAGCircuit`.
 *
 * @param dag The input `DAGCircuit` to convert.
 * @param num_qubits The desired number of qubits for the QuantumCircuit.
 *                   If the DAG contains gates on qubits with indices higher than num_qubits,
 *                   the resulting QuantumCircuit will be created with the maximum of num_qubits
 *                   and the highest qubit index + 1.
 * @return A `QuantumCircuit` reconstructed from the DAG.
 */
QuantumCircuit dag_to_circuit(const DAGCircuit& dag, int num_qubits);

} // namespace qsteedcpp
