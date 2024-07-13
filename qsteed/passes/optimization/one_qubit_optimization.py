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

from qsteed.dag.circuit_dag_convert import circuit_to_dag, dag_to_circuit
from qsteed.dag.circuit_dag_convert import node_to_gate, gate_to_node
from qsteed.dag.dagcircuit import DAGCircuit
from qsteed.passes.basepass import BasePass
from qsteed.passes.decomposition.ZYZ_decompose import zyz_decomposition, xyx_decomposition, zxz_decomposition
from quafu import QuantumCircuit
from quafu.elements.element_gates.rotation import RZGate, RYGate, RXGate


class OneQubitGateOptimization(BasePass):
    """
    Remove redundant gates
    """

    def run(self, circuit: QuantumCircuit) -> QuantumCircuit:
        """
        Transform the circuit into a DAG, remove redundant nodes, and transform the DAG back to a circuit.

        Args:
            circuit (QuantumCircuit): The input quantum circuit.

        Returns:
            QuantumCircuit: The optimized quantum circuit with redundant gates removed.
        """
        # new_circuit = circuit_to_dag(circuit, measure_flag=True)
        # new_circuit = self._remove_one_qubit_Gate(circuit=new_circuit)
        # new_circuit = dag_to_circuit(new_circuit, circuit.num)

        dag = circuit_to_dag(circuit, measure_flag=True)
        optimized_dag = self._remove_one_qubit_Gate(dag)
        optimized_circuit = dag_to_circuit(optimized_dag, circuit.num)
        return optimized_circuit

    def _sorted_node_list(self, circuit: DAGCircuit) -> dict:
        """
        Sort and return the nodes of the DAGCircuit.

        Args:
            circuit (DAGCircuit): The input DAGCircuit.

        Returns:
            dict: A sorted dictionary of nodes from the DAGCircuit.
        """
        node_list = circuit.nodes_dict()
        if 'm' in node_list:
            sorted_node = {k: node_list[k] for k in list(node_list.keys())[:-1]}
            sorted_node = {k: sorted_node[k] for k in sorted(sorted_node)}
        else:
            sorted_node = {k: node_list[k] for k in sorted(node_list)}
        return sorted_node

    # def _sorted_node_list(self, circuit: DAGCircuit) -> dict:
    #     """
    #     Sort and return the nodes of the DAGCircuit.
    #
    #     Args:
    #         circuit (DAGCircuit): The input DAGCircuit.
    #
    #     Returns:
    #         dict: A sorted dictionary of nodes from the DAGCircuit.
    #     """
    #     node_list = circuit.nodes_dict()
    #     sorted_keys = sorted(node_list.keys())
    #
    #     if 'm' in node_list:
    #         sorted_keys.remove('m')
    #         sorted_keys.append('m')
    #
    #     sorted_node = {k: node_list[k] for k in sorted_keys}
    #     return sorted_node

    '''
    This function is to combine continous one_qubit_gate
    '''

    def _remove_one_qubit_Gate(self, circuit: DAGCircuit) -> DAGCircuit:
        sorted_node = self._sorted_node_list(circuit)
        one_qubit_gate = ['H', 'RX', 'RZ', 'RY']
        gate_record = {}
        # loop all the nodes
        for item in sorted_node.items():
            node = item[1]
            # add one qubit gate
            if node.name in one_qubit_gate:
                successors = circuit.node_qubits_successors(node)
                # check current qubit has previous one-qubit gate
                pos = node.pos[0]
                gate = node_to_gate(copy.deepcopy(node))
                if pos in gate_record and gate_record[pos]:
                    # for each item in gate_record, the format is
                    # {key: [successors, num_combined_gate, combined_matrix, nodes]}
                    gate_record[pos][-1].append(node)
                    gate_record[pos] = [successors, gate_record[pos][1] + 1, gate.matrix @ gate_record[pos][2],
                                        gate_record[pos][-1]]
                else:
                    gate_record[pos] = [successors, 1, gate.matrix, [node]]
            else:
                # for this qubit, encounter multi-qubit gate, now just add the combined gate back to
                for pos in node.pos:
                    if pos in gate_record and gate_record[pos] and len(gate_record[pos][-1]) > 3:
                        # to find the minimum gate based on the angle, for RX, RY gate,
                        # when the theta is 0, it is the same as indentity matrix
                        self.gate_check(circuit=circuit, gate_record=gate_record, pos=pos)
                    gate_record[pos] = None
        # at the end of list, clean up record in case some gates are missing 
        for pos in gate_record.keys():
            if gate_record[pos] and len(gate_record[pos][-1]) > 3:
                self.gate_check(circuit=circuit, gate_record=gate_record, pos=pos)
            gate_record[pos] = []
        return circuit

    def gate_check(self, circuit, gate_record, pos, node=None):
        '''
            use zyz, zxz, xyx to decompose the gates and check which function will provide the minimum number of gates
            only RX, RY will be removed when the angle is 0. For zyz and zxz, if the middle angle is zero, we only have one gate.
        '''
        node = gate_record[pos][3][0]
        predecessors = circuit.node_qubits_predecessors(node)

        label = []
        for gate in gate_record[pos][-1]:
            label.append(gate.label)
            circuit.remove_instruction_node(gate)

        # gamma, beta, alpha, global_phase = xyx_decomposition(gate_record[pos][2])
        # if gamma * beta * alpha == 0:
        #     if gamma != 0:
        #         circuit.add_instruction_node(gate_to_node(RXGate(pos, gamma), label[0]),
        #                                      predecessors, gate_record[pos][0])
        #         label = label[1:]
        #     if beta != 0:
        #         circuit.add_instruction_node(gate_to_node(RYGate(pos, beta), label[0]),
        #                                      predecessors, gate_record[pos][0])
        #         label = label[1:]
        #     if alpha != 0:
        #         circuit.add_instruction_node(gate_to_node(RXGate(pos, alpha), label[0]),
        #                                      predecessors, gate_record[pos][0])
        #     return

        gamma, beta, alpha, global_phase = xyx_decomposition(gate_record[pos][2])
        if beta == 0:
            circuit.add_instruction_node(gate_to_node(RXGate(pos, gamma + alpha), label[0]),
                                         predecessors, gate_record[pos][0])
            return

        gamma, beta, alpha, global_phase = zxz_decomposition(gate_record[pos][2])
        if beta == 0:
            circuit.add_instruction_node(gate_to_node(RZGate(pos, gamma + alpha), label[0]),
                                         predecessors, gate_record[pos][0])
            return

        gamma, beta, alpha, global_phase = zyz_decomposition(gate_record[pos][2])
        if beta == 0:
            circuit.add_instruction_node(gate_to_node(RZGate(pos, gamma + alpha), label[0]),
                                         predecessors, gate_record[pos][0])
            return

        else:
            node1 = gate_to_node(RZGate(pos, gamma), label[0])
            circuit.add_instruction_node(node1, predecessors, gate_record[pos][0])
            node2 = gate_to_node(RYGate(pos, beta), label[1])
            circuit.add_instruction_node(node2, {pos: node1}, gate_record[pos][0])
            node3 = gate_to_node(RZGate(pos, alpha), label[2])
            circuit.add_instruction_node(node3, {pos: node2}, gate_record[pos][0])
        return
