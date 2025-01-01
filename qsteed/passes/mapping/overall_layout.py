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


from qsteed.passes.mapping.baselayout import Layout
from qsteed.graph.couplinggraph import CouplingGraph


class OverallLayout(Layout):
    def __init__(self,
                 coupling_graph: CouplingGraph = None,
                 coupling_list: list = None,
                 num_qubits: int = None,
                 qubits_list: list = None):
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
            if num_qubits != len(qubits_list):
                self.num_qubits = len(qubits_list)
        elif num_qubits is None and qubits_list is None:
            raise ValueError('num_qubits and qubits_list must be given one.')

    def get_graph(self):
        if self.coupling_graph is not None:
            return self.coupling_graph.graph
        elif self.coupling_list is not None:
            self.coupling_graph = CouplingGraph(coupling_list=self.coupling_list)
            return self.coupling_graph.graph
        else:
            raise ValueError('coupling_graph and coupling_list must be given one.')

    def overall_layout(self):
        """Used to layout quantum circuits as a whole to the appropriate area of the chip."""
        pass
