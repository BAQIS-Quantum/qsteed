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
from quafu.elements.element_gates import CPGate, CXGate, PhaseGate

from qsteed.passes.basepass import UnrollPass


class CPToCNOT(UnrollPass):
    """The CPToCNOT pass.

    convert CPhase gate to {CNOT, Phase}.

    CP gate decomposition rule:
    q[0]  -----*------     q[0]  --P(theta/2)----*-------------------*----------------
               |        ≡                        |                   |
    q[1]  --P(theta)--     q[1]  ----------------+----P(-theta/2)----+----P(theta/2)--
    """

    def __init__(self) -> None:
        super().__init__()
        self.parameter_type = 'parameterized_gate'
        self.original = CPGate(0, 1, 0).name.lower()
        self.basis = [CXGate.name.lower(), PhaseGate(0, 0).name.lower()]
        # qc = QuantumCircuit(2)
        # qc.p(0, theta/2)
        # qc.cnot(0, 1)
        # qc.p(1, -theta/2)
        # qc.cnot(0, 1)
        # qc.p(1, theta/2)
        # self.circuit = qc

    def run(self, op: Instruction) -> List[Instruction]:
        rule = []
        if isinstance(op, CPGate):
            if isinstance(op.paras, list):
                theta = op.paras[0]
            else:
                theta = op.paras
            rule.append(PhaseGate(op.pos[0], theta / 2))
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(PhaseGate(op.pos[1], -theta / 2))
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(PhaseGate(op.pos[1], theta / 2))
        else:
            rule.append(op)
        self.rule = rule
        return rule

    # def run(self, circuit: QuantumCircuit) -> QuantumCircuit:
    #     new_circuit = QuantumCircuit(circuit.num)
    #     for op in circuit.gates:
    #         if isinstance(op, CPGate):
    #             new_circuit.add_gate(PhaseGate(op.pos[0],theta/2))
    #             new_circuit.add_gate(CXGate(op.pos[0], op.pos[1]))
    #             new_circuit.add_gate(PhaseGate(op.pos[1],-theta/2))
    #             new_circuit.add_gate(CXGate(op.pos[0],op.pos[1]))
    #             new_circuit.add_gate(PhaseGate(op.pos[1],theta/2))
    #         else:
    #             new_circuit.add_gate(op)
    #     return new_circuit
