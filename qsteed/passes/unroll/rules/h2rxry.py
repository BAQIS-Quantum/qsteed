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

from qsteed.qsteedcpp import (
    CircuitInstruction,
    H as HGate,
    RY as RYGate,
    RX as RXGate,
)

from qsteed.passes.basepass import UnrollPass


class HToRXRY(UnrollPass):
    """The HToRXRY pass.
    convert H gate to {RX, RY}.

    H gate decomposition rule:
    q[0] ----H---- ≡ q[0] ----RY(π/2)---RX(π)---- * global_phase(π/2)
    """

    def __init__(self) -> None:
        super().__init__()
        self.original = HGate.name.lower()
        self.basis = [RXGate.name.lower(), RYGate.name.lower()]
        self.global_phase = pi / 2

    def run(self, op: CircuitInstruction) -> List[CircuitInstruction]:
        rule = []
        if op.name == 'h':
            if isinstance(op.pos, list):
                pos = op.pos[0]
            else:
                pos = op.pos
            rule.append(RYGate(pi / 2, pos))
            rule.append(RXGate(pi, pos))
        else:
            rule.append(op)
        self.rule = rule
        return rule
