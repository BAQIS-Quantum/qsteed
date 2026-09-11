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
from quafu.elements.element_gates import HGate, RXGate, RYGate

from qsteed.passes.basepass import UnrollPass


class HToRXRY(UnrollPass):
    """Convert an H gate to RY(pi/2), RX(pi), up to a global phase."""

    def __init__(self) -> None:
        super().__init__()
        self.original = HGate.name.lower()
        self.basis = [RXGate(0, 0).name.lower(), RYGate(0, 0).name.lower()]
        self.global_phase = pi / 2

    def run(self, op: Instruction) -> List[Instruction]:
        rule = []
        if isinstance(op, HGate):
            pos = op.pos[0] if isinstance(op.pos, list) else op.pos
            rule.append(RYGate(pos, pi / 2))
            rule.append(RXGate(pos, pi))
        else:
            rule.append(op)
        self.rule = rule
        return rule
