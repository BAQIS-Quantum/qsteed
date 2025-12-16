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

from math import pi
from typing import List

from qsteed.qsteedcpp import (
    CircuitInstruction,
    RYY as RYYGate,
    CX as CXGate,
    RZ as RZGate,
    RX as RXGate,
)

from qsteed.passes.basepass import UnrollPass


class RYYToCNOT(UnrollPass):
    """The RYYToCNOT pass.

    convert RYY gate to {RX, RZ, CNOT}.

    # RYY gate decomposition rule:
    q[0]  ------#------      q[0]  --RX(π/2)----*-----------------*----RX(-π/2)--
            RYY(theta)    ≡                     |                 |
    q[1]  ------#------      q[1]  --RX(π/2)----+----RZ(theta)----+----RX(-π/2)--
    """

    def __init__(self) -> None:
        super().__init__()
        self.parameter_type = 'parameterized_gate'
        self.original = RYYGate.name.lower()
        self.basis = [CXGate.name.lower(), RXGate.name.lower(), RZGate.name.lower()]

    def run(self, op: CircuitInstruction) -> List[CircuitInstruction]:
        rule = []
        if op.name == 'ryy':
            if isinstance(op.paras, list):
                paras = op.paras[0]
            else:
                paras = op.paras
            rule.append(RXGate(pi / 2, op.pos[0]))
            rule.append(RXGate(pi / 2, op.pos[1]))
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(RZGate(paras, op.pos[1]))
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(RXGate(-pi / 2, op.pos[0]))
            rule.append(RXGate(-pi / 2, op.pos[1]))
        else:
            rule.append(op)
        self.rule = rule
        return rule

    # def run(self, circuit: QuantumCircuit) -> QuantumCircuit:
    #     new_circuit = QuantumCircuit(circuit.num)
    #     for op in circuit.gates:
    #         if op.name == 'ryy':
    #             new_circuit.add_gate(RXGate(np.pi/2, op.pos[0]))
    #             new_circuit.add_gate(RXGate(np.pi/2, op.pos[1]))
    #             new_circuit.add_gate(CXGate(op.pos[0], op.pos[1]))
    #             new_circuit.add_gate(RZGate(op.pos[1],op.paras))
    #             new_circuit.add_gate(CXGate(op.pos[0], op.pos[1]))
    #             new_circuit.add_gate(RXGate(-np.pi/2, op.pos[0]))
    #             new_circuit.add_gate(RXGate(-np.pi/2, op.pos[1]))
    #         else:
    #             new_circuit.add_gate(op)
    #     return new_circuit
