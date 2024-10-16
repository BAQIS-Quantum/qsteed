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

import copy

import networkx as nx
from quafu import QuantumCircuit

from qsteed.graph.circuitgraph import circuit_to_graph, relabel_graph
from qsteed.graph.graphkernel import wl_subtree_kernel, fast_subtree_kernel, wl_oa_kernel
from qsteed.compiler.qasm_parser import qreg_creg


def similar_structure(circuit: str, vqpus: list):
    """ Compare the similarity of weighted graph of circuit and substructure of quantum chip.

    Args:
        circuit(str): OpenQASM 2.0
        vqpus(list): [VQPU,]
    Returns:
        most_similar_struct(list): Chip substructures similar to quantum circuits,
                                    sorted according to the value of the kernel,
                                    the higher the kernel value, the more similar.
    """
    qreg_name, creg_name, qubit_num, cbit_num = qreg_creg(circuit)
    qc = QuantumCircuit(qubit_num, cbit_num)
    qc.from_openqasm(circuit)
    qc.draw_circuit()
    g1 = circuit_to_graph(qc)
    similar_structure_list = []
    kernel_value_list = []
    for vqpu in vqpus:
        g2 = nx.Graph()
        g1_copy = copy.deepcopy(g1)
        for item in vqpu.coupling_list:
            g2.add_edges_from([(item[0], item[1], {'weight': item[2]})])
        g2 = relabel_graph(g2)

        # W-L subtree kernel iteration
        kernel_value = wl_subtree_kernel(g1_copy, g2, iteration=10)
        if round(kernel_value, 1) not in kernel_value_list:
            kernel_value_list.append(round(kernel_value, 1))
            similar_structure_list.append((vqpu, kernel_value))

        # # Weisfeiler-Lehman Optimal Assignment (WL-OA) Kernel iteration
        # kernel_value = wl_oa_kernel(g1_copy, g2, iteration=10)
        # if round(kernel_value, 1) not in kernel_value_list:
        #     kernel_value_list.append(round(kernel_value, 1))
        #     similar_structure_list.append((vqpu, kernel_value))

        # # fast subtree kernel iteration
        # g2 = nx.convert_node_labels_to_integers(g2)
        # kernel_value = fast_subtree_kernel(g1_copy, g2, iteration=10)
        # if round(kernel_value, 1) not in kernel_value_list:
        #     kernel_value_list.append(round(kernel_value, 1))
        #     similar_structure_list.append((vqpu, kernel_value))

        g1_copy.clear()
        g2.clear()

    similar_structure_list = sorted(similar_structure_list, key=lambda x: x[1], reverse=True)
    return similar_structure_list
