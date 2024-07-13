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

import numpy as np
from quafu import QuantumCircuit

from qsteed.passes.decomposition.ZYZ_decompose import zyz_decomposition
from qsteed.passes.decomposition.utils.matrix_utils import general_kron, general_CNOT


def gates_list_to_circuit(gates_list, nqubit):
    """ Convert gates_list to circuit.

    Args:
        gates_list (list): UnitaryDecompose.gates_list in decomposition.py
        nqubit (int): number of qubits

    Returns:
        circuit: pyquafu QuantumCircuit class
    """
    circuit = QuantumCircuit(nqubit)
    for g in gates_list:
        if g[2] == 'CX':
            circuit.cnot(g[1][0], g[1][1])
        elif g[2] == 'RY':
            circuit.ry(g[1][0], g[3].real)
        elif g[2] == 'RZ':
            circuit.rz(g[1][0], g[3].real)
        elif g[2] == 'U':
            gamma, beta, alpha, global_phase = zyz_decomposition(g[0])
            circuit.rz(g[1][0], gamma)
            circuit.ry(g[1][0], beta)
            circuit.rz(g[1][0], alpha)
        else:
            pass
    return circuit


def gates_list_to_unitary(num_qubit, gates_list):
    """ Convert gates_list to unitary.

    Args:
        num_qubit (int): qubits number of circuit
        gates_list (list): UnitaryDecompose.gates_list in decomposition.py

    Returns:
        unitary: np.array
    """
    unitary_list = []
    for g in gates_list:
        operator = g[0]
        inds = g[1]
        # print(operator, len(operator[::]))
        # assert len(operator) == 2 ** len(inds)

        if len(inds) == 1:
            Ag = general_kron(operator, inds[0], num_qubit)
        elif len(inds) == 2:
            Ag = general_CNOT(num_qubit, inds[0], inds[1])
        else:
            raise (ValueError("The gate must be one/two qubit gate"))
        unitary_list.append(Ag)

    nsize = 2 ** num_qubit
    unitary = np.eye(nsize)
    for u in unitary_list[::-1]:
        unitary = unitary @ u

    return unitary


def circuit_to_unitary(circuit):
    """ Convert gates_list to unitary.

    Args:
        circuit (QuantumCircuit): pyquafu QuantumCircuit class

    Returns:
        unitary: np.array
    """
    num_qubit = circuit.num
    layered_gates = circuit.layered_circuit().T[1:]
    unitary_list = []
    # reverse circuit
    for gates in layered_gates[::-1]:
        for g in gates:
            if g:
                if isinstance(g.pos, int):
                    unit = general_kron(g.matrix, g.pos, num_qubit)
                elif isinstance(g.pos, list) and len(g.pos) == 1:
                    unit = general_kron(g.matrix, g.pos[0], num_qubit)
                elif isinstance(g.pos, list) and g.name == "CX":
                    unit = general_CNOT(num_qubit, g.pos[0], g.pos[1])
                else:
                    raise (ValueError("The gate must be one-qubit gate or CNOT gate."))
                unitary_list.append(unit)

    matrix_size = 2 ** num_qubit
    unitary = np.eye(matrix_size)
    for u in unitary_list:
        unitary = unitary @ u

    return unitary


def get_circuit_depth(num_qubit, gates_list):
    cnot_count = 0
    gate_count_in_each_qubit = np.zeros(num_qubit)
    for g in gates_list:
        operator, inds = g[0], g[1]

        if len(inds) == 2:
            cnot_count += 1

        for ind in inds:
            gate_count_in_each_qubit[ind] += 1

    assert len(gates_list) == (int(sum(gate_count_in_each_qubit)) - cnot_count)
    circuit_depth = int(max(gate_count_in_each_qubit))
    return cnot_count, circuit_depth
