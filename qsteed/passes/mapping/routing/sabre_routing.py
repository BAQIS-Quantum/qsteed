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
from collections import defaultdict
from functools import lru_cache
from random import choice
from typing import Union, List

import numpy as np
from quafu import QuantumCircuit
from quafu.elements import Barrier, XYResonance, Measure
from quafu.elements.element_gates import SwapGate

from qsteed.dag.circuit_dag_convert import circuit_to_dag, gate_to_node, copy_dag
from qsteed.dag.dagcircuit import DAGCircuit
from qsteed.graph.couplinggraph import CouplingGraph
from qsteed.passes.basepass import BasePass
from qsteed.passes.datadict import DataDict
from qsteed.passes.mapping.baselayout import Layout


class SabreRouting(BasePass):
    """Execute the SABER routing algorithm once.

    References:
        Gushu Li, Yufei Ding, and Yuan Xie. "Tackling the qubit mapping problem
        for NISQ-era quantum devices." ASPLOS ’19, 1001–1014 (ACM, 2019).
        URL https://doi.org/10.1145/3297858.3304023
    """

    def __init__(self,
                 coupling_list: List = None,
                 heuristic="distance",
                 initial_layout: Layout = None,
                 modify_dag=False,
                 decay_delta: float = 0.01,
                 decay_reset_interval: int = 5,
                 extended_set_size: int = 20,
                 extended_set_weight: float = 0.5,
                 ):
        """SabreRouting initializer.

        Args:
            coupling_list (list): qubit coupling structure, e.g. [(0,1,0.99), (1,0,0.99), (1,2,0.98),(2,1,0.95)]
            heuristic (str): 'distance': The original SABER algorithm is based on the swap gates distance;
                             'fidelity': Noise-aware SABER algorithm based on two-qubit gates fidelity;
                             'mixture': First based on the 'distance', when the distance cannot be determined,
                                        based on the two-qubit gate 'fidelity'.
            initial_layout (Layout): The initial layout of the SABER routing algorithm.
            modify_dag (bool): The default is False, that is, the SWAP gate is not actually added to the
                                original dag, and the routing algorithm only changes the layout.
            decay_delta (float): The decay factor for penalizing serial swaps.
            decay_reset_interval (int): The number of swaps to apply before resetting the decay factor.
            extended_set_size (int): The size of the extended set.
            extended_set_weight (float): Weight of extended set compared to front_layer.
        """

        super().__init__()

        self.coupling_list = coupling_list
        self.coupling_graph = None
        self.initial_layout = initial_layout
        self.heuristic = heuristic
        self.modify_dag = modify_dag
        self.distance_matrix = None
        self.path_fidelity = None
        self.model = None
        self.add_swap_count = 0

        self.decay_delta = decay_delta
        self.decay_reset_interval = decay_reset_interval
        self.extended_set_size = extended_set_size
        self.extended_set_weight = extended_set_weight
        self.qubits_decay = None
        self.pre_executed_counts = None

        self.nodes_label = []

    def set_model(self, model):
        """Set the model, including information such as backend and layout.

        Args:
            model (Model): The given model includes information such as backend and layout.
        """
        self.model = model
        self.coupling_graph = self.model.get_backend().get_property('coupling_graph')

        if self.coupling_graph is None:
            if self.coupling_list is not None:
                coupling_graph = CouplingGraph(self.coupling_list)
                if coupling_graph.is_bidirectional is False:
                    coupling_graph.do_bidirectional()
                self.coupling_graph = coupling_graph
            else:
                raise ValueError("Error: There is no qubits coupling structure.")

        if self.model.datadict is None:
            self.model.datadict = DataDict()

    def get_model(self):
        """Get the model, including information such as backend and layout.
        """
        return self.model

    def run(self, circ_dag: Union[QuantumCircuit, DAGCircuit]):
        """Run the SabreRouting pass on QuantumCircuit or DAGCircuit.

        Args:
            circ_dag (DAGCircuit or QuantumCircuit): a dag or circuit to perform routing algorithm
        Returns:
            DAGCircuit: The original dag or the mapped_dag with added swap gate.
        """
        if isinstance(circ_dag, QuantumCircuit):
            dag = circuit_to_dag(circ_dag)
        elif isinstance(circ_dag, DAGCircuit):
            # dag = copy.deepcopy(circ_dag)
            dag = copy_dag(circ_dag)
        else:
            raise TypeError('Error: SabreSwap pass only supports QuantumCircuit or DAGCircuit.')

        if len(dag.qubits_used) == 1:  # Single-qubit circuit do not need to perform sabre
            return dag

        if float('inf') in dag.nodes:
            dag.remove_node(float('inf'))

        if len(dag.qubits_used) > self.coupling_graph.num_qubits:
            raise ValueError("More virtual qubits than physical qubits.")

        # In order to select SWAPs that can be executed in parallel,
        # a decay effect is introduced in the heuristic cost function.
        # If a qubit q_i is involved in a SWAP recently,
        # then its decay parameter will increase by δ, that is, decay(q_i) = 1 + δ.
        # TODO: dag.qubits_used is the actual qubit used. If the original circuit has an idle qubit line,
        #  this qubit will be lost, but it needs to be retained.
        self.qubits_decay = {qubit: 1 for qubit in dag.qubits_used}

        # Size of lookahead window. Set to number of qubits
        self.extended_set_size = self.coupling_graph.num_qubits

        # Decay coefficient for penalizing serial swaps. Set to average fidelity
        if self.heuristic == 'fidelity':
            self.decay_delta = 1 - sum(item for item in self.coupling_graph.edge_dict.values()) / len(
                self.coupling_graph.edge_dict)

        # How often to reset all decay rates to 1.
        self.decay_reset_interval = round(self.extended_set_size / 2)

        self.add_swap_count = 0  # Add number of SWAP gates.

        self.distance_matrix = self.coupling_graph.distance_matrix
        self.path_fidelity = self.coupling_graph.path_fidelity

        # Initialize mapped_dag, retaining only the name and qubit number information of the input original DAG.
        mapped_dag = None
        if self.modify_dag:
            mapped_dag = DAGCircuit()
            mapped_dag.name = dag.name
            mapped_dag.update_circuit_qubits(dag.circuit_qubits)

        if self.initial_layout is not None:
            self.model.set_layout({'initial_layout': self.initial_layout})
        elif self.model.get_layout()["initial_layout"] is None:
            layout = Layout()
            # Method1: Choose a trivial initial_layout.
            # layout.generate_trivial_layout(virtual_qubits=dag.circuit_qubits)
            # Method2: Choose a random initial_layout.
            layout.generate_random_layout(len(dag.qubits_used), self.coupling_graph.num_qubits)
            self.model.set_layout({'initial_layout': layout})

        # Copy initial_layout. Initialize Layout first and then copy 'initial_layout' through
        # set_layout() function, which is faster than deepcopy.
        # current_layout = copy.deepcopy(self.model.get_layout()["initial_layout"])
        current_layout = Layout()
        current_layout.set_layout(self.model.get_layout()["initial_layout"])

        # Parameter preparation before iteration.
        self.pre_executed_counts = defaultdict(int)  # executed counts
        front_layer = []
        for edge in dag.out_edges():
            if edge[0] == -1:
                self.pre_executed_counts[edge[1]] += 1
                if edge[1].name not in (Barrier.name, Measure.name):
                    if self.pre_executed_counts[edge[1]] == 2 or len(edge[1].pos) == 1:
                        front_layer.append(edge[1])
            else:
                break

        executed_2gate_list = []  # The hardware execution order list of executable 2-qubit gates under current_layout.
        unavailable_2qubits = set()

        # Start the algorithm from the front layer and iterate until all gates are completed.
        iteration_count = 0
        while front_layer:
            execute_gate_list = []

            # Delete executable gate
            for node in front_layer:
                if len(node.pos) == 2 and node.name not in [Barrier.name, XYResonance.name, Measure.name]:
                    v0, v1 = node.pos
                    p0, p1 = current_layout.v2p[v0], current_layout.v2p[v1]
                    if self.coupling_graph.graph.has_edge(p0, p1):
                        execute_gate_list.append(node)
                        # Record the coupling 2-qubits that have been used in the circuit.
                        unavailable_2qubits_new = {item for item in unavailable_2qubits if
                                                   p0 not in item and p1 not in item}
                        unavailable_2qubits = unavailable_2qubits_new
                else:  # Single-qubit gates, barriers, XY-gates and measures are both executable gates.
                    execute_gate_list.append(node)

            if execute_gate_list:
                for node in execute_gate_list:
                    self._apply_gate(mapped_dag, node, current_layout)
                    front_layer.remove(node)
                    for successor in self._dag_successors(dag, node):
                        if isinstance(successor.pos, dict):
                            self.pre_executed_counts[successor] += len(node.pos)
                            # self.pre_executed_counts[successor] += 1
                        else:
                            self.pre_executed_counts[successor] += 1
                        if self.pre_executed_counts[successor] == len(successor.pos):
                            # If all predecessors of a node in dag are applied, add this node to the front_layer.
                            front_layer.append(successor)

                # Reset qubits decay factors when a logical gate is applied.
                iteration_count = 0
                self._reset_qubits_decay()

                continue

            # When all available gates are exhausted, heuristically find the best swap and insert it.
            extended_set = self._calc_extended_set(dag, front_layer)
            swap_candidates = self._obtain_swaps(front_layer, current_layout)
            best_swap = self._get_best_swap(swap_candidates, current_layout, front_layer, extended_set,
                                            unavailable_2qubits)
            swap_node = gate_to_node(SwapGate(best_swap[0], best_swap[1]),
                                     specific_label='add' + str(self.add_swap_count))
            self._apply_gate(mapped_dag, swap_node, current_layout)
            self.add_swap_count += 1
            current_layout.swap(best_swap[0], best_swap[1])

            # Update the unavailable_2qubits set to prevent the same qubits from being inserted
            # into swap continuously, because two consecutive swaps on the same qubits can be eliminated.
            executed_2gate_list.append(
                (tuple(sorted((current_layout.v2p[best_swap[0]], current_layout.v2p[best_swap[1]]))), 1))
            unavailable_2qubits.add(
                tuple(sorted((current_layout.v2p[best_swap[0]], current_layout.v2p[best_swap[1]]))))

            # Update qubits_decay
            iteration_count += 1
            if iteration_count % self.decay_reset_interval == 0:
                self._reset_qubits_decay()
            else:
                self.qubits_decay[best_swap[0]] += self.decay_delta
                self.qubits_decay[best_swap[1]] += self.decay_delta

        # Update model data
        self.model.get_layout()["final_layout"] = current_layout
        self.model.datadict['add_swap_count'] = self.add_swap_count

        if self.modify_dag:
            return mapped_dag
        return dag

    def _apply_gate(self, mapped_dag, node, current_layout):
        """Add a node (gate) to dag.

        Args:
            mapped_dag (DAGCircuit): a real-time updated dag.
            node (dag.instruction_node.InstructionNode): The gate that needs to be inserted into the mapped_dag.
            current_layout (Layout): current layout
        """
        if not self.modify_dag:
            return

        # # Remove duplicate nodes
        # if node.label in self.nodes_label:
        #     return
        # else:
        #     self.nodes_label.append(node.label)

        mapped_op_node = copy.copy(node)
        if isinstance(node.pos, dict):
            mapped_qargs = {}
            for qubit, cbit in node.pos.items():
                mapped_qargs[current_layout.v2p[qubit]] = cbit
        else:
            mapped_qargs = [current_layout.v2p[pos] for pos in node.pos]
        mapped_op_node.pos = mapped_qargs
        mapped_dag.add_instruction_node_end(mapped_op_node)

    def _reset_qubits_decay(self):
        """Reset all qubit decay factors to 1.
        """
        self.qubits_decay = {k: 1 for k in self.qubits_decay.keys()}

    @lru_cache(maxsize=128)
    def _dag_successors(self, dag, node):
        """Find the successors of a node in the dag.

        Args:
            dag (DAGCircuit): a dag
            node: A node in the dag.
        Returns:
            successors: All successors of a node
        """
        successors = []
        for _, successor in dag.out_edges(node):
            successors.append(successor)
        return successors

    def _calc_extended_set(self, dag, front_layer):
        """Calculate the extended set for lookahead capabilities.

        Args:
            dag (DAGCircuit): a dag
            front_layer (list): The front layer in the dag.
        Returns:
            extended_set (set): Set of expansion gates obtained according to requirements.
        """
        extended_set = set()
        new_front_layer = list(front_layer)
        while len(new_front_layer) > 0 and len(extended_set) < self.extended_set_size:
            node = new_front_layer.pop(0)
            successors_nodes = self._dag_successors(dag, node)
            successors_nodes = [sn for sn in successors_nodes if sn.name not in [Barrier.name, Measure.name]]
            new_front_layer.extend(successors_nodes)
            new_front_layer = list(set(new_front_layer))
            for successor in successors_nodes:
                if len(successor.pos) == 2 and isinstance(successor.pos, list):
                    extended_set.update([successor])
        return extended_set

    def _obtain_swaps(self, front_layer, current_layout):
        """Get the set of candidate swap gates.

        Args:
            front_layer (list): front_layer gates list
            current_layout (Layout): current layout
        Returns:
            candidate_swaps (set): Set of candidate swap gates
        """
        candidate_swaps = set()
        for node in front_layer:
            for virtual in node.pos:
                physical = current_layout.v2p[virtual]
                for neighbor in self.coupling_graph.neighbors(physical):
                    virtual_neighbor = current_layout.p2v[neighbor]
                    swap = sorted([virtual, virtual_neighbor])
                    candidate_swaps.add(tuple(swap))

        return candidate_swaps

    def _swap_score(self, physical_swap_qubits):
        """Obtain the cost of the swap gate for the heuristic for 'fidelity'.

        Args:
            physical_swap_qubits (tuple): The physical qubits of the swap gate.
        Returns:
            swap_cost (float): The swap_cost calculated based on the coupled two-qubit fidelity.
        """
        min_f = min(np.log(self.coupling_graph.edge_dict[(physical_swap_qubits[0], physical_swap_qubits[1])]), np.log(
            self.coupling_graph.edge_dict[(physical_swap_qubits[1], physical_swap_qubits[0])]))
        max_f = max(np.log(self.coupling_graph.edge_dict[(physical_swap_qubits[0], physical_swap_qubits[1])]), np.log(
            self.coupling_graph.edge_dict[(physical_swap_qubits[1], physical_swap_qubits[0])]))
        swap_cost = 2 * max_f + min_f
        return swap_cost

    def _get_best_swap(self, swap_candidates, current_layout, front_layer, extended_set, unavailable_2qubits):
        """Get the best swap based on different heuristics.

        Args:
            swap_candidates (set): The set of all candidate swap gates.
            current_layout (Layout): current layout
            front_layer (list): front layer gates list
            extended_set: set of expansion gates
            unavailable_2qubits (set): set of unavailable two-qubits
        Returns:
            best_swap (tuple): the best swap based on different heuristics
        """
        swap_scores = dict.fromkeys(swap_candidates, -100000)
        if self.heuristic == 'fidelity':
            for swap_qubits in swap_scores:
                physical_swap_qubits = (current_layout.v2p[swap_qubits[0]], current_layout.v2p[swap_qubits[1]])
                # Determine whether the added swap gate is valid, because swap(1,2)swap(1,2)=identity
                if tuple(sorted(physical_swap_qubits)) not in unavailable_2qubits:
                    swap_cost = self._swap_score(physical_swap_qubits)
                    score_h = self._score_heuristic(
                        self.heuristic, front_layer, extended_set, current_layout, swap_qubits
                    )
                    score = swap_cost + score_h
                    swap_scores[swap_qubits] = score

            best_score = max(swap_scores.values())
            best_swaps = [k for k, v in swap_scores.items() if v == best_score]
            if len(best_swaps) == 1:
                best_swap = best_swaps[0]
            else:
                best_swap = choice(best_swaps)  # randomly select one

        elif self.heuristic == 'distance':
            for swap_qubits in swap_scores:
                score = self._score_heuristic(
                    self.heuristic, front_layer, extended_set, current_layout, swap_qubits
                )
                swap_scores[swap_qubits] = score

            best_score = min(swap_scores.values())
            best_swaps = [k for k, v in swap_scores.items() if v == best_score]
            if len(best_swaps) == 1:
                best_swap = best_swaps[0]
            else:
                best_swap = choice(best_swaps)  # randomly select one

        elif self.heuristic == 'mixture':
            for swap_qubits in swap_scores:
                score = self._score_heuristic(
                    'distance', front_layer, extended_set, current_layout, swap_qubits
                )
                swap_scores[swap_qubits] = score

            best_score = min(swap_scores.values())
            best_swaps = [k for k, v in swap_scores.items() if v == best_score]
            best_swap = best_swaps[0]

            if len(best_swaps) > 1:
                max_score = 0
                for swap_qubits in best_swaps:
                    physical_swap_qubits = (current_layout.v2p[swap_qubits[0]], current_layout.v2p[swap_qubits[1]])
                    if tuple(sorted(physical_swap_qubits)) not in unavailable_2qubits:
                        swap_cost = self._swap_score(physical_swap_qubits)
                        score_h = self._score_heuristic(
                            'fidelity', front_layer, extended_set, current_layout, swap_qubits
                        )
                        score = swap_cost + score_h
                        if max_score > score:
                            max_score = score
                            best_swap = swap_qubits

        else:
            raise NameError("No heuristic %s, currently only 'distance' and 'fidelity' are supported." % self.heuristic)
        return best_swap

    def _compute_distance_cost(self, layer, layout):
        """Calculate the distance sum of the gates list (layer) of the given layout.

        Args:
            layer: Gates list of a certain layer of the circuit.
            layout (Layout): Mapping of virtual qubits to physical qubits.

        Returns:
            cost (int): distance sum of the gates list in the given layer.
        """
        cost = 0
        for node in layer:
            cost += self.distance_matrix[layout.v2p[node.pos[0]], layout.v2p[node.pos[1]]]
        return cost

    def _compute_fidelity_cost(self, layer, layout):
        """Calculate the fidelity sum of the gates list (layer) of the given layout.

        Args:
            layer (list): Gates list of a certain layer of the circuit.
            layout (Layout): Mapping of virtual qubits to physical qubits.

        Returns:
            fidelity_cost (int): path_fidelity sum of the gates list in the given layer.
                                Since there may be exchanges of target qubit and control qubit that make
                                the fidelity of the two-qubit gate different, the average is taken here.
        """
        fidelity_cost = 0
        for node in layer:
            p1, p2 = layout.v2p[node.pos[0]], layout.v2p[node.pos[1]]
            fidelity_cost += 0.5 * (self.path_fidelity[(p1, p2)] + self.path_fidelity[(p2, p1)])
        return fidelity_cost

    def _compute_mixture_cost(self, layer, layout):
        """
        TODO: Deprecated
        """
        mixture_cost = 0
        sum_fidelity = 0
        for node in layer:
            s1, s2 = layout.v2p[node.pos[0]], layout.v2p[node.pos[1]]
            mean_fidelity_s12 = 0.5 * (np.exp(self.path_fidelity[(s1, s2)]) + np.exp(self.path_fidelity[(s2, s1)]))
            mixture_cost += self.distance_matrix[s1, s2] * mean_fidelity_s12
            sum_fidelity += mean_fidelity_s12
        mixture_cost = mixture_cost / sum_fidelity
        # mixture_cost = mixture_cost
        return mixture_cost

    def _score_heuristic(self, heuristic, front_layer, extended_set, current_layout, swap_qubits=None):
        """Get the heuristic score for the current layout.

        Args:
            heuristic (str): "distance" or "fidelity"
            front_layer (list): front layer gates list
            extended_set: set of expansion gates
            current_layout (Layout): current layout
            swap_qubits (tuple): swap qubits (i, j)

        Returns:
            the heuristic score
        """
        # trial_layout = copy.deepcopy(current_layout)
        trial_layout = Layout()
        trial_layout.set_layout(current_layout)
        trial_layout.swap(swap_qubits[0], swap_qubits[1])
        if heuristic == "distance":
            front_cost = self._compute_distance_cost(front_layer, trial_layout) / len(front_layer)
            extended_cost = 0
            if extended_set:
                extended_cost = self._compute_distance_cost(extended_set, trial_layout) / len(extended_set)
            total_cost = front_cost + self.extended_set_weight * extended_cost
            return total_cost * max(self.qubits_decay[swap_qubits[0]], self.qubits_decay[swap_qubits[1]])

        elif heuristic == "fidelity":
            # noise_front_cost = self._compute_fidelity_cost(front_layer, layout) / len(front_layer)
            noise_front_cost = self._compute_fidelity_cost(front_layer, trial_layout)
            noise_extended_cost = 0
            if extended_set:
                # noise_extended_cost = self._compute_fidelity_cost(extended_set, layout) / len(extended_set)
                noise_extended_cost = self._compute_fidelity_cost(extended_set, trial_layout)
            noise_total_cost = noise_front_cost + self.extended_set_weight * noise_extended_cost
            return 0.5 * (self.qubits_decay[swap_qubits[0]] + self.qubits_decay[swap_qubits[1]]) * noise_total_cost

        # elif heuristic == "mixture":
        #     mixture_front_cost = self._compute_mixture_cost(front_layer, trial_layout)
        #     mixture_extended_cost = 0
        #     if extended_set:
        #         mixture_extended_cost = self._compute_mixture_cost(extended_set, trial_layout)
        #     mixture_total_cost = mixture_front_cost + self.extended_set_weight * mixture_extended_cost
        #     return 0.5 * (self.qubits_decay[swap_qubits[0]] + self.qubits_decay[swap_qubits[1]]) * mixture_total_cost

        else:
            raise NameError("Heuristic %s not recognized." % heuristic)
