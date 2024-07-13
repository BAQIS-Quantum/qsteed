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
from quafu.elements.element_gates import ISwapGate, CXGate, HGate, SGate

from qsteed.passes.basepass import UnrollPass


class ISWAPToCNOT(UnrollPass):
    """The ISWAPToCNOT pass.

    convert ISWAP gate to {S, H, CNOT}.

    ISWAP gate decomposition rule:
    q[0] ----#----     q[0] --S----H----*----+-------
           iSWAP    ≡                   |    |
    q[1] ----#----     q[1] --S---------+----*----H--
    """

    def __init__(self) -> None:
        super().__init__()
        self.original = ISwapGate.name.lower()
        self.basis = [SGate.name.lower(), HGate.name.lower(), CXGate.name.lower()]
        # qc = QuantumCircuit(2)
        # qc.s(0)
        # qc.s(1)
        # qc.h(0)
        # qc.cnot(0,1)
        # qc.cnot(1,0)
        # qc.h(1)
        # self.circuit = qc

    def run(self, op: Instruction) -> List[Instruction]:
        rule = []
        if isinstance(op, ISwapGate):
            rule.append(SGate(op.pos[0]))
            rule.append(SGate(op.pos[1]))
            rule.append(HGate(op.pos[0]))
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(CXGate(op.pos[1], op.pos[0]))
            rule.append(HGate(op.pos[1]))
        else:
            rule.append(op)
        self.rule = rule
        return rule

    # def run(self, circuit: QuantumCircuit) -> QuantumCircuit:
    #     new_circuit = QuantumCircuit(circuit.num)
    #     for op in circuit.gates:
    #         if isinstance(op, ISwapGate):
    #             new_circuit.add_gate(SGate(op.pos[0]))
    #             new_circuit.add_gate(SGate(op.pos[1]))
    #             new_circuit.add_gate(HGate(op.pos[0]))
    #             new_circuit.add_gate(CXGate(op.pos[0], op.pos[1]))
    #             new_circuit.add_gate(CXGate(op.pos[1], op.pos[0]))
    #             new_circuit.add_gate(HGate(op.pos[1]))
    #         else:
    #             new_circuit.add_gate(op)
    #     return new_circuit
