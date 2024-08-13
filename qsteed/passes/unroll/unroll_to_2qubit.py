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

from quafu import QuantumCircuit
from quafu.elements import Barrier, Delay, XYResonance, Measure

from qsteed.passes.basepass import BasePass
from qsteed.passes.unroll.rules_library import Rules_dict


class UnrollTo2Qubit(BasePass):
    """Unroll a circuit to two-qubit gates.
    """

    def __init__(self):
        """Initialize UnrollTo2Qubit
        """
        super().__init__()
        self.quantum_element = [Barrier.name, Delay.name, XYResonance.name, Measure.name]
        self.gate_run_limit = 8
        self.qubits = None
        self.global_phase = 0

    def run(self, circuit: QuantumCircuit) -> QuantumCircuit:
        gates = circuit.gates
        self.qubits = circuit.num
        new_circuit = QuantumCircuit(circuit.num)
        for op in gates:
            if (op.name.lower() in self.quantum_element) or (get_length(op.pos) <= 2):
                new_circuit.add_gate(op)
            else:
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
            # based on 1-qubit and 2-qubit gates set has been found among all rules.
            raise TypeError("The instruction %s cannot be unrolled to 2-qubit gates." % (
                current_op.name.lower()))

        if (gate.name.lower() in self.quantum_element) or (get_length(gate.pos) <= 2):
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
            rule_class = Rules_dict[gate.name.lower()][1]
            rule = rule_class.run(gate)
            self.global_phase += rule_class.global_phase
            for gate in rule:
                new_circuit = self._apply_gate_rules(current_op, gate, new_circuit, depth=depth - 1)
            return new_circuit


def get_length(obj):
    if isinstance(obj, list) or isinstance(obj, dict):
        return len(obj)
    elif isinstance(obj, int):
        return 1
    else:
        return None
