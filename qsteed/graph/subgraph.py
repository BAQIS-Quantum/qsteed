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
    """Return whether ``graph`` is connected in the routing direction."""
    if graph.number_of_nodes() == 0:
        return False
    if isinstance(graph, nx.DiGraph):
        return nx.is_strongly_connected(graph)
    return nx.is_connected(graph)


def valid_subgraphs(graph: Union[nx.Graph, nx.DiGraph], num_nodes: int):
    """Return connected components large enough to contain ``num_nodes`` nodes."""
    if num_nodes < 1:
        raise ValueError('The requested subgraph must contain at least one node.')
    if num_nodes > graph.number_of_nodes():
        raise ValueError('The graph does not have enough nodes.')

    if isinstance(graph, nx.DiGraph):
        components = nx.strongly_connected_components(graph)
    else:
        components = nx.connected_components(graph)
    result = [graph.subgraph(component) for component in components if len(component) >= num_nodes]
    if not result:
        raise ValueError('No connected component with enough nodes found.')
    return result


def random_subgraph(graph: Union[nx.Graph, nx.DiGraph], num_nodes: int):
    """Randomly select an induced connected subgraph with exactly ``num_nodes`` nodes."""
    component = random.choice(valid_subgraphs(graph, num_nodes))
    start_node = random.choice(list(component.nodes()))
    visited = {start_node}
    queue = [start_node]

    while len(visited) < num_nodes and queue:
        current_node = queue.pop(0)
        neighbors = list(component.neighbors(current_node))
        random.shuffle(neighbors)
        for neighbor in neighbors:
            if neighbor not in visited:
                visited.add(neighbor)
                queue.append(neighbor)
            if len(visited) == num_nodes:
                break

    selected = graph.subgraph(visited)
    if selected.number_of_nodes() != num_nodes or not is_connected(selected):
        raise ValueError('Unable to find a connected subgraph with the requested number of nodes.')
    return selected


def max_weight_subgraph(graph: Union[nx.Graph, nx.DiGraph], num_nodes: int):
    """Find a connected ``num_nodes`` subgraph with maximum total edge fidelity."""
    best_weight = -float('inf')
    best_subgraph = None
    for component in valid_subgraphs(graph, num_nodes):
        for nodes in combinations(component.nodes, num_nodes):
            candidate = graph.subgraph(nodes)
            if is_connected(candidate):
                total_weight = sum(
                    data.get('fidelity', 1.0)
                    for _, _, data in candidate.edges(data=True)
                )
                if total_weight > best_weight:
                    best_weight = total_weight
                    best_subgraph = candidate
    if best_subgraph is None:
        raise ValueError('Unable to find a connected subgraph with the requested number of nodes.')
    return best_subgraph


def max_dense_subgraph(graph: Union[nx.Graph, nx.DiGraph], num_nodes: int):
    """Find a connected ``num_nodes`` subgraph with maximum edge density."""
    best_density = -1
    best_subgraph = None
    for component in valid_subgraphs(graph, num_nodes):
        for nodes in combinations(component.nodes, num_nodes):
            candidate = graph.subgraph(nodes)
            if is_connected(candidate):
                density = nx.density(candidate)
                if density > best_density:
                    best_density = density
                    best_subgraph = candidate
    if best_subgraph is None:
        raise ValueError('Unable to find a connected subgraph with the requested number of nodes.')
    return best_subgraph
