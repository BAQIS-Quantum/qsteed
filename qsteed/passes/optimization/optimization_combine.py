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

from quafu import QuantumCircuit

from qsteed.dag.circuit_dag_convert import circuit_to_dag, dag_to_circuit
from qsteed.dag.dagcircuit import DAGCircuit
from qsteed.passes.basepass import BasePass


class GateCombineOptimization(BasePass):
    """
    Remove redundant gates
    """

    '''
    Here we only consider the basic gate since the previous step has transformed the gates into basic gates
    '''

    def run(self, circuit: QuantumCircuit) -> QuantumCircuit:
        """
        Transform the circuit into dag, remove redandunt node, then transform the dag back to circuit
        """
        new_circuit = circuit_to_dag(circuit, measure_flag=True)
        new_circuit = self._remove_pair_gate(circuit=new_circuit)
        new_circuit = self._combine_rotate_gate(circuit=new_circuit)
        new_circuit = dag_to_circuit(new_circuit, circuit.num)
        return new_circuit

    def _remove_pair_gate(self, circuit: DAGCircuit) -> DAGCircuit:
        """
        This function is to remove two adjacent gates since there are redunant.
        target gate: CX_Gate, H_Gate, CZ_Gate
        """
        sorted_node = self._sorted_node_list(circuit)
        gate_record = {}
        for item in sorted_node.items():
            node = item[1]
            previous_node = self._get_previous_node(node.pos, gate_record)
            if previous_node and previous_node.name in ['CX', 'H', 'CZ'] and node.name == previous_node.name:
                if (previous_node.name != 'CZ' and node.pos == previous_node.pos) or \
                        (previous_node.name == 'CZ' and set(node.pos) == set(previous_node.pos)):
                    # For CX and H doors, the positions must be exactly the same.
                    # For CZ door, the position set must be the same.
                    circuit.remove_instruction_node(previous_node)
                    circuit.remove_instruction_node(node)
                    for pos in node.pos:
                        gate_record[pos].pop()
                    continue

            # Update gate_record
            for pos in node.pos:
                gate_record.setdefault(pos, []).append(node)

        return circuit

    def _combine_rotate_gate(self, circuit: DAGCircuit) -> DAGCircuit:
        """
        This function is to combine two adjacent gate since they can be implemented in one gate
        target gate: RX_Gate, RY_Gate, RZ_Gate
        """
        sorted_node = self._sorted_node_list(circuit)
        previous_node = None
        target_gates = ['RX', 'RZ', 'RY']
        gate_record = {}
        for item in sorted_node.items():
            node = item[1]
            previous_node = self._get_previous_node(node.pos, gate_record)
            if previous_node and previous_node.name in target_gates and (
                    node.name == previous_node.name and node.pos == previous_node.pos):
                new_node = copy.deepcopy(previous_node)

                # new_node.paras = [sum(previous_node.paras + node.paras)] #  sum([int] + [parameter]) is not supported
                # fix:sum([int] + [parameter]) is not supported
                if len(previous_node.paras) == len(node.paras):
                    new_node.paras = [sum([i, j]) for i, j in zip(previous_node.paras, node.paras)]
                else:
                    raise ValueError(
                        'The number of parameters of the two gates is not the same.' + str(previous_node.paras) + str(
                            node.paras))
                # check the type of the new_node.paras
                # print('new_node.paras:', new_node.paras,list(map(type,new_node.paras)))

                predecessors = circuit.node_qubits_predecessors(previous_node)
                successors = circuit.node_qubits_successors(node)
                circuit.remove_instruction_node(previous_node)
                circuit.remove_instruction_node(node)
                circuit.add_instruction_node(new_node, predecessors, successors)
                for pos in node.pos:
                    gate_record[pos].pop()
                    gate_record[pos].append(new_node)
                continue

            # Update gate_record
            for pos in node.pos:
                gate_record.setdefault(pos, []).append(node)

        return circuit

    def _sorted_node_list(self, circuit: DAGCircuit) -> dict:
        node_list = circuit.nodes_dict()
        # print(node_list) # print the node_list
        # for i in list(node_list.keys()):
        #     print(i)
        if 'm' in node_list:
            # fix the first element to be 'm'  or other strange situation

            sorted_node = {k: node_list[k] for k in list(node_list.keys()) if k != 'm'}
            sorted_node = {k: sorted_node[k] for k in sorted(sorted_node)}
            sorted_node['m'] = node_list['m']
        else:
            sorted_node = {k: node_list[k] for k in sorted(node_list)}
        return sorted_node

    def _get_previous_node(self, positions, gate_record):
        """
        Get the previous node based on position, if there are multiple positions,
        make sure the previous node is the same at all positions.
        """
        previous_nodes = {gate_record[pos][-1] for pos in positions if pos in gate_record and gate_record[pos]}
        return previous_nodes.pop() if len(previous_nodes) == 1 else None
