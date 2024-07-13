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
from quafu.elements.element_gates import PhaseGate
from quafu.elements.element_gates.rotation import RZGate

from qsteed.passes.basepass import UnrollPass


class PhaseToRZ(UnrollPass):
    """The PhaseToRZ pass.
    convert Phase gate to {RZ}.

    Phase gate decomposition rule:
    q[0] ----P(lambda)---- ≡ U3(0, 0, lambda)
                           ≡ q[0] ----RZ(lambda)---- * global_phase(lambda/2)
    """

    def __init__(self) -> None:
        super().__init__()
        self.parameter_type = 'parameterized_gate'
        self.original = PhaseGate(0, 0).name.lower()
        self.basis = [RZGate(0, 0).name.lower()]
        self.global_phase = 'lambda/2'
        # qc = QuantumCircuit(1)
        # qc.rz(0, lambda)
        # self.circuit = qc

    def run(self, op: Instruction) -> List[Instruction]:
        rule = []
        if isinstance(op, PhaseGate):
            if isinstance(op.paras, list):
                paras = op.paras[0]
            else:
                paras = op.paras
            rule.append(RZGate(op.pos[0], paras))
            self.global_phase = paras / 2
        else:
            rule.append(op)
        self.rule = rule
        return rule
