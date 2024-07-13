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

from typing import List

from quafu.elements import Instruction
from quafu.elements.element_gates import RZZGate, CXGate, RZGate

from qsteed.passes.basepass import UnrollPass


class RZZToCNOT(UnrollPass):
    """The RZZToCNOT pass.
    convert RZZ gate to {CNOT, Rz}.
    """

    # RZZ gate decomposition rule:
    # q[0] ------#------     q[0] ---*-----------------*--
    #       RZZ(theta)    ≡          |                 |
    # q[1] ------#------     q[1] ---+----RZ(theta)----+--

    def __init__(self) -> None:
        super().__init__()
        self.parameter_type = 'parameterized_gate'
        self.original = RZZGate(0, 1, 0).name.lower()
        self.basis = [CXGate.name.lower(), RZGate(0, 0).name.lower()]

        # TODO: After pyquafu has parameterized quantum circuits, the circuit can be generated during initialization.
        # qc = QuantumCircuit(2)
        # qc.cnot(0, 1)
        # qc.rz(0, theta)
        # qc.cnot(0, 1)
        # self.circuit = qc

    def run(self, op: Instruction) -> List[Instruction]:
        rule = []
        if isinstance(op, RZZGate):
            if isinstance(op.paras, list):
                paras = op.paras[0]
            else:
                paras = op.paras
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(RZGate(op.pos[1], paras))
            rule.append(CXGate(op.pos[0], op.pos[1]))
        else:
            rule.append(op)
        self.rule = rule
        return rule

    # def run(self, circuit: QuantumCircuit) -> QuantumCircuit:
    #     new_circuit = QuantumCircuit(circuit.num)
    #     for op in circuit.gates:
    #         if isinstance(op, RZZGate):
    #             new_circuit.add_gate(CXGate(op.pos[0], op.pos[1]))
    #             new_circuit.add_gate(RZGate(op.pos[1],op.paras))
    #             new_circuit.add_gate(CXGate(op.pos[0], op.pos[1]))
    #
    #         else:
    #             new_circuit.add_gate(op)
    #     return new_circuit
