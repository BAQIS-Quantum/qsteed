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

import networkx as nx
import numpy as np


def wl_subtree_kernel(g1: nx.Graph, g2: nx.Graph, iteration: int = 3, consider_weight: bool = True):
    """
    Compute the Weisfeiler-Lehman Subtree Kernel between two graphs.

    Args:
        g1 (nx.Graph): First input graph.
        g2 (nx.Graph): Second input graph.
        iteration (int): Maximum height for the subtree kernel computation.
        consider_weight (bool): Whether to consider edge weights in the kernel computation.

    Returns:
        kernel_value (float): Subtree kernel similarity between g1 and g2.
    """
    kernel_value = 0
    for _ in range(iteration):
        kernel_value += _subtree_kernel(g1, g2, consider_weight=consider_weight)
        g1, g2 = _iteration_graph(g1, g2, consider_weight=consider_weight)
    return kernel_value


def _subtree_kernel(g1: nx.Graph, g2: nx.Graph, consider_weight: bool = False):
    """
    Compute the Subtree Kernel between two graphs, optionally considering edge weights.

    Args:
        g1 (nx.Graph): First input graph.
        g2 (nx.Graph): Second input graph.
        consider_weight (bool): Whether to consider weights in the edge comparison.

    Returns:
        value (float): Kernel value based on edge structure and optionally weights.
    """
    value = 0
    for edge1 in g1.edges.data():
        from1, to1 = sorted((edge1[0], edge1[1]))  # Ensure consistent ordering
        weight1 = edge1[2].get('weight', 1) if consider_weight else None  # Default to 1 if no weight

        for edge2 in g2.edges.data():
            from2, to2 = sorted((edge2[0], edge2[1]))
            weight2 = edge2[2].get('weight', 1) if consider_weight else None  # Default to 1 if no weight

            # Compare edge structure and optionally weights
            if from1 == from2 and to1 == to2:
                if consider_weight:
                    value += 1 / np.exp((weight1 - weight2) ** 2)
                else:
                    value += 1
    return value


def _iteration_graph(g1: nx.Graph, g2: nx.Graph, consider_weight: bool = False):
    """
    Iteratively generate subtree graph by updating node labels, optionally considering weights.

    Args:
        g1 (nx.Graph): First input graph.
        g2 (nx.Graph): Second input graph.
        consider_weight (bool): Whether to consider weights when updating node labels.

    Returns:
        g1 (nx.Graph), g2 (nx.Graph): Updated graphs with new node labels.
    """
    num = 0
    dic = {}  # A dictionary to map node labels to unique values
    res1, res2 = {}, {}  # Dictionaries to store updated node labels for g1 and g2

    # Process graph g1 nodes
    for node1, data1 in g1.nodes.data():
        weight1 = data1.get('weight', 0) if consider_weight else 0  # Default to 0 if no weight
        key = str(weight1) + "," if consider_weight else ","
        neighbor_weights1 = [g1.nodes[neighbor].get('weight', 0) if consider_weight else 0 for neighbor in
                             g1.neighbors(node1)]
        neighbor_weights1.sort()  # Sort neighbors' weights

        for neighbor_weight in neighbor_weights1:
            key += str(neighbor_weight)

        if key not in dic:
            dic[key] = num
            res1[node1] = num
            num += 1
        else:
            res1[node1] = dic[key]

    # Process graph g2 nodes
    for node2, data2 in g2.nodes.data():
        weight2 = data2.get('weight', 0) if consider_weight else 0
        key = str(weight2) + "," if consider_weight else ","
        neighbor_weights2 = [g2.nodes[neighbor].get('weight', 0) if consider_weight else 0 for neighbor in
                             g2.neighbors(node2)]
        neighbor_weights2.sort()  # Sort neighbors' weights

        for neighbor_weight in neighbor_weights2:
            key += str(neighbor_weight)

        if key not in dic:
            dic[key] = num
            res2[node2] = num
            num += 1
        else:
            res2[node2] = dic[key]

    # Update node labels in both graphs based on new labels
    for node1, new_label in res1.items():
        g1.nodes[node1]['weight'] = new_label
    for node2, new_label in res2.items():
        g2.nodes[node2]['weight'] = new_label

    return g1, g2


def wl_subtree_kernel_cached(g1: nx.Graph, g2: nx.Graph, iteration: int = 3, consider_weight: bool = True):
    """
    Compute the Weisfeiler-Lehman Subtree Kernel between two graphs.

    Args:
        g1 (nx.Graph): First input graph.
        g2 (nx.Graph): Second input graph.
        iteration (int): Maximum height for the subtree kernel computation.
        consider_weight (bool): Whether to consider edge weights in the kernel computation.

    Returns:
        kernel_value (float): Subtree kernel similarity between g1 and g2.
    """
    kernel_value = 0
    cache = {}  # Initialize a cache to store previously computed edge/node comparisons

    for _ in range(iteration):
        # Compute the kernel value by comparing g1 and g2's features at each iteration
        kernel_value += _subtree_kernel_cached(g1, g2, consider_weight=consider_weight, cache=cache)

        # Update the graphs for the next iteration
        g1, g2, cache = _iteration_graph_cached(g1, g2, consider_weight=consider_weight, cache=cache)

    return kernel_value


def _subtree_kernel_cached(g1: nx.Graph, g2: nx.Graph, consider_weight: bool = False, cache: dict = None):
    """
    Compute the Subtree Kernel between two graphs, with caching for edge and node comparisons.

    Args:
        g1 (nx.Graph): First input graph.
        g2 (nx.Graph): Second input graph.
        consider_weight (bool): Whether to consider weights in the edge comparison.
        cache (dict): Cache dictionary to store previously computed edge/node comparisons.

    Returns:
        value (float): Kernel value based on edge structure and optionally weights.
    """
    if cache is None:
        cache = {}

    value = 0

    # Iterate over edges in g1
    for edge1 in g1.edges.data():
        from1, to1 = sorted((edge1[0], edge1[1]))  # Ensure consistent ordering
        weight1 = edge1[2].get('weight', 1) if consider_weight else None  # Default to 1 if no weight

        # Check if the edge comparison is already cached
        if (from1, to1) in cache:
            # Use the cached value for this edge
            value += cache[(from1, to1)]
        else:
            # If not cached, compare edges with g2
            for edge2 in g2.edges.data():
                from2, to2 = sorted((edge2[0], edge2[1]))
                weight2 = edge2[2].get('weight', 1) if consider_weight else None  # Default to 1 if no weight

                # Compare edge structure and optionally weights
                if from1 == from2 and to1 == to2:
                    if consider_weight:
                        similarity = 1 / np.exp((weight1 - weight2) ** 2)
                    else:
                        similarity = 1
                    cache[(from1, to1)] = similarity  # Cache the result
                    value += similarity

    return value


def _iteration_graph_cached(g1: nx.Graph, g2: nx.Graph, consider_weight: bool = False, cache: dict = None):
    """
    Iteratively generate subtree graph by updating node labels, with caching to avoid redundant computations.

    Args:
        g1 (nx.Graph): First input graph.
        g2 (nx.Graph): Second input graph.
        consider_weight (bool): Whether to consider weights when updating node labels.
        cache (dict): Cache dictionary to store previously computed node labels.

    Returns:
        g1 (nx.Graph), g2 (nx.Graph): Updated graphs with new node labels.
    """
    if cache is None:
        cache = {}

    num = 0
    dic = {}  # A dictionary to map node labels to unique values
    res1, res2 = {}, {}  # Dictionaries to store updated node labels for g1 and g2

    # Process graph g1 nodes
    for node1, data1 in g1.nodes.data():
        weight1 = data1.get('weight', 0) if consider_weight else 0  # Default to 0 if no weight
        key = str(weight1) + "," if consider_weight else ","

        # Check if the node's label calculation is cached
        if node1 in cache:
            res1[node1] = cache[node1]
            continue

        # Otherwise, calculate the node's label and cache it
        neighbor_weights1 = [g1.nodes[neighbor].get('weight', 0) if consider_weight else 0 for neighbor in
                             g1.neighbors(node1)]
        neighbor_weights1.sort()  # Sort neighbors' weights

        for neighbor_weight in neighbor_weights1:
            key += str(neighbor_weight)

        if key not in dic:
            dic[key] = num
            res1[node1] = num
            num += 1
        else:
            res1[node1] = dic[key]

        # Cache the result for this node
        cache[node1] = res1[node1]

    # Process graph g2 nodes
    for node2, data2 in g2.nodes.data():
        weight2 = data2.get('weight', 0) if consider_weight else 0
        key = str(weight2) + "," if consider_weight else ","

        # Check if the node's label calculation is cached
        if node2 in cache:
            res2[node2] = cache[node2]
            continue

        # Otherwise, calculate the node's label and cache it
        neighbor_weights2 = [g2.nodes[neighbor].get('weight', 0) if consider_weight else 0 for neighbor in
                             g2.neighbors(node2)]
        neighbor_weights2.sort()  # Sort neighbors' weights

        for neighbor_weight in neighbor_weights2:
            key += str(neighbor_weight)

        if key not in dic:
            dic[key] = num
            res2[node2] = num
            num += 1
        else:
            res2[node2] = dic[key]

        # Cache the result for this node
        cache[node2] = res2[node2]

    # Update node labels in both graphs based on new labels
    for node1, new_label in res1.items():
        g1.nodes[node1]['weight'] = new_label
    for node2, new_label in res2.items():
        g2.nodes[node2]['weight'] = new_label

    return g1, g2, cache


def fast_subtree_kernel(g1, g2, iteration: int = 3, consider_weight: bool = False):
    """Compute the Fast Subtree Kernel between two graphs.

    References:
        Shervashidze, N., & Borgwardt, K. M. (2009). Fast subtree kernels on graphs.
        In Advances in Neural Information Processing Systems (pp. 1660-1668).

    Args:
        g1 (nx.Graph): The first graph.
        g2 (nx.Graph): The second graph.
        iteration (int): Maximum height for the subtree kernel computation.
        consider_weight (bool): Whether to consider edge weights in the kernel computation.

    Returns:
        float: Subtree kernel similarity between g1 and g2.
    """
    labels1 = _get_labels(g1, consider_weight)
    labels2 = _get_labels(g2, consider_weight)
    node_pairs1 = _get_node_pairs(g1)
    node_pairs2 = _get_node_pairs(g2)
    subtrees1 = _get_subtrees(g1, node_pairs1, consider_weight)
    subtrees2 = _get_subtrees(g2, node_pairs2, consider_weight)

    K = np.zeros((len(node_pairs1), len(node_pairs2)))
    for i in range(iteration):
        for j in range(len(node_pairs1)):
            for k in range(len(node_pairs2)):
                if i == 0:
                    if labels1[node_pairs1[j][0]] == labels2[node_pairs2[k][0]]:
                        K[j][k] = 1
                else:
                    subtree1 = subtrees1[node_pairs1[j]]
                    subtree2 = subtrees2[node_pairs2[k]]
                    if nx.is_isomorphic(subtree1, subtree2):
                        K[j][k] += K[j - 1][k - 1]

    return K[-1][-1]


def _get_labels(graph, consider_weight):
    """Get labels of graph, optionally considering edge weights."""
    labels = {}
    for node in graph.nodes():
        # Generate node label based on node weight and neighbors' weights if consider_weight is True
        if consider_weight:
            label = str(graph.nodes[node].get('weight', 1))  # Default weight is 1
            for neighbor in graph.neighbors(node):
                label += '_' + str(graph[node][neighbor].get('weight', 1))  # Add edge weight
        else:
            label = str(graph.nodes[node].get('weight', 1))  # Default weight is 1
        labels[node] = label
    return labels


def _get_node_pairs(graph):
    """Obtain all node pairs in graph."""
    node_pairs = []
    for node in graph.nodes():
        for neighbor in graph.neighbors(node):
            node_pairs.append((node, neighbor))
    return node_pairs


def _get_subtrees(graph, node_pairs, consider_weight):
    """Generate a subgraph based on node pairs, optionally considering edge weights."""
    subtrees = {}
    for node_pair in node_pairs:
        subtree = nx.Graph()
        # Use all shortest paths between the node pair to generate subgraph
        paths = list(nx.all_shortest_paths(graph, node_pair[0], node_pair[1]))

        for path in paths:
            for i in range(len(path) - 1):
                u, v = path[i], path[i + 1]
                if consider_weight:
                    weight = graph[u][v].get('weight', 1)  # Consider edge weight
                else:
                    weight = 1  # Default weight if not considering weights
                subtree.add_edge(u, v, weight=weight)

        subtrees[node_pair] = subtree
    return subtrees


def wl_oa_kernel(g1, g2, iteration=3):
    """
    Compute the Weisfeiler-Lehman Optimal Assignment (WL-OA) Kernel between two graphs.

    Args:
        g1 (nx.Graph): The first graph.
        g2 (nx.Graph): The second graph.
        iteration (int): Number of WL iterations.

    Returns:
        float: The WL-OA kernel value.
    """
    # Step 1: Initialization - assign initial labels
    labels_g1 = {node: str(data) for node, data in g1.nodes(data=True)}
    labels_g2 = {node: str(data) for node, data in g2.nodes(data=True)}

    for _ in range(iteration):
        # Step 2: Generate multisets of neighborhood labels
        for graph, labels in zip([g1, g2], [labels_g1, labels_g2]):
            new_labels = {}
            for node in graph:
                neighbor_labels = sorted([labels[neighbor] for neighbor in graph.neighbors(node)])
                new_labels[node] = labels[node] + "_" + "_".join(neighbor_labels)
            labels.update(new_labels)

        # Step 3: Relabel nodes to unique new labels
        unique_labels = set(labels_g1.values()).union(set(labels_g2.values()))
        label_mapping = {label: idx for idx, label in enumerate(unique_labels)}
        labels_g1 = {node: label_mapping[label] for node, label in labels_g1.items()}
        labels_g2 = {node: label_mapping[label] for node, label in labels_g2.items()}

        # Step 4: Compute Optimal Assignment (OA) Kernel
        cost_matrix = np.zeros((len(g1.nodes), len(g2.nodes)))
        for i, label1 in enumerate(labels_g1.values()):
            for j, label2 in enumerate(labels_g2.values()):
                cost_matrix[i, j] = 1 if label1 == label2 else 0

        from scipy.optimize import linear_sum_assignment
        row_ind, col_ind = linear_sum_assignment(-cost_matrix)
        kernel_value = cost_matrix[row_ind, col_ind].sum()

        return kernel_value
