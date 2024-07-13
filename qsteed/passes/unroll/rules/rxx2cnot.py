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
from quafu.elements.element_gates import RXXGate, CXGate, RZGate, HGate

from qsteed.passes.basepass import UnrollPass


class RXXToCNOT(UnrollPass):
    """The RXXToCNOT pass.
    convert RXX gate to {CNOT, H, Rz}.

    # RXX gate decomposition rule:
    q[0]  ------#-------     q[0]  --H----*-----------------*----H--
            RXX(theta)    ≡               |                 |
    q[1]  ------#-------     q[1]  --H----+----RZ(theta)----+----H--
    """

    def __init__(self) -> None:
        super().__init__()
        self.parameter_type = 'parameterized_gate'
        self.original = RXXGate(0, 1, 0).name.lower()
        self.basis = [CXGate.name.lower(), RZGate(0, 0).name.lower(), HGate.name.lower()]

    def run(self, op: Instruction) -> List[Instruction]:
        rule = []
        if isinstance(op, RXXGate):
            if isinstance(op.paras, list):
                paras = op.paras[0]
            else:
                paras = op.paras
            rule.append(HGate(op.pos[0]))
            rule.append(HGate(op.pos[1]))
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(RZGate(op.pos[1], paras))
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(HGate(op.pos[0]))
            rule.append(HGate(op.pos[1]))
        else:
            rule.append(op)
        self.rule = rule
        return rule

    # def run(self, circuit: QuantumCircuit) -> QuantumCircuit:
    #     new_circuit = QuantumCircuit(circuit.num)
    #     for op in circuit.gates:
    #         if isinstance(op, RXXGate):
    #             new_circuit.add_gate(HGate(op.pos[0]))
    #             new_circuit.add_gate(HGate(op.pos[1]))
    #             new_circuit.add_gate(CXGate(op.pos[0], op.pos[1]))
    #             new_circuit.add_gate(RZGate(op.pos[1], op.paras))
    #             new_circuit.add_gate(CXGate(op.pos[0], op.pos[1]))
    #             new_circuit.add_gate(HGate(op.pos[0]))
    #             new_circuit.add_gate(HGate(op.pos[1]))
    #         else:
    #             new_circuit.add_gate(op)
    #     return new_circuit
