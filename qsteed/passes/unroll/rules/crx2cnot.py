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
from quafu.elements.element_gates import CRXGate, CXGate, SGate, RYGate, SdgGate

from qsteed.passes.basepass import UnrollPass


class CRXToCNOT(UnrollPass):
    """The CRXToBasis pass.

    Convert CRX gate to {H, CX, RX}.

    CRX gate decomposition rule:
    q[0] ----*----         q[0] -------*----------------*---------------- 
             |      ≡                  |                |
    q[1] ----RX(θ)----     q[1] --S----+----RY(-θ/2)----+--RY(θ/2)----Sdg--

                               
    """

    def __init__(self) -> None:
        super().__init__()
        self.original = CRXGate(0, 1, 0).name.lower()
        self.basis = [CXGate.name.lower(), SGate.name.lower(), SdgGate.name.lower(), RYGate(0, 0).name.lower()]

    def run(self, op: Instruction) -> List[Instruction]:
        rule = []
        if isinstance(op, CRXGate):
            theta = op.paras[0]  # Assuming CRZGate takes one parameter θ

            rule.append(SGate(op.pos[1]))
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(RYGate(op.pos[1], -theta / 2))
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(RYGate(op.pos[1], theta / 2))
            rule.append(SdgGate(op.pos[1]))
        else:
            rule.append(op)
        self.rule = rule
        return rule
