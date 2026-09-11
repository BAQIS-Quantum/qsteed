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
from quafu.elements.element_gates import CZGate, RXGate, RYGate, SwapGate

from qsteed.passes.basepass import UnrollPass


class SwapToCZRXRY(UnrollPass):
    """Convert a SWAP gate to CZ, RX and RY gates, up to a global phase."""

    def __init__(self) -> None:
        super().__init__()
        self.original = SwapGate.name.lower()
        self.basis = [CZGate.name.lower(), RXGate(0, 0).name.lower(), RYGate(0, 0).name.lower()]
        self.global_phase = pi

    @staticmethod
    def _h_as_rx_ry(pos):
        return [RYGate(pos, pi / 2), RXGate(pos, pi)]

    def run(self, op: Instruction) -> List[Instruction]:
        rule = []
        if isinstance(op, SwapGate):
            q0, q1 = op.pos
            rule.extend(self._h_as_rx_ry(q0))
            rule.append(CZGate(q0, q1))
            rule.extend(self._h_as_rx_ry(q0))
            rule.extend(self._h_as_rx_ry(q1))
            rule.append(CZGate(q0, q1))
            rule.extend(self._h_as_rx_ry(q0))
            rule.extend(self._h_as_rx_ry(q1))
            rule.append(CZGate(q0, q1))
            rule.extend(self._h_as_rx_ry(q0))
        else:
            rule.append(op)
        self.rule = rule
        return rule
