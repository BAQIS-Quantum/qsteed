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
import math
import re
from queue import PriorityQueue

import networkx as nx


class BuildLibrary:
    """Based on the hardware coupling structure and the fidelity information of two-qubit gates,
    construct an optimal qubit coupling substructure library."""

    def __init__(self, backend: str = 'ScQ-P10', fidelity_threshold: float = 0.9):
        """Initialize BuildLibrary.
        Args:
            backend: quafu backends ('ScQ-P10', 'ScQ-P18', 'ScQ-P136').
            fidelity_threshold: Two-qubit gate fidelity.
            backend_info (dict): Get backend information from :py:func:`quafu.Task.get_backend_info()`
        """
        self.backend = backend
        self.fidelity_threshold = fidelity_threshold

    def get_structure(self, json_topo_struct):
        """
        Args:
            json_topo_struct(dict): backend_info['full_info']["topological_structure"]
        Returns:
            int_to_qubit(dict): {0: Q01, 1: Q02, ...}
            qubit_to_int(dict): {Q01: 0, Q02: 0, ...}
            directed_weighted_edges(list):[[qubit1,qubit2,fidelity],...]
            connected_substructure_list(list): [networkx.Graph,...]
        """
        # Please note that 'int_to_qubit' needs to be consistent with the mapping of the backend,
        # and you can check 'backend_info['mapping']'.

        qubits_list = []
        for gate in json_topo_struct.keys():
            qubit = gate.split('_')
            qubits_list.append(qubit[0])
            qubits_list.append(qubit[1])
        qubits_list = list(set(qubits_list))
        qubits_list = sorted(qubits_list, key=lambda x: int(re.findall(r"\d+", x)[0]))
        int_to_qubit = {k: v for k, v in enumerate(qubits_list)}
        qubit_to_int = {v: k for k, v in enumerate(qubits_list)}

        directed_weighted_edges = []
        for gate, name_fidelity in json_topo_struct.items():
            gate_qubit = gate.split('_')
            qubit1 = qubit_to_int[gate_qubit[0]]
            qubit2 = qubit_to_int[gate_qubit[1]]
            gate_name = list(name_fidelity.keys())[0]
            fidelity = name_fidelity[gate_name]['fidelity']
            directed_weighted_edges.append([qubit1, qubit2, fidelity])

        connected_substructure_list = self.connected_substructure(directed_weighted_edges)
        return int_to_qubit, qubit_to_int, directed_weighted_edges, connected_substructure_list

    def connected_substructure(self, directed_weighted_edges: list = None):
        d = {}
        for item in directed_weighted_edges:
            key = tuple(sorted(item[:2]))
            if key in d:
                if item[2] > d[key][2]:
                    d[key] = item
            else:
                d[key] = item
        weighted_edges = list(d.values())
        G = nx.Graph()
        G.add_weighted_edges_from(weighted_edges)
        G0 = copy.deepcopy(G)
        connected_substructure_list = []
        while len(G.nodes()) > 0:
            connected_nodes = max(nx.connected_components(G), key=len)
            connected_subgraph = G0.subgraph(connected_nodes)
            connected_substructure_list.append(connected_subgraph)
            G.remove_nodes_from(connected_nodes)
        return connected_substructure_list

    def substructure(self, directed_weighted_edges, connected_substructure_list, qubits_need):
        """
        Args:
            directed_weighted_edges(list): [[qubit1,qubit2,fidelity],...], get from get_structure()
            connected_substructure_list(list): [networkx.Graph,...], get from get_structure()
            qubits_need: number of qubits required
        Returns:
            all_substructure(list): All coupling substructures with 'qubits_need' qubits
        """
        if directed_weighted_edges[0][2] > 1:
            directed_weighted_edges = [[item[0], item[1], item[2] / 100] for item in directed_weighted_edges]
        fidelity_threshold_fixed = self.fidelity_threshold
        all_substructure = []
        # substructure_nodes = []
        while len(all_substructure) == 0:
            for cg in connected_substructure_list:
                if len(cg.nodes()) >= qubits_need:
                    substructure_nodes = []  #
                    fidelity_threshold = fidelity_threshold_fixed  #
                    sorted_edges = sorted(cg.edges(data=True), key=lambda x: x[2]['weight'], reverse=True)
                    for elem in sorted_edges:
                        if elem[2]['weight'] > fidelity_threshold:
                            neighbors = PriorityQueue()
                            neighbors.put((-1, elem[0]))
                            ret_nodes = []
                            log_weight_product = 0
                            for node in cg.nodes():
                                cg.nodes[node]['visited'] = False
                            while neighbors.not_empty:
                                temp = neighbors.get()
                                node = temp[1]
                                if cg.nodes[node]['visited']:
                                    continue
                                weight = -temp[0]
                                if weight <= 0:
                                    weight = 1e-10
                                log_weight_product += math.log(weight)
                                cg.nodes[node]['visited'] = True
                                ret_nodes.append(node)
                                if len(ret_nodes) == qubits_need:
                                    break
                                for neighbor in cg[node]:
                                    if not cg.nodes[neighbor]['visited']:
                                        weight = cg[node][neighbor]['weight']
                                        neighbors.put((-weight, neighbor))
                            out = []
                            for edge in directed_weighted_edges:
                                if edge[0] in ret_nodes and edge[1] in ret_nodes:
                                    out.append(edge)
                            if sorted(ret_nodes) not in substructure_nodes and all(
                                    qubit[2] > fidelity_threshold for qubit in out):
                                substructure_nodes.append(sorted(ret_nodes))
                                # all_substructure.append([log_weight_product, out])
                                ave_weight = 0  # Dealing with loop structures
                                for e in out:
                                    if e[2] <= 0:  # If fidelity is 0, set it to 1e-10
                                        e[2] = 1e-10
                                    ave_weight += math.log(e[2])
                                ave_weight = ave_weight / len(out)
                                all_substructure.append([ave_weight, out])
                            if not substructure_nodes:
                                fidelity_threshold = fidelity_threshold - 0.01
            # If the current threshold does not find a substructure, reduce the threshold value.
            if len(all_substructure) == 0:
                fidelity_threshold_fixed = fidelity_threshold_fixed - 0.01
        all_substructure = sorted(all_substructure, key=lambda x: x[0], reverse=True)
        return all_substructure

    def build_substructure_library(self, json_topo_struct):
        # substructure_dict = defaultdict(list)
        substructure_dict = {}
        int_to_qubit, qubit_to_int, directed_weighted_edges, connected_substructure_list = self.get_structure(
            json_topo_struct)
        substructure_dict[1] = [[[q, q, 0.99]] for q in int_to_qubit.keys()]
        for qubits in range(2, len(connected_substructure_list[0].nodes()) + 1):
            sub_graph = self.substructure(directed_weighted_edges, connected_substructure_list, qubits)
            qlist = []
            for j in range(len(sub_graph)):
                sublist = sub_graph[j][1]
                sublist.sort(key=lambda x: x[0])
                qlist.append(sublist)
            substructure_dict[qubits] = qlist

        sorted_weighted_edges = sorted(directed_weighted_edges, key=lambda x: x[2], reverse=True)
        save_substructure = {'structure': sorted_weighted_edges, 'substructure_dict': substructure_dict,
                             'int_to_qubit': int_to_qubit, 'qubit_to_int': qubit_to_int}
        return save_substructure
