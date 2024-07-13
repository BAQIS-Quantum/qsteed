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
from collections import defaultdict
from math import pi

from quafu import QuantumCircuit
from quafu.elements.element_gates import QuantumGate
from quafu.elements.element_gates import pauli


class RandomCircuit:

    def __init__(self,
                 num_qubit: int = 4,
                 depth: int = None,
                 gates_list: list = None,
                 gates_number: int = None,
                 max_qubit: int = 3,
                 measure: bool = True):
        self.num_qubit = num_qubit
        self.depth = depth
        self.gates_list = gates_list
        self.gates_number = gates_number
        self.max_qubit = max_qubit
        self.measure = measure
        self._circuit = None

    def add_gate(self, gates_group, qubits, used_qubits):
        gate = random.choice(self.gates_list)
        if gate.lower() in gates_group['1-qubit-constant']:
            pos = random.choice(qubits)
            self._circuit.add_gate(QuantumGate.gate_classes[gate](pos))
            used_qubits.add(pos)
        elif gate.lower() in gates_group['1-qubit-parameterized']:
            if gate.lower() != 'u3':
                pos = random.choice(qubits)
                para = random.uniform(0, 2 * pi)
                self._circuit.add_gate(QuantumGate.gate_classes[gate](pos, para))
                used_qubits.add(pos)
            else:
                pos = random.choice(qubits)
                para = [random.uniform(0, 2 * pi) for _ in range(3)]
                self._circuit.add_gate(QuantumGate.gate_classes[gate](pos, para[0], para[1], para[2]))
                used_qubits.add(pos)
        elif gate.lower() in gates_group['2-qubit-constant']:
            pos = random.sample(qubits, 2)
            self._circuit.add_gate(QuantumGate.gate_classes[gate](pos[0], pos[1]))
            [used_qubits.add(i) for i in pos]
        elif gate.lower() in gates_group['2-qubit-parameterized']:
            pos = random.sample(qubits, 2)
            para = random.uniform(0, 2 * pi)
            self._circuit.add_gate(QuantumGate.gate_classes[gate](pos[0], pos[1], para))
            [used_qubits.add(i) for i in pos]
        elif gate.lower() in gates_group['3-qubit-constant']:
            pos = random.sample(qubits, 3)
            self._circuit.add_gate(QuantumGate.gate_classes[gate](pos[0], pos[1], pos[2]))
            [used_qubits.add(i) for i in pos]
        elif gate.lower() in gates_group['multi-qubit-constant']:
            if gate.lower() in ['mcx', 'mcy', 'mcz']:
                pos = random.sample(qubits, self.max_qubit)
                self._circuit.add_gate(QuantumGate.gate_classes[gate](pos[1:], pos[0]))
                [used_qubits.add(i) for i in pos]
            elif gate.lower() in ['mcrx', 'mcry', 'mcrz']:
                pos = random.sample(qubits, self.max_qubit)
                para = random.uniform(0, 2 * pi)
                self._circuit.add_gate(QuantumGate.gate_classes[gate](pos[1:], pos[0], para))
                [used_qubits.add(i) for i in pos]
        else:
            raise NameError("Error: %s gate does not support." % (gate.lower()))

    def random_circuit_gates(self) -> QuantumCircuit:
        if self.gates_list is None:
            self.gates_list = list(QuantumGate.gate_classes.keys())
        gates_group = self.gates_group

        qubits = [q for q in range(self.num_qubit)]
        self._circuit = QuantumCircuit(self.num_qubit)
        used_qubits = set()

        for _ in range(self.gates_number):
            self.add_gate(gates_group, qubits, used_qubits)

        unused_qubits = list(set(qubits) - set(used_qubits))
        if unused_qubits:  # If the qubit is not used, add the IdGate gate.
            for pos in unused_qubits:
                self._circuit.add_gate(pauli.IdGate(pos))
        return self._circuit

    def random_circuit_depth(self) -> QuantumCircuit:
        if self.gates_list is None:
            self.gates_list = list(QuantumGate.gate_classes.keys())

        gates_group = self.gates_group

        qubits = [q for q in range(self.num_qubit)]
        self._circuit = QuantumCircuit(self.num_qubit)
        used_qubits = set()

        depth = 0
        while depth < self.depth:
            self.add_gate(gates_group, qubits, used_qubits)
            depth = len(self._circuit.layered_circuit()[0]) - 1

        unused_qubits = list(set(qubits) - set(used_qubits))
        if unused_qubits:  # If the qubit is not used, add the IdGate gate.
            for pos in unused_qubits:
                self._circuit.add_gate(pauli.IdGate(pos))
        return self._circuit

    def random_circuit(self):
        if self.depth is not None and self.gates_number is None:
            circuit = self.random_circuit_depth()
        elif self.depth is None and self.gates_number is not None:
            circuit = self.random_circuit_gates()
        elif self.depth is not None and self.gates_number is not None:
            circuit = self.random_circuit_gates()
        else:
            raise ValueError("At least one of depth and gates_number must be specified.")
        if self.measure:
            circuit.measure([q for q in range(self.num_qubit)], [q for q in range(self.num_qubit)])
        return circuit

    @property
    def gates_group(self):
        _gates_group = defaultdict(list)
        for gate_name in list(QuantumGate.gate_classes.keys()):
            if gate_name in ['id', 'x', 'y', 'z', 's', 'sdg', 't', 'tdg', 'sx', 'sxdg', 'sy', 'sydg', 'h',
                             'w', 'sw', 'swdg']:
                _gates_group['1-qubit-constant'].append(gate_name)
            elif gate_name in ['rx', 'ry', 'rz', 'u3', 'p']:
                _gates_group['1-qubit-parameterized'].append(gate_name)
            elif gate_name in ['rxx', 'ryy', 'rzz', 'cp', 'crx', 'cry', 'crz']:
                _gates_group['2-qubit-parameterized'].append(gate_name)
            elif gate_name in ['cx', 'cy', 'cz', 'cs', 'ct', 'swap', 'iswap', 'cnot']:
                _gates_group['2-qubit-constant'].append(gate_name)
            elif gate_name in ['ccx', 'cswap', 'toffoli', 'fredon']:
                _gates_group['3-qubit-constant'].append(gate_name)
            elif gate_name in ['mcx', 'mcy', 'mcz', 'mcrx', 'mcry', 'mcrz']:
                _gates_group['multi-qubit-constant'].append(gate_name)
        return _gates_group

# tests
# rqc = RandomCircuit(num_qubit=4, depth=6, gates_number=20, max_qubit=2, gates_list=['cswap'])
# qc = rqc.random_circuit()
# qc.draw_circuit()
