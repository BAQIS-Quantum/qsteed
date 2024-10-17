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
import json
import math
import os
import re
from collections import defaultdict
from queue import PriorityQueue

import networkx as nx


class BuildLibrary:
    """Based on the hardware coupling structure and the fidelity information of two-qubit gates,
    construct an optimal qubit coupling substructure library."""

    def __init__(self, backend: str = None, fidelity_threshold: float = 0.9):
        """Initialize BuildLibrary.
        Args:
            backend: quafu backends ('ScQ-P10', 'ScQ-P18', 'ScQ-P136').
            fidelity_threshold: Two-qubit gate fidelity.
            backend_info (dict): Get backend information from :py:func:`quafu.Task.get_backend_info()`
        """
        self.backend = backend
        self.fidelity_threshold = fidelity_threshold

    def get_structure(self, json_topo_struct, qubits_info, int_to_qubit=None):
        """
        Args:
            json_topo_struct(dict): backend_info['full_info']["topological_structure"]
            qubits_info(dict): backend_info['full_info']['qubits_info']
            int_to_qubit(dict): {0: Q01, 1: Q02, ...}
        Returns:
            int_to_qubit(dict): {0: Q01, 1: Q02, ...}
            qubit_to_int(dict): {Q01: 0, Q02: 0, ...}
            directed_weighted_edges(list):[[qubit1,qubit2,fidelity],...]
            connected_substructure_list(list): [networkx.Graph,...]
        """
        # Please note that 'int_to_qubit' needs to be consistent with the mapping of the backend,
        # and you can check 'backend_info['mapping']'.

        if int_to_qubit is None:
            qubits_list = list(qubits_info.keys())
            qubits_list = sorted(qubits_list, key=lambda x: int(re.findall(r"\d+", x)[0]))
            int_to_qubit = {k: v for k, v in enumerate(qubits_list)}
            qubit_to_int = {v: k for k, v in enumerate(qubits_list)}
        else:
            qubit_to_int = {v: k for k, v in int_to_qubit.items()}

        int_to_qubit = {v: k for k, v in qubit_to_int.items()}

        directed_weighted_edges = []
        for gate, name_fidelity in json_topo_struct.items():
            gate_qubit = gate.split('_')
            if gate_qubit[0] in qubit_to_int and gate_qubit[1] in qubit_to_int:
                qubit1 = qubit_to_int[gate_qubit[0]]
                qubit2 = qubit_to_int[gate_qubit[1]]
                gate_name = list(name_fidelity.keys())[0]
                fidelity = name_fidelity[gate_name]['fidelity']
                directed_weighted_edges.append([qubit1, qubit2, fidelity])

        structure = sorted(directed_weighted_edges, key=lambda x: x[2], reverse=True)
        # structure = [edge for edge in structure if edge[2] != 0]
        structure = self.get_symmetrical_structure(structure)
        return int_to_qubit, qubit_to_int, structure

    def connected_substructure(self, structure: list = None):
        d = {}
        for item in structure:
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

    def substructure(self, structure, connected_substructure_list, qubits_need):
        """
        Args:
            structure(list): [[qubit1,qubit2,fidelity],...], get from get_structure()
            connected_substructure_list(list): [networkx.Graph,...], get from connected_substructure()
            qubits_need: number of qubits required
        Returns:
            all_substructure(list): All coupling substructures with 'qubits_need' qubits
        """
        if structure[0][2] > 1:
            structure = [[item[0], item[1], item[2] / 100] for item in structure]
        fidelity_threshold_fixed = self.fidelity_threshold
        all_substructure = []
        has_zero_fidelity = False
        available_connected_substructure_list = []
        for cg in connected_substructure_list:
            if len(cg.nodes()) >= qubits_need:
                available_connected_substructure_list.append(cg)

        if available_connected_substructure_list:
            while len(all_substructure) == 0:
                for cg in available_connected_substructure_list:
                    # if len(cg.nodes()) >= qubits_need:
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
                                    has_zero_fidelity = True
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
                            for edge in structure:
                                if edge[0] in ret_nodes and edge[1] in ret_nodes:
                                    out.append(edge)
                            if sorted(ret_nodes) not in substructure_nodes and all(
                                    qubit[2] > fidelity_threshold for qubit in out):
                                substructure_nodes.append(sorted(ret_nodes))
                                ave_weight = 0  # Dealing with loop structures
                                for e in out:
                                    if e[2] <= 0:  # If fidelity is 0, set it to 1e-10
                                        has_zero_fidelity = True
                                        e[2] = 1e-10
                                    ave_weight += math.log(e[2])

                                ave_weight = ave_weight / len(out)
                                all_substructure.append([ave_weight, out])
                            if not substructure_nodes:
                                fidelity_threshold = fidelity_threshold - 0.01

                # If the current threshold does not find a substructure, reduce the threshold value.
                if len(all_substructure) == 0:
                    fidelity_threshold_fixed = fidelity_threshold_fixed - 0.01
        else:
            raise ValueError(
                'The number of qubits required for the task exceeds the number of qubits available in the hardware.')
        all_substructure = sorted(all_substructure, key=lambda x: x[0], reverse=True)
        # if has_zero_fidelity:  # If fidelity = 0, remove this substructure
        #     all_substructure = [sub for sub in all_substructure if not any(sublist[2] == 1e-10 for sublist in sub[1])]
        return all_substructure

    def build_substructure_library(self, structure, int_to_qubit, priority_qubits=None):
        substructure_dict = {}
        # int_to_qubit, qubit_to_int, structure = self.get_structure(
        #     json_topo_struct, int_to_qubit)
        substructure_dict[1] = [[[q, q, 0.999]] for q in int_to_qubit.keys()]

        connected_substructure_list = self.connected_substructure(structure)

        # Disconnect two-qubit gate fidelity zero
        re_structure = [edge for edge in structure if edge[2] != 0]
        # print('structure',structure)
        # print('re_structure',re_structure)
        re_connected_substructure_list = self.connected_substructure(re_structure)
        # max_priority = max((len(item) if isinstance(item, tuple) else 1 for item in priority_regions), default=0)

        if priority_qubits is None:
            max_priority = 0
        else:
            max_priority = max((len(item) if isinstance(item, tuple) else 1 for item in priority_qubits), default=0)

        if priority_qubits is None or max_priority <= 1:
            for qubits in range(2, len(re_connected_substructure_list[0].nodes()) + 1):
                sub_graph = self.substructure(re_structure, re_connected_substructure_list, qubits)
                qlist = self.get_qlist(sub_graph)
                substructure_dict[qubits] = qlist
        else:
            # Create substructure libraries based on priority qubits
            record_sub_graph = defaultdict(list)
            for qubits_list in priority_qubits:
                for qubits in range(2, len(qubits_list) + 1):
                    # Need to get directed_weighted_edges based on qubits_list,
                    # Then use connected_substructure to get connected_substructure_list
                    sub_directed_weighted, sub_connected_structure = self.get_subchip(re_structure, qubits_list)
                    sub_graph = self.substructure(sub_directed_weighted, sub_connected_structure, qubits)
                    record_sub_graph[qubits].extend(sub_graph)
            for qubits, sub_graph in record_sub_graph.items():
                # Sort or not, If you need to sort, uncomment the following.
                # sub_graph = sorted(sub_graph, key=lambda x: x[0], reverse=True)
                qlist = self.get_qlist(sub_graph)
                if qubits in substructure_dict:
                    substructure_dict[qubits].append(qlist)
                else:
                    substructure_dict[qubits] = qlist

            # Create the remaining substructure libraries
            max_priority_qubits = max(priority_qubits, key=len)
            for qubits in range(len(max_priority_qubits) + 1, len(re_connected_substructure_list[0].nodes()) + 1):
                sub_graph = self.substructure(re_structure, re_connected_substructure_list, qubits)
                qlist = self.get_qlist(sub_graph)
                if qubits in substructure_dict:
                    substructure_dict[qubits].append(qlist)
                else:
                    substructure_dict[qubits] = qlist

        # Even two-qubit gates with zero fidelity are connected
        for qubits in range(len(substructure_dict) + 1, len(connected_substructure_list[0].nodes()) + 1):
            sub_graph = self.substructure(structure, connected_substructure_list, qubits)
            qlist = self.get_qlist(sub_graph)
            substructure_dict[qubits] = qlist

        # print('SubQPU coupling structure:\n', substructure_dict)
        print("Mapping:", int_to_qubit)

        save_substructure = {'structure': structure, 'substructure_dict': substructure_dict,
                             'int_to_qubit': int_to_qubit}

        # current_dir = os.path.abspath(os.path.dirname(__file__))
        user_home = os.path.expanduser('~')
        # target_dir = os.path.join(current_dir, '..', '..', '..')
        # normalized_path = os.path.normpath(target_dir)
        folder_path = os.path.join(user_home, "QSteed")
        if not os.path.exists(folder_path):
            os.mkdir(folder_path)
        file_name = self.backend + '_structure.txt'
        file_path = os.path.join(folder_path, file_name)

        with open(file_path, "w") as file:
            file.write(json.dumps(save_substructure))
            # file.write(str(save_substructure))
        print("Check the construction data of the chip in the txt file: " + file_path)
        return substructure_dict

    def get_subchip(self, structure, qubits_list):
        """Priority selected qubits
        Args:
            structure(list): [[0,1,0.99],[1,2,0.98]]
            qubits_list(list): for example: [108, 109, 119, 120, 121]
        Returns:
            directed_weighted_edges(list):[[qubit1,qubit2,fidelity],...]
            connected_substructure_list(list): [networkx.Graph,...]

        """
        directed_weighted_edges = []
        for qubit1, qubit2, fidelity in structure:
            if qubit1 in qubits_list and qubit2 in qubits_list:
                directed_weighted_edges.append([qubit1, qubit2, fidelity])

        connected_substructure_list = self.connected_substructure(directed_weighted_edges)
        return directed_weighted_edges, connected_substructure_list

    def get_qlist(self, sub_graph):
        qlist = []
        for j in range(len(sub_graph)):
            sublist = sub_graph[j][1]
            sublist.sort(key=lambda x: x[0])
            qlist.append(sublist)
        return qlist

    def get_symmetrical_structure(self, structure):
        # Create a dictionary to track pairs and their values
        pair_dict = {}

        # Fill the dictionary with initial pairs and check for reverse pairs
        for pair in structure:
            key = tuple(pair[:2])
            reverse_key = tuple(pair[:2][::-1])

            # Add the pair and its reverse to the dictionary if not already present
            pair_dict[key] = pair[2]
            if reverse_key not in pair_dict:
                pair_dict[reverse_key] = pair[2]

        # Convert the dictionary back to a list format
        structure = [[k[0], k[1], v] for k, v in pair_dict.items()]

        return structure
