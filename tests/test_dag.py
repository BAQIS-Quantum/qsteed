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

from qsteed.dag.circuit_dag_convert import circuit_to_dag, dag_to_circuit, draw_dag, show_dag, nodelist_to_dag, gate_to_node, nodelist_qubit_mapping_dict
from qsteed.utils.random_circuit import RandomCircuit
import matplotlib.pyplot as plt

rqc = RandomCircuit(num_qubit=5, gates_number=20, gates_list=['mcx', 'cx', 'ry', 'h'],  measure=False)
qc = rqc.random_circuit()
qc.plot_circuit()
plt.show()

dag = circuit_to_dag(qc)
draw_dag(dag)
show_dag(dag)

re_qc = dag_to_circuit(dag, qubits=5)
re_qc.plot_circuit()
plt.show()

nodelist = [gate_to_node(gate, specific_label=i) for i, gate in enumerate(qc.gates)]
dag1 = nodelist_to_dag(nodelist)
draw_dag(dag1)
show_dag(dag1)
