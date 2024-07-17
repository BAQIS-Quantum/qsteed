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

from qsteed.dag.circuit_dag_convert import circuit_to_dag, dag_to_circuit, draw_dag, nodelist_to_dag, \
    gate_to_node
from tests.shared_utils import get_random_circuit


class TestDAGConversion:

    def test_circuit_to_dag_and_back(self):
        qc = get_random_circuit(gates_number=20)
        assert qc is not None
        qc.draw_circuit()

        dag = circuit_to_dag(qc)
        # draw_dag(dag)  # You can uncomment the lines locally to display the DAG diagram.
        # show_dag(dag)

        re_qc = dag_to_circuit(dag, qubits=qc.num)
        assert re_qc is not None
        re_qc.draw_circuit()
        assert len(qc.gates) == len(re_qc.gates), "Number of gates should be the same"

    def test_nodelist_to_dag(self):
        qc = get_random_circuit(gates_number=20)
        assert qc is not None
        qc.draw_circuit()

        nodes_list = [gate_to_node(gate, specific_label=i) for i, gate in enumerate(qc.gates)]
        dag = nodelist_to_dag(nodes_list)
        # draw_dag(dag)  # You can uncomment the lines locally to display the DAG diagram.
        # show_dag(dag)

        re_qc = dag_to_circuit(dag, qubits=qc.num)
        assert re_qc is not None
        assert len(qc.gates) == len(re_qc.gates), "Number of gates should be the same"
