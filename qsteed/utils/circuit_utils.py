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

"""
Circuit utilities for compatibility with Quafu API
"""

import numpy as np
from qsteed.qsteedcpp import QuantumCircuit


def layered_circuit(self) -> np.ndarray:
    """
    Make layered circuit from the gate sequence self.gates.

    This method converts the linear gate sequence into a 2D layered representation
    where each row represents a qubit and each column represents a time layer.

    Returns:
        np.ndarray: A layered array with shape (num_used_qubits, max_depth + 1)
                   First column contains qubit indices, remaining columns contain gates or None.
    """
    num = self.num_qubits()

    gatelist = self.gates

    gateQlist = [[] for _ in range(num)]
    used_qubits = []

    for gate in gatelist:
        # Skip non-gate instructions
        if not gate.is_gate():
            continue

        qubits = gate.qubits

        if len(qubits) == 1:
            # Single qubit gate (including Delay)
            pos = qubits[0]
            gateQlist[pos].append(gate)
            if pos not in used_qubits:
                used_qubits.append(pos)
        else:
            # Multi-qubit gate
            pos1 = min(qubits)
            pos2 = max(qubits)

            # Add gate to the first qubit
            gateQlist[pos1].append(gate)

            # Add None placeholders to other qubits involved
            for j in range(pos1 + 1, pos2 + 1):
                gateQlist[j].append(None)

            # Track used qubits
            for pos in qubits:
                if pos not in used_qubits:
                    used_qubits.append(pos)

            # Align all involved qubits to the same layer
            maxlayer = max([len(gateQlist[j]) for j in range(pos1, pos2 + 1)])
            for j in range(pos1, pos2 + 1):
                layerj = len(gateQlist[j])
                if layerj < maxlayer:
                    pos = layerj - 1
                    for _ in range(maxlayer - layerj):
                        gateQlist[j].insert(pos, None)

    # Add qubits used in measurements
    for m in self.measures.keys():
        if m not in used_qubits:
            used_qubits.append(m)

    # Sort used qubits
    used_qubits = sorted(used_qubits)

    # Pad all qubit layers to the same depth
    if len(used_qubits) > 0:
        maxdepth = max([len(gateQlist[i]) for i in range(num)]) if max([len(gateQlist[i]) for i in range(num)], default=0) > 0 else 0
    else:
        maxdepth = 0

    for gates in gateQlist:
        gates.extend([None] * (maxdepth - len(gates)))

    # Filter to only used qubits
    new_gateQlist = []
    for old_qi in range(len(gateQlist)):
        gates = gateQlist[old_qi]
        if old_qi in used_qubits:
            new_gateQlist.append(gates)

    # Convert to numpy array and add qubit indices as first column
    if len(new_gateQlist) > 0:
        lc = np.array(new_gateQlist, dtype=object)
        lc = np.vstack((used_qubits, lc.T)).T
    else:
        lc = np.array([[]], dtype=object)

    return lc


def attach_layered_circuit_method():
    """Attach the layered_circuit method to QuantumCircuit class"""
    QuantumCircuit.layered_circuit = layered_circuit


__all__ = ['layered_circuit', 'attach_layered_circuit_method']