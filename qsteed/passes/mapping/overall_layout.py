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

from qsteed.graph.couplinggraph import CouplingGraph
from qsteed.passes.mapping.baselayout import Layout


class OverallLayout(Layout):
    """Base class for selecting an exact-size physical subgraph and initial layout."""

    def __init__(self, coupling_graph: CouplingGraph = None, coupling_list=None,
                 num_qubits: int = None, qubits_list=None):
        super().__init__()
        self.coupling_graph = coupling_graph
        self.coupling_list = coupling_list
        self.num_qubits = num_qubits
        self.qubits_list = qubits_list

        if num_qubits is None and qubits_list is not None:
            self.num_qubits = len(qubits_list)
        elif num_qubits is not None and qubits_list is None:
            self.qubits_list = list(range(num_qubits))
        elif num_qubits is not None and qubits_list is not None:
            self.num_qubits = len(qubits_list)
        else:
            raise ValueError('num_qubits and qubits_list must be given one.')

    def get_graph(self):
        if self.coupling_graph is not None:
            return self.coupling_graph.graph
        if self.coupling_list is not None:
            self.coupling_graph = CouplingGraph(coupling_list=self.coupling_list)
            return self.coupling_graph.graph
        raise ValueError('coupling_graph and coupling_list must be given one.')

    def _set_mapping_from_subgraph(self, subgraph):
        physical_qubits = list(subgraph.nodes())
        if len(physical_qubits) != self.num_qubits:
            raise ValueError('Selected subgraph size does not match the circuit qubit count.')
        self.v2p = dict(zip(self.qubits_list, physical_qubits))
        self.p2v = {physical: virtual for virtual, physical in self.v2p.items()}
        return subgraph
