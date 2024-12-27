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
from qsteed.graph.subgraph import random_subgraph
from qsteed.passes.mapping.create_layout import CreateLayout


class InitialLayoutRandom(CreateLayout):
    def __init__(self,
                 coupling_graph: CouplingGraph = None,
                 coupling_list: list = None,
                 num_qubits: int = None,
                 qubits_list: list = None
                 ):
        super().__init__(coupling_graph, coupling_list, num_qubits, qubits_list)

    def create_layout(self):
        """

        Returns:

        """
        graph = self.get_graph()
        subgraph = random_subgraph(graph=graph, num_nodes=self.num_qubits)
        phys_qubits_list = list(subgraph.nodes())
        self.v2p = dict(zip(self.qubits_list, phys_qubits_list))
        self.p2v = {p: v for v, p in self.v2p.items()}
        return subgraph
