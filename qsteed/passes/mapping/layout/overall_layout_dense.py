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

from qsteed.graph.subgraph import max_dense_subgraph
from qsteed.passes.mapping.overall_layout import OverallLayout


class OverallLayoutDense(OverallLayout):
    """Choose the connected physical subgraph with greatest edge density."""

    def overall_layout(self):
        subgraph = max_dense_subgraph(self.get_graph(), self.num_qubits)
        return self._set_mapping_from_subgraph(subgraph)
