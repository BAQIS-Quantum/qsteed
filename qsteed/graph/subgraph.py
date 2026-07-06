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
from itertools import combinations
from typing import Union

import networkx as nx


def is_connected(graph):
    if isinstance(graph, nx.DiGraph):
        return nx.is_strongly_connected(graph)
    else:
        return nx.is_connected(graph)

def valid_subgraph(graph: Union[nx.Graph, nx.DiGraph], num_nodes: int):
    """Find all connected components with the number of nodes greater than or equal to num_nodes.
    Args:
        graph (networkx.Graph): A weighted undirected graph.
        num_nodes (int): The number of nodes in the desired connected subgraph.

    Returns:
        valid_subgraphs (list): all connected components with the number of nodes greater than or equal to num_nodes.
    """
    if num_nodes > graph.number_of_nodes():
        raise ValueError("The graph does not have enough nodes.")

    # connected_components = list(nx.strongly_connected_components(graph))

    if isinstance(graph, nx.DiGraph):
        connected_components = list(nx.strongly_connected_components(graph))
    else:
        connected_components = list(nx.connected_components(graph))

    valid_components = [comp for comp in connected_components if len(comp) >= num_nodes]

    if valid_components:
        valid_subgraphs = []
        for vc in valid_components:
            subgraph = graph.subgraph(vc)
            valid_subgraphs.append(subgraph)
        return valid_subgraphs
    else:
        raise ValueError("No connected component with enough nodes found.")


def random_subgraph(graph: nx.Graph, num_nodes: int):
    """
    Randomly select a connected subgraph with the specified number of nodes.

    Args:
        graph (networkx.Graph): A weighted undirected graph.
        num_nodes (int): The number of nodes in the desired connected subgraph.

    Returns:
        networkx.Graph: A connected subgraph with the specified number of nodes.
    """
    valid_subgraphs = valid_subgraph(graph, num_nodes)

    # Randomly select one connected subgraph.
    selected_subgraph = random.choice(valid_subgraphs)

    # Randomly select a starting node.
    start_node = random.choice(list(selected_subgraph.nodes()))

    # Use Breadth First Search (BFS) to find a connected set of nodes.
    visited = {start_node}
    queue = [start_node]

    while len(visited) < num_nodes and queue:
        current_node = queue.pop(0)
        neighbors = list(graph.neighbors(current_node))
        random.shuffle(neighbors)

        for neighbor in neighbors:
            if neighbor not in visited:
                visited.add(neighbor)
                queue.append(neighbor)
            if len(visited) == num_nodes:
                break

    # Check if enough connected nodes are found.
    if len(visited) < num_nodes:
        raise ValueError("Unable to find a connected subgraph with the requested number of nodes.")

    used_subgraph = graph.subgraph(visited)
    return used_subgraph


def max_weight_subgraph(graph: nx.Graph, num_nodes):
    """
    Find a k-node subgraph with the maximum total edge weight.

    Args:
        graph (networkx.Graph): A weighted undirected graph.
        num_nodes (int): The number of nodes in the desired subgraph.

    Returns:
        used_subgraph: subgraph with the maximum total edge weight.
    """
    valid_subgraphs = valid_subgraph(graph, num_nodes)

    # Initialize the maximum weight and the best node combination
    max_weight = -float('inf')
    best_subgraph_nodes = None

    weight = list(list(graph.edges(data=True))[0][2].keys())[0]

    for vs in valid_subgraphs:
        # Traverse all possible node combinations
        for nodes in combinations(vs.nodes, num_nodes):
            subgraph = vs.subgraph(nodes)
            # Determine whether the subgraph composed of node combinations is connected
            if is_connected(subgraph):
                # Calculate the total edge weight of the subgraph
                sum_weight = sum(data[weight] for u, v, data in subgraph.edges(data=True))

                # Update the maximum weight and best subgraph node
                if sum_weight > max_weight:
                    max_weight = sum_weight
                    best_subgraph_nodes = nodes

    used_subgraph = graph.subgraph(best_subgraph_nodes)

    return used_subgraph


def max_dense_subgraph(graph: nx.Graph, num_nodes):
    """
    Find a k-node most dense subgraph.

    Args:
        graph (networkx.Graph): A weighted undirected graph.
        num_nodes (int): The number of nodes in the desired subgraph.

    Returns:
        used_subgraph: most dense subgraph
    """
    valid_subgraphs = valid_subgraph(graph, num_nodes)

    max_density = 0
    dense_subgraph = None

    for vs in valid_subgraphs:
        # Traverse all possible node combinations
        for nodes in combinations(vs.nodes, num_nodes):
            subgraph = graph.subgraph(nodes)

            # Determine whether the subgraph composed of node combinations is connected
            if is_connected(subgraph):
                # Calculate density
                density = subgraph.number_of_edges() / (
                        num_nodes * (num_nodes - 1) / 2) if num_nodes > 1 else 0

                # Update the densest subgraph
                if density > max_density:
                    max_density = density
                    dense_subgraph = subgraph

    return dense_subgraph
