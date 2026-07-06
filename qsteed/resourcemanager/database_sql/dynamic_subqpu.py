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

from qsteed.resourcemanager.build_library import BuildLibrary
# import matplotlib.pyplot as plt
from qsteed.resourcemanager.database_sql.sql_models import StdQPU


class DynamicSubQPU:
    """

    """

    def __init__(self, stdqpu: StdQPU = None, used_qubits: list = None):
        qpu = stdqpu.std2qpu
        self.backend = stdqpu.qpu_name
        self.int_to_qubit = qpu.int_to_qubit
        self.qubit_to_int = qpu.qubit_to_int
        self.node_to_qubit = stdqpu.node_to_qubit
        self.qubit_to_node = stdqpu.qubit_to_node
        self.stdqpu = stdqpu
        self.standard_graph = stdqpu.standard_graph
        self.used_qubits = used_qubits
        self.structure = stdqpu.structure

    def update_graph(self, crosstalk_constraint=1):
        used_nodes = []
        for q in self.used_qubits:
            used_nodes.append(self.qubit_to_node[self.int_to_qubit[q]])

        standard_graph = copy.deepcopy(self.standard_graph)
        # pos = {(i, j): (j, -i) for i, j in standard_graph.nodes()}

        # Create a virtual node and connect the nodes in the center region to the virtual node.
        virtual_node = tuple(used_nodes)
        standard_graph.add_node(virtual_node)
        for node in used_nodes:
            standard_graph.add_edge(virtual_node, node)

        # Get the neighborhood of the central node (ego graph).
        ego_graph = nx.ego_graph(standard_graph, virtual_node, radius=crosstalk_constraint)

        # Get all neighbor nodes of the central node and its neighborhood.
        neighbors_to_remove = set(neighbor for neighbor in standard_graph.neighbors(virtual_node))
        for node in ego_graph.nodes():
            neighbors_to_remove.update(standard_graph.neighbors(node))

        # Delete the central node and all neighbor nodes in its neighborhood.
        standard_graph.remove_nodes_from(neighbors_to_remove)
        self.standard_graph = standard_graph
        return standard_graph

    def update_structure(self):
        available_qubits = []
        for node in self.standard_graph.nodes(data=True):
            if node[1]['label'] == 'available':
                available_qubits.append(self.qubit_to_int[self.node_to_qubit[node[0]]])

        new_structure = []
        for edge in self.structure:
            if edge[0] in available_qubits and edge[1] in available_qubits:
                new_structure.append(edge)

        self.structure = new_structure

        return new_structure

    def substructure_partitioning(self, qubits_num):
        if self.used_qubits:
            self.update_graph()
            self.update_structure()
        build_lib = BuildLibrary(backend=self.backend)
        connected_subgraphs = build_lib.connected_substructure(self.structure)
        substructure = build_lib.substructure(self.structure, connected_subgraphs, qubits_num)
        return substructure

    def parallel_selector(self, tasks_qubits_num):
        substructure_list = []
        for qubits_num in tasks_qubits_num:
            substructure = self.substructure_partitioning(qubits_num)[0][1]
            self.used_qubits = sorted({bit for item in substructure for bit in item[:2]})
            substructure_list.append(substructure)
        return substructure_list
