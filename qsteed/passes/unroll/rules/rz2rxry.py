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

from quafu.elements import Instruction
from quafu.elements.element_gates import RXGate, RYGate, RZGate

from qsteed.passes.basepass import UnrollPass


class RZToRXRY(UnrollPass):
    """The RZToRXRY pass.
    convert RZ gate to {RY, RX}.

    H gate decomposition rule:
    q[0] ----RZ(theta)---- ≡ q[0] ----RX(π/2)---RY(-theta)---RX(-π/2)----
    """

    def __init__(self) -> None:
        super().__init__()
        self.original = RZGate(0, 0).name.lower()
        self.basis = [RYGate(0, 0).name.lower(), RXGate(0, 0).name.lower()]

    def run(self, op: Instruction) -> List[Instruction]:
        rule = []
        if isinstance(op, RZGate):
            if isinstance(op.pos, list):
                pos = op.pos[0]
            else:
                pos = op.pos
            if isinstance(op.paras, list):
                paras = op.paras[0]
            else:
                paras = op.paras
            rule.append(RXGate(pos, pi / 2))
            rule.append(RYGate(pos, -paras))
            rule.append(RXGate(pos, -pi / 2))
        else:
            rule.append(op)
        self.rule = rule
        return rule
