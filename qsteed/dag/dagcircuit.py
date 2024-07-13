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

from typing import Dict

import networkx as nx
from networkx.classes.multidigraph import MultiDiGraph

from qsteed.dag.instruction_node import InstructionNode


class DAGCircuit(MultiDiGraph):
    """
    A directed acyclic graph (DAG) representation of a quantum circuit.

    Inherits from MultiDiGraph, which is a directed graph class that can store multiedges.
    Each node in the DAGCircuit represents a quantum operation, and each edge represents a
    dependency between operations. The DAGCircuit is used to optimize the circuit by
    identifying and merging common subcircuits.
    """

    def __init__(self, qubits_used=None, cbits_used=None, incoming_graph_data=None, **attr):
        """
        Create a DAGCircuit.

        Args:
            qubits_used (set[int]): A set of qubits used in the circuit. Defaults to None.
            cbits_used (set[int]): A set of classical bits used in the circuit. Defaults to None.
            circuit_qubits (int): the number of qubits in the circuit. Defaults to None.
            incoming_graph_data: Data to initialize graph. If None (default) an empty graph is created.
            **attr: Keyword arguments passed to the MultiDiGraph constructor.

        Attributes:
            qubits_used (set[int]): A set of qubits used in the circuit.
            cbits_used (set[int]): A set of classical bits used in the circuit.
            num_instruction_nodes (int): The number of instruction nodes in the circuit.
        """
        super().__init__(incoming_graph_data, **attr)

        if qubits_used is None:
            self.qubits_used = set()
        elif isinstance(qubits_used, set):
            self.qubits_used = qubits_used
        else:
            raise ValueError('qubits_used should be a set or None')

        if cbits_used is None:
            self.cbits_used = set()
        elif isinstance(cbits_used, set):
            self.cbits_used = cbits_used
        else:
            raise ValueError('cbits_used should be a set or None')

        self.circuit_qubits = None
        self.num_instruction_nodes = 0

    # Add new methods or override existing methods here.

    def update_circuit_qubits(self, circuit_qubits: int) -> int:
        """
        Update the number of qubits in the quantum circuit.

        Args:
            circuit_qubits (int): The number of qubits in the DAGCircuit.

        Returns:
            int: The updated number of qubits in the DAGCircuit.
        """
        self.circuit_qubits = circuit_qubits
        return self.circuit_qubits

    def update_qubits_used(self) -> set:
        """
        Update and return the set of qubits used in the DAGCircuit.

        The qubits used are determined based on the labels of the edges connected to node -1.
        The qubits are the integer part of the label.

        Returns:
            qubits_used (set): The set of qubits used in the DAGCircuit.
        """
        if -1 not in self.nodes:
            raise ValueError('-1 should be in DAGCircuit, please add it first')

        self.qubits_used = set([int(edge[2]['label'][1:]) for edge in self.out_edges(-1, data=True)])
        return self.qubits_used

    def update_cbits_used(self) -> set:
        """
        Update and return the set of classical bits (cbits) used in the DAGCircuit.

        The cbits used are determined based on the positions of measure nodes in the DAGCircuit.

        Returns:
            cbits_used (set): The set of cbits used in the DAGCircuit.
        """
        for node in self.nodes:
            if hasattr(node, 'name') and node.name == 'measure':
                self.cbits_used = set(node.pos.values())
        return self.cbits_used

    def update_num_instruction_nodes(self) -> int:
        """
        Update and return the number of instruction nodes in the DAGCircuit.

        The number of instruction nodes is calculated as the total number of nodes
        minus 2 (excluding the special nodes -1 and float('inf')).

        Returns:
            num_instruction_nodes (int): The number of instruction nodes in the DAGCircuit.
        """
        if -1 not in self.nodes:
            raise ValueError('-1 should be in DAGCircuit, please add it first')
        if float('inf') not in self.nodes:
            raise ValueError('float("inf") should be in DAGCircuit, please add it first')
        self.num_instruction_nodes = len(self.nodes) - 2

        return self.num_instruction_nodes

    def nodes_labels_resorted(self):
        """
        Update and resort the labels of the nodes in the DAGCircuit.

        This is only for convenience and does not affect the DAGCircuit itself.

        Returns:
            new_dag (DAGCircuit): A new DAGCircuit with nodes' labels resorted.
        """
        # Create a new DAGCircuit
        new_dag = DAGCircuit()

        # Get the sorted list of nodes
        sorted_nodes_list = self.nodes_list()

        # Iterate through the sorted nodes and update labels
        i = 0
        for node in sorted_nodes_list:
            if node.name != 'measure':
                node.label = i
                new_dag.add_instruction_node_end(node)
                i += 1
            else:
                new_dag.add_instruction_node_end(node)

        return new_dag

    def nodes_dict(self) -> dict:
        """
        Return a dictionary of nodes with the node label as key and the node as value,
        excluding nodes with labels -1 and float('inf').

        Returns:
            dict: A dictionary where keys are node labels and values are nodes.
        """
        nodes_dict = {}
        for node in nx.topological_sort(self):
            if node != -1 and node != float('inf'):
                nodes_dict[node.label] = node
        return nodes_dict

    def nodes_list(self) -> list:
        """
        Return a list of nodes without  -1 and float('inf').

        Returns:
            nodes_list (list): A list of nodes excluding -1 and float('inf').
        """
        nodes_list = []
        for node in nx.topological_sort(self):
            if node != -1 and node != float('inf'):
                nodes_list.append(node)
        return nodes_list

    def node_qubits_predecessors(self, node: InstructionNode) -> dict:
        """
        Return a dictionary of qubits to their predecessor nodes for the given node.

        Args:
            node (InstructionNode): A node in the DAGCircuit. The node should not be -1.

        Returns:
            node_qubits_predecessors (dict): A dictionary where keys are qubits and values are predecessor nodes.
        """
        if node not in self.nodes:
            raise ValueError('Node should be in DAGCircuit')
        if node == -1:
            raise ValueError('-1 has no predecessors')

        node_qubits_predecessors = {
            int(edge[2]['label'][1:]): edge[0]
            for edge in self.in_edges(node, data=True)
        }

        return node_qubits_predecessors

    def node_qubits_successors(self, node: InstructionNode) -> dict:
        """
        Return a dictionary of qubits to their successor nodes for the given node.

        Args:
            node (InstructionNode): A node in the DAGCircuit. The node should not be float('inf').

        Returns:
            node_qubits_successors (dict): A dictionary where keys are qubits and values are successor nodes.
        """
        if node not in self.nodes:
            raise ValueError('Node should be in DAGCircuit')
        if node == float('inf'):
            raise ValueError('float("inf") has no successors')

        node_qubits_successors = {
            int(edge[2]['label'][1:]): edge[1]
            for edge in self.out_edges(node, data=True)
        }

        return node_qubits_successors

    def node_qubits_inedges(self, node: InstructionNode) -> dict:
        """
        Return a dictionary of qubits to their incoming edges for the given node.

        Args:
            node (InstructionNode): A node in the DAGCircuit. The node should not be -1.

        Returns:
            dict: A dictionary where keys are qubits and values are incoming edges.
        """
        if node not in self.nodes:
            raise ValueError('Node should be in DAGCircuit')
        if node == -1:
            raise ValueError('-1 has no predecessors')

        inedges = [edge for edge in self.in_edges(node, data=True, keys=True)]
        qubits_labels = [int(edge[2]['label'][1:]) for edge in self.in_edges(node, data=True)]
        node_qubits_inedges = dict(zip(qubits_labels, inedges))
        return node_qubits_inedges

    def node_qubits_outedges(self, node: InstructionNode) -> dict:
        """
        Return a dictionary of qubits to their outgoing edges for the given node.

        Args:
            node (InstructionNode): A node in the DAGCircuit. The node should not be float('inf').

        Returns:
            dict: A dictionary where keys are qubits and values are outgoing edges.
        """
        if node not in self.nodes:
            raise ValueError('Node should be in DAGCircuit')
        if node == float('inf'):
            raise ValueError('float("inf") has no successors')

        outedges = [edge for edge in self.out_edges(node, data=True, keys=True)]
        qubits_labels = [int(edge[2]['label'][1:]) for edge in self.out_edges(node, data=True)]
        node_qubits_outedges = dict(zip(qubits_labels, outedges))
        return node_qubits_outedges

    def remove_instruction_node(self, gate: InstructionNode) -> None:
        """
        Remove a gate from the DAGCircuit and all edges connected to it.
        Add new edges for qubits of the removed gate between all predecessors and successors of the removed gate.

        Args:
            gate (InstructionNode): The gate to be removed from the DAGCircuit. The gate should be in the DAGCircuit,
                                    and should not be -1 or float('inf').

        Raises:
            ValueError: If the gate is not in the DAGCircuit or is -1 or float('inf').
        """
        if gate not in self.nodes:
            raise ValueError('Gate should be in DAGCircuit')
        if gate in {-1, float('inf')}:
            raise ValueError('Gate should not be -1 or float("inf")')

        qubits_predecessors = self.node_qubits_predecessors(gate)
        qubits_successors = self.node_qubits_successors(gate)

        for qubit in gate.pos:
            pred = qubits_predecessors[qubit]
            succ = qubits_successors[qubit]
            if pred != -1 and succ != float('inf'):
                self.add_edge(pred, succ, label=f'q{qubit}')
            elif pred == -1 and succ != float('inf'):
                self.add_edge(pred, succ, label=f'q{qubit}', color='green')
            else:
                self.add_edge(pred, succ, label=f'q{qubit}', color='red')

        self.remove_node(gate)
        self.update_qubits_used()

    def merge_dag(self, other_dag: 'DAGCircuit') -> 'DAGCircuit':
        """
        Merge another DAGCircuit into this DAGCircuit.

        Args:
            other_dag (DAGCircuit): The DAGCircuit to merge into this one.

        Returns:
            DAGCircuit: The merged DAGCircuit.
        """
        # TODO: For two large lines, the error 'networkx.exception.NetworkXUnfeasible:
        #  Graph contains a cycle or graph changed during iteration' is reported.
        #  The labels inside need to be recalibrated to avoid the same nodes.
        if not isinstance(other_dag, DAGCircuit):
            raise ValueError('other_dag should be a DAGCircuit')
        if other_dag is None:
            return self
        if self is None:
            return other_dag

        # For the same qubits (intersection), remove the outgoing edges from the final node of the original DAG
        # and the incoming edges from the initial node of the other DAG,
        # then connect the corresponding tail and head nodes by adding edges
        other_dag_qubits_used = other_dag.update_qubits_used()
        self_qubits_used = self.update_qubits_used()

        # Find intersection of qubits used in both DAGs
        intersect_qubits = self_qubits_used & other_dag_qubits_used
        end_edges_labels_1 = self.node_qubits_inedges(float('inf'))
        start_edges_labels_2 = other_dag.node_qubits_outedges(-1)

        # TODO: For dag with measurement operations, additional processing is required
        if intersect_qubits:
            for qubit in intersect_qubits:
                self.remove_edges_from([end_edges_labels_1[qubit]])
                other_dag.remove_edges_from([start_edges_labels_2[qubit]])
                self.add_edge(end_edges_labels_1[qubit][0], start_edges_labels_2[qubit][1], label=f'q{qubit}')

        # Add nodes and edges from the other DAG to this DAG
        self.add_nodes_from(other_dag.nodes(data=True))
        self.add_edges_from(other_dag.edges(data=True))

        # remove the edges between -1 and float('inf')
        # self.remove_edges_from([edge for edge in self.edges(keys=True) if edge[0] == -1 and edge[1] == float('inf')])

        # Update qubits used in the merged DAG
        self.update_qubits_used()

    def add_instruction_node(self, gate: InstructionNode, predecessors_dict: Dict[int, InstructionNode],
                             successors_dict: Dict[int, InstructionNode]) -> None:
        """
        Add an instruction node into the DAGCircuit, along with all edges connected to it.
        Add new edges for qubits of the new gate between all predecessors and successors of the new gate.

        Args:
            gate (InstructionNode): The instruction node to add. The gate should not be -1 or float('inf').
            predecessors_dict (Dict[int, InstructionNode]): A dictionary of {qubits -> predecessors} of the gate.
            successors_dict (Dict[int, InstructionNode]): A dictionary of {qubits -> successors} of the gate.

        Raises:
            ValueError: If the gate is -1 or float('inf').
        """
        if gate in {-1, float('inf')}:
            raise ValueError('Gate should not be -1 or float("inf")')

        # Remove the edges between the predecessors and successors for the qubits used by the added node
        qubits_pre_out_edges = []
        qubits_suc_in_edges = []
        self.update_qubits_used()

        for qubit in gate.pos:
            if qubit in self.qubits_used:
                pre_out_edges = self.node_qubits_outedges(predecessors_dict[qubit])
                qubits_pre_out_edges.append(pre_out_edges[qubit][:3])  # Use [:3] to get the key of the edge: u, v, k

                suc_in_edges = self.node_qubits_inedges(successors_dict[qubit])
                qubits_suc_in_edges.append(suc_in_edges[qubit][:3])

        qubits_removed_edges = set(qubits_pre_out_edges) | set(qubits_suc_in_edges)
        self.remove_edges_from(qubits_removed_edges)

        # Add the new node and edges
        self.add_node(gate, color="blue")
        for qubit in gate.pos:
            pred = predecessors_dict[qubit]
            succ = successors_dict[qubit]

            if pred == -1:
                self.add_edge(pred, gate, label=f'q{qubit}', color='green')
            else:
                self.add_edge(pred, gate, label=f'q{qubit}')

            if succ == float('inf'):
                self.add_edge(gate, succ, label=f'q{qubit}', color='red')
            else:
                self.add_edge(gate, succ, label=f'q{qubit}')

        # Update qubits
        self.update_qubits_used()

    def add_instruction_node_end(self, gate: InstructionNode):
        """
        Add an instruction node at the end of the DAGCircuit, before the float('inf') node.

        Args:
            gate (InstructionNode): The instruction node to add. The gate should not be -1 or float('inf').

        Raises:
            ValueError: If the gate is -1 or float('inf'), or if the DAGCircuit does not have both -1 and float('inf') nodes.
        """
        if gate in {-1, float('inf')}:
            raise ValueError('Gate should not be -1 or float("inf")')

        if -1 not in self.nodes or float('inf') not in self.nodes:
            # If DAGCircuit is empty, add -1 and float('inf') first
            self.add_nodes_from([(-1, {"color": "green"}), (float('inf'), {"color": "red"})])

            # Add the new node and edges
            self.add_node(gate, color="blue")
            for qubit in gate.pos:
                self.add_edge(-1, gate, label=f'q{qubit}', color='green')
                self.add_edge(gate, float('inf'), label=f'q{qubit}', color='red')

        elif -1 in self.nodes and float('inf') in self.nodes:
            # Get the predecessors_dict of the new node
            inf_predecessors_dict = self.node_qubits_predecessors(float('inf'))
            gate_predecessors_dict = {qubit: inf_predecessors_dict.get(qubit, -1) for qubit in gate.pos}
            gate_successors_dict = {qubit: float('inf') for qubit in gate.pos}

            # Add the new node and edges using add_instruction_node method
            self.add_instruction_node(gate, gate_predecessors_dict, gate_successors_dict)
        else:
            raise ValueError('DAGCircuit should have both -1 and float("inf") nodes at the same time')

    def substitute_node_with_dag(self, gate: InstructionNode, input_dag):
        """
        Substitute a node in the DAGCircuit with another DAGCircuit.

        Args:
            gate (InstructionNode): The node to be substituted.
            input_dag (DAGCircuit): The DAGCircuit to substitute the node with.

        Raises:
            ValueError: If the node is not in the DAGCircuit, is -1 or float('inf'),
                        input_dag is not a DAGCircuit, or the qubits set of input_dag
                        is not the same as the gate's qubits.
        """
        if gate not in self.nodes:
            raise ValueError('node should be in DAGCircuit')
        if gate in {-1, float('inf')}:
            raise ValueError('node should not be -1 or float("inf")')
        if not isinstance(input_dag, DAGCircuit):
            raise ValueError('input_dag should be a DAGCircuit')
        if input_dag is None:
            self.remove_instruction_node(gate)
            return
        if self is None:
            return input_dag

        # Ensure the qubits set of input_dag is the same as the gate‘s qubits
        if input_dag.update_qubits_used() != set(gate.pos):
            raise ValueError('Qubits set of input_dag should be the same as the gate‘s qubits')

        # Find all predecessors and successors of the node to be replaced
        predecessors_dict = self.node_qubits_predecessors(gate)
        successors_dict = self.node_qubits_successors(gate)

        # Remove the node and its edges from the dag graph
        self.remove_node(gate)

        # Add the input_dag into self
        input_dag_startnodes = input_dag.node_qubits_successors(-1)
        input_dag_endnodes = input_dag.node_qubits_predecessors(float('inf'))

        input_dag_qubits = input_dag.update_qubits_used()

        input_dag.remove_node(-1)
        input_dag.remove_node(float('inf'))

        # Add edges between the nodes in input_dag and the predecessors and successors of the original node
        for qubit in input_dag_qubits:
            pred = predecessors_dict[qubit]
            self.add_edge(
                pred, input_dag_startnodes[qubit], label=f'q{qubit}', color='green' if pred == -1 else 'black')
            succ = successors_dict[qubit]
            self.add_edge(
                input_dag_endnodes[qubit], succ, label=f'q{qubit}', color='red' if succ == float('inf') else 'black')

        # Add nodes and edges from input_dag to self
        self.add_nodes_from(input_dag.nodes(data=True))
        self.add_edges_from(input_dag.edges(data=True))

        # Update other attributes of the DAG
        self.update_qubits_used()

    def is_dag(self) -> bool:
        """
        Check if the DAGCircuit is a DAG.

        Returns:
            bool: True if the DAGCircuit is a DAG, False otherwise.
        """
        return nx.is_directed_acyclic_graph(self)

    def get_measure_nodes(self) -> list:
        """
        Get all 'measure' nodes in the DAGCircuit.

        Returns:
            list: A list of 'measure' nodes in the DAGCircuit.
        """
        return [node for node in self.nodes if node not in {-1, float('inf')} and node.name == 'measure']

    def remove_measure_nodes(self, only_last_measure: bool = False) -> None:
        """
        Remove measure nodes in the DAGCircuit.

        Args:
            only_last_measure (bool): If False, remove all measure nodes in the DAGCircuit;
                                      If True, remove the last measure node in the DAGCircuit.
        """
        measure_nodes = self.get_measure_nodes()
        if only_last_measure:
            if measure_nodes:
                self.remove_instruction_node(measure_nodes[-1])
        else:
            for node in measure_nodes:
                self.remove_instruction_node(node)
