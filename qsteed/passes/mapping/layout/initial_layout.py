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

import random
from collections import defaultdict

import networkx as nx
import numpy as np

from qsteed.graph.circuitgraph import circuit_to_graph
from qsteed.passes.mapping.baselayout import Layout


def node_degree(g):
    """Get the degree of nodes in the given dag.

    Args:
        g (nx.Graph): a given dag
    Returns:
        g_node_degree (list): Sort by node degree, [(node, node degree),...]
    """
    g_node_degree = []
    for node in g.nodes():
        g_node_degree.append((node, g.degree[node]))
    g_node_degree = sorted(g_node_degree, key=lambda item: item[1], reverse=True)
    return g_node_degree


def initial_layout(circuit, coupling, method: str = None):
    """Get the initial mapping according to the degree of the node of the circuit graph
    and the qubit coupling graph. Mainly used for initial layout of Sabre algorithm.

    Args:
        circuit (QuantumCircuit): a pyquafu QuantumCircuit
        coupling (list): qubits coupling list, [[q0, q1, fidelity],...]
    Returns:
        initial_layout (Layout): initial layout
    """
    g1 = circuit_to_graph(circuit)
    g2 = nx.Graph()
    for item in coupling:
        g2.add_edges_from([(item[0], item[1], {'weight': item[2]})])

    matcher = nx.isomorphism.GraphMatcher(g1, g2)

    dict_mapping = {}
    if matcher.is_isomorphic():
        mapping = matcher.mapping
        for node_g1, node_g2 in mapping.items():
            dict_mapping[node_g1] = node_g2
    else:
        if method == 'fidelity':
            dict_mapping = initial_mapping_fidelity(g1, g2)
        elif method == 'degree':
            dict_mapping = initial_mapping_degree(g1, g2)
        elif method is None or "random":
            nodes_g1 = list(g1.nodes())
            nodes_g2 = list(g2.nodes())
            if len(nodes_g1) != len(nodes_g2):
                raise ValueError(
                    "The number of nodes in a circuit is different from the number of nodes in a given coupling structure.")

            shuffled_g2 = nodes_g2[:]
            random.shuffle(shuffled_g2)
            dict_mapping = dict(zip(nodes_g1, shuffled_g2))
        else:
            raise ValueError("Method does not exist.")

    # Do not perform any initial layout sorting.
    phys_qubits = [dict_mapping[i] for i in range(len(dict_mapping))]
    v2p_dict = {i: phys_qubits[i] for i in range(len(phys_qubits))}
    initial_layout = Layout(v2p_dict)

    # Initialize sorting according to the minimum fidelity qubit.
    # initial_layout = _sort_layout_minimum_fidelity(dict_mapping, coupling)

    # Sort the initialized Layout according to the product of all qubits fidelity.
    # initial_layout = _sort_layout_all_fidelity(dict_mapping, coupling)
    return initial_layout


def initial_layout_isomorphism(circuit, coupling):
    """Get the initial mapping according to the degree of the node of the circuit graph
    and the qubit coupling graph. Mainly used for initial layout of Sabre algorithm.

    Args:
        circuit (QuantumCircuit): a pyquafu QuantumCircuit
        coupling (list): qubits coupling list, [[q0, q1, fidelity],...]
    Returns:
        initial_layout (Layout): initial layout
    """
    g1 = circuit_to_graph(circuit)
    g2 = nx.Graph()
    for item in coupling:
        g2.add_edges_from([(item[0], item[1], {'weight': item[2]})])

    matcher = nx.isomorphism.GraphMatcher(g1, g2)

    dict_mapping = {}
    if matcher.is_isomorphic():
        mapping = matcher.mapping
        for node_g1, node_g2 in mapping.items():
            dict_mapping[node_g1] = node_g2
    else:
        nodes_g1 = list(g1.nodes())
        nodes_g2 = list(g2.nodes())
        if len(nodes_g1) != len(nodes_g2):
            raise ValueError(
                "The number of nodes in a circuit is different from the number of nodes in a given coupling structure.")

        shuffled_g2 = nodes_g2[:]
        random.shuffle(shuffled_g2)
        dict_mapping = dict(zip(nodes_g1, shuffled_g2))

    # Sort the initialized Layout according to the product of all qubits fidelity.
    # initial_layout = _sort_layout_all_fidelity(dict_mapping, coupling)

    # Do not perform any initial layout sorting.
    phys_qubits = [dict_mapping[i] for i in range(len(dict_mapping))]
    v2p_dict = {i: phys_qubits[i] for i in range(len(phys_qubits))}
    initial_layout = Layout(v2p_dict)
    return initial_layout


def initial_mapping_degree(g1, g2):
    """
    """
    g1_node_degree = node_degree(g1)
    g2_node_degree = node_degree(g2)

    dict_mapping = {}
    for index in range(len(g1_node_degree)):
        dict_mapping[g1_node_degree[index][0]] = g2_node_degree[index][0]

    return dict_mapping


def initial_mapping_fidelity(g1, g2):
    """The initial mapping is obtained according to the node degrees
     and edge weights of the circuit graph and qubit coupling graph.
     Prioritize the mapping of nodes with high degrees,
     and map to nodes with large edge weights when the nodes have the same degree.
     Mainly used for initial layout of Sabre algorithm.

    Args:
        circuit (QuantumCircuit): a pyquafu QuantumCircuit
        coupling (list): qubits coupling list, [[q0, q1, fidelity],...]
    Returns:
        initial_layout (Layout): initial layout
    """
    g1_node_degree = node_degree(g1)
    g2_node_degree = node_degree(g2)

    dict_mapping = {}

    g1_dict = defaultdict(list)
    for item in g1_node_degree:
        g1_dict[item[1]].append(item[0])
    g2_dict = defaultdict(list)
    for item in g2_node_degree:
        g2_dict[item[1]].append(item[0])

    while g1_dict:
        max_g1_d = max(g1_dict.keys())
        if max_g1_d == 0:  # The node degree is 0, which means that there is no two-qubit gate for this qubit
            remaining_node = g1_dict[max_g1_d]
            remaining_qubit = [i for k, v in g2_dict.items() for i in v]
            for i in range(len(remaining_node)):
                dict_mapping[remaining_node[i]] = remaining_qubit[i]
            break
        else:
            if len(g1_dict[max_g1_d]) == 1:
                g1_node = g1_dict[max_g1_d][0]

                max_g2_d = max(g2_dict.keys())
                if len(g2_dict[max_g2_d]) == 1:
                    dict_mapping[g1_node] = g2_dict[max_g2_d][0]
                    g2_dict.pop(max_g2_d)
                else:
                    max_fidelity = 0
                    best_node = 0
                    for node in g2_dict[max_g2_d]:
                        edge = list(max(g2.edges(node), key=lambda e: g2.get_edge_data(*e)['weight']))
                        fidelity = g2.get_edge_data(edge[0], edge[1])['weight']
                        if fidelity > max_fidelity:
                            max_fidelity = fidelity
                            best_node = node
                    dict_mapping[g1_node] = best_node
                    g2_dict[max_g2_d].remove(best_node)

                g1_dict.pop(max_g1_d)
            elif len(g1_dict[max_g1_d]) > 1:  # When the node degrees are the same, compare the edge weights.
                max_g1_weight = 0
                best_g1_node = 0
                for node in g1_dict[max_g1_d]:
                    edge = list(max(g1.edges(node), key=lambda e: g1.get_edge_data(*e)['weight']))
                    weight = g1.get_edge_data(edge[0], edge[1])['weight']
                    if weight > max_g1_weight:
                        max_g1_weight = weight
                        best_g1_node = node
                ####
                max_g2_d = max(g2_dict.keys())
                if len(g2_dict[max_g2_d]) == 1:
                    dict_mapping[best_g1_node] = g2_dict[max_g2_d][0]
                    g2_dict.pop(max_g2_d)
                else:
                    max_fidelity = 0
                    best_node = 0
                    for node in g2_dict[max_g2_d]:
                        edge = list(max(g2.edges(node), key=lambda e: g2.get_edge_data(*e)['weight']))
                        fidelity = g2.get_edge_data(edge[0], edge[1])['weight']
                        if fidelity > max_fidelity:
                            max_fidelity = fidelity
                            best_node = node
                    dict_mapping[best_g1_node] = best_node
                    g2_dict[max_g2_d].remove(best_node)

                g1_dict[max_g1_d].remove(best_g1_node)
            else:
                pass

    return dict_mapping


def _sort_layout_minimum_fidelity(dict_mapping, coupling):
    # Initialize sorting according to the minimum fidelity qubit
    phys_qubits = [dict_mapping[i] for i in range(len(dict_mapping))]
    min_coupling = sorted(coupling, key=lambda x: x[2])[0]
    if min_coupling[0] > min_coupling[1]:
        if phys_qubits[min_coupling[0]] < phys_qubits[min_coupling[1]]:
            phys_qubits.reverse()
    else:
        if phys_qubits[min_coupling[0]] > phys_qubits[min_coupling[1]]:
            phys_qubits.reverse()
    v2p_dict = {i: phys_qubits[i] for i in range(len(phys_qubits))}
    initial_layout = Layout(v2p_dict)
    return initial_layout


def _sort_layout_all_fidelity(dict_mapping, coupling):
    # Sort the initialized Layout according to the product of all qubits fidelity
    mapping_pyh2logi = {v: k for k, v in dict_mapping.items()}
    same_direction_fidelity = 0
    same_direction_num = 0
    reverse_fidelity = 0
    reverse_num = 0
    illegal_gates = 0
    # sorted_coupling = sorted(coupling, key=lambda x: x[2])
    # for g in sorted_coupling[0:int(len(sorted_coupling)/2)]:
    for g in coupling:
        if g[2] < 1e-5:
            illegal_gates += 1
        if (g[0] > g[1] and mapping_pyh2logi[g[0]] > mapping_pyh2logi[g[1]]) or \
                (g[0] < g[1] and mapping_pyh2logi[g[0]] < mapping_pyh2logi[g[1]]):
            same_direction_fidelity += np.log(g[2])
            same_direction_num += 1
        else:
            reverse_fidelity += np.log(g[2])
            reverse_num += 1

    phys_qubits = [dict_mapping[i] for i in range(len(dict_mapping))]
    if illegal_gates > 0:
        if same_direction_fidelity < reverse_fidelity:
            phys_qubits.reverse()
    else:
        if same_direction_num < reverse_num:
            phys_qubits.reverse()
        elif same_direction_num == reverse_num:
            if same_direction_fidelity < reverse_fidelity:
                phys_qubits.reverse()
    v2p_dict = {i: phys_qubits[i] for i in range(len(phys_qubits))}
    initial_layout = Layout(v2p_dict)
    return initial_layout
