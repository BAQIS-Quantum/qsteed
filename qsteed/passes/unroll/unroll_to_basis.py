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

import copy
from math import pi
from typing import List

from quafu import QuantumCircuit
from quafu.elements import Barrier, Delay, XYResonance, Measure
from quafu.elements.element_gates import CXGate, RXGate, RYGate, RZGate, IdGate, CPGate, CZGate, ISwapGate

from qsteed.passes.basepass import BasePass
from qsteed.passes.unroll.rules_library import Rules_dict, CX_rules


class UnrollToBasis(BasePass):
    """Unroll a circuit to a given basis gates.
    """

    def __init__(self, basis_gates: List[str] = None):
        """Initialize UnrollToBasis

        Args:
            basis_gates (list): e.g. ['cx','rx','ry','rz','id']
        """
        super().__init__()
        self.quantum_element = [Barrier.name, Delay.name, XYResonance.name, Measure.name]
        if basis_gates is None:
            self.basis_gates = [CXGate.name, RXGate.name, RYGate.name, RZGate.name, IdGate.name]
            # self.basis_gates = ['CX', 'RX', 'RY', 'RZ', 'Id']
        else:
            self.basis_gates = basis_gates
        self.basis_gates = self.basis_gates + self.quantum_element
        self.basis_gates = [gate.lower() for gate in self.basis_gates]
        self.gate_run_limit = 8
        self.qubits = None
        self.global_phase = 0

        if CZGate.name.lower() in self.basis_gates:
            Rules_dict.update(CX_rules[CZGate.name.lower()])
        elif ISwapGate.name.lower() in self.basis_gates:
            Rules_dict.update(CX_rules[ISwapGate.name.lower()])
        elif CPGate(0, 1, 0).name.lower() in self.basis_gates:
            Rules_dict.update(CX_rules[CPGate(0, 1, 0).name.lower()])

    def run(self, circuit: QuantumCircuit) -> QuantumCircuit:
        gates = circuit.gates
        self.qubits = circuit.num
        new_circuit = QuantumCircuit(circuit.num)
        for op in gates:
            current_op = copy.deepcopy(op)
            new_circuit = self._apply_gate_rules(current_op, op, new_circuit=new_circuit, depth=self.gate_run_limit)
        [new_circuit.measure([q], [c]) for q, c in circuit.measures.items()]
        self.global_phase = self.global_phase % (2 * pi)
        return new_circuit

    def _apply_gate_rules(self, current_op, gate, new_circuit=None, depth=5):
        if new_circuit is None:
            new_circuit = QuantumCircuit(self.qubits)

        if depth <= 0:
            # Exceeding the number of recursions indicates that no decomposition of this gate
            # based on the given basis gates set has been found among all rules.
            raise ValueError("The instruction %s cannot be unrolled to the given basis gates set %s." % (
                current_op.name.lower(), self.basis_gates))

        if gate.name.lower() in self.basis_gates:
            new_circuit.add_gate(gate)
            return new_circuit
        elif gate.name.lower() == 'unitary':
            # TODO: Currently quafu has no definition of unitary gate.
            pass
        else:
            try:
                Rules_dict[gate.name.lower()][0]
            except KeyError:
                print("Error: The instruction %s has no unrolling rule." % gate.name)
            basis = Rules_dict[gate.name.lower()][0]
            rule_class = Rules_dict[gate.name.lower()][1]
            rule = rule_class.run(gate)
            self.global_phase += rule_class.global_phase
            if set(basis).issubset(set(self.basis_gates)):
                [new_circuit.add_gate(g) for g in rule]
                return new_circuit
            else:
                for gate in rule:
                    new_circuit = self._apply_gate_rules(current_op, gate, new_circuit, depth=depth - 1)
                return new_circuit
