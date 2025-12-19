# This code is part of QSteed.
#
# (C) Copyright 2024 Beijing Academy of Quantum Information Sciences
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import math

import networkx as nx
from qsteed.qsteedcpp import QuantumCircuit

from qsteed.compiler.qasm_parser import qreg_creg
from qsteed.graph.circuitgraph import circuit_to_graph, relabel_graph
from qsteed.graph.graphkernel import wl_subtree_kernel_cached


def average_fidelity(edges):
    """
    Calculate the average fidelity of given edges.

    Args:
        edges (list): A list of edges, where each edge is represented as a list or tuple
                      (node1, node2, fidelity).

    Returns:
        float: The average logarithmic fidelity of the edges.
    """
    if not edges:
        return 0

    total_log_fidelity = 0
    has_zero_fidelity = False

    for edge in edges:
        fidelity = edge[2]
        if fidelity <= 0:
            has_zero_fidelity = True
            fidelity = 1e-10
        total_log_fidelity += math.log(fidelity)

    # Calculate average
    ave_weight = total_log_fidelity / len(edges)

    return ave_weight


def similar_structure(circuit: str, vqpus: list):
    """
    Compare the similarity of the weighted graph of a quantum circuit and the VQPUs.

    Args:
        circuit (str): OpenQASM 2.0 circuit string.
        vqpus (list): List of VQPU objects, each with a coupling_list attribute.

    Returns:
        List: A list of chip substructures similar to the quantum circuit,
              sorted by similarity (the higher the similarity, the more similar).
    """

    qreg_name, creg_name, qubit_num, cbit_num = qreg_creg(circuit)
    qc = QuantumCircuit.from_openqasm(circuit)

    # Convert the quantum circuit to a graph representation
    g1 = circuit_to_graph(qc)

    # Compute the Weisfeiler-Lehman hash of the circuit graph (for isomorphism check)
    g1_hash = nx.weisfeiler_lehman_graph_hash(g1, iterations=3)

    similar_structure_list = []
    found_isomorphic = False

    for vqpu in vqpus:
        g2 = nx.Graph()
        for item in vqpu.coupling_list:
            g2.add_edges_from([(item[0], item[1])])

        g2 = relabel_graph(g2)
        g2_hash = nx.weisfeiler_lehman_graph_hash(g2, iterations=3)

        # Check if the two graphs are isomorphic based on their hashes (ignoring weights)
        if g1_hash == g2_hash:
            average_weight = average_fidelity(vqpu.coupling_list)
            similar_structure_list.append((vqpu, float('inf'), average_weight))
            found_isomorphic = True
            break
        elif not found_isomorphic:
            # If no isomorphic graph is found, calculate the similarity using the WL subtree kernel
            average_weight = average_fidelity(vqpu.coupling_list)
            kernel_value = wl_subtree_kernel_cached(g1, g2, iteration=3, consider_weight=False)
            similar_structure_list.append((vqpu, kernel_value, average_weight))

    # Sort the similar structures first by kernel similarity, and then by average fidelity (both in descending order)
    similar_structure_list = sorted(similar_structure_list, key=lambda x: (x[1], x[2]), reverse=True)

    return similar_structure_list
