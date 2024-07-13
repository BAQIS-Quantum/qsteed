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
from quafu.elements.element_gates import CSGate, CXGate, PhaseGate

from qsteed.passes.basepass import UnrollPass


class CSToCNOT(UnrollPass):
    """The CSToCNOT pass.

    convert CS gate to {CNOT, Phase}.

    CS gate decomposition rule:
    q[0]  -----*------     q[0]  --P(π/4)----*---------------*-------------
               |        ≡                    |               |
    q[1]  ---P(π/2)---     q[1]  ------------+----P(-π/4)----+----P(π/4)---
    """

    def __init__(self) -> None:
        super().__init__()
        self.original = CSGate.name.lower()
        self.basis = [CXGate.name.lower(), PhaseGate(0, 0).name.lower()]

    def run(self, op: Instruction) -> List[Instruction]:
        rule = []
        if isinstance(op, CSGate):
            rule.append(PhaseGate(op.pos[0], pi / 4))
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(PhaseGate(op.pos[1], -pi / 4))
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(PhaseGate(op.pos[1], pi / 4))
        else:
            rule.append(op)
        self.rule = rule
        return rule
