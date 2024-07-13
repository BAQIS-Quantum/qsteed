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
from quafu.elements.element_gates import CRZGate, CXGate, RZGate

from qsteed.passes.basepass import UnrollPass


class CRZToCNOT(UnrollPass):
    """The CRZToBasis pass.

    Convert CRZ gate to {H, CX, RZ}.

    CRZ gate decomposition rule:
    q[0] ----*----         q[0] ---------------*---------------*--
             |         ≡                       |               |
    q[1] ----RZ(θ)----     q[1] ----RZ(θ/2)----+----RZ(-θ/2)---+--

                               
    """

    def __init__(self) -> None:
        super().__init__()
        self.original = CRZGate(0, 1, 0).name.lower()
        self.basis = [CXGate.name.lower(), RZGate(0, 0).name.lower()]

    def run(self, op: Instruction) -> List[Instruction]:
        rule = []
        if isinstance(op, CRZGate):
            theta = op.paras[0]  # Assuming CRZGate takes one parameter θ

            rule.append(RZGate(op.pos[1], theta / 2))
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(RZGate(op.pos[1], -theta / 2))
            rule.append(CXGate(op.pos[0], op.pos[1]))
        else:
            rule.append(op)
        self.rule = rule
        return rule
