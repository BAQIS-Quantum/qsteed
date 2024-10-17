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


def virtual_qubits(substructure):
    """
    Reset substructure, qubits start from 0.

    Args:
        substructure (List): qubits coupling substructure e.g.[[3,5,0.98],[5,3,0.99],...]

    Returns:
        coupling_list (List): [[0,1,0.98],[1,0,0.99],...]
        vq_to_q (Dict): {0: 3, 1: 5,...}
    """
    qubits_list = []
    for coupling in substructure:
        if coupling[0] not in qubits_list:
            qubits_list.append(coupling[0])
        if coupling[1] not in qubits_list:
            qubits_list.append(coupling[1])
    qubits_list = sorted(qubits_list)
    vq_to_q = {q: qubits_list[q] for q in range(len(qubits_list))}
    q_to_vq = {qubits_list[q]: q for q in range(len(qubits_list))}
    coupling_list = []
    for coupling in substructure:
        coupling_list.append([q_to_vq[coupling[0]], q_to_vq[coupling[1]], coupling[2]])
    return coupling_list, vq_to_q
