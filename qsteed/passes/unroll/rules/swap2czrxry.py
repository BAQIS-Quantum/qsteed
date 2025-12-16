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
from math import pi

from qsteed.qsteedcpp import (
    CircuitInstruction,
    Swap as SwapGate,
    CZ as CZGate,
    RY as RYGate,
    RX as RXGate,
)

from qsteed.passes.basepass import UnrollPass


class SwapToCZRXRY(UnrollPass):
    """The SwapToCZRXRY pass.
    convert SWAP gate to {CZ, rx, ry}.

    SWAP gate decomposition rule:
    q[0]  --x--      q[0]  --Ry(π/2)--Rx(π)--◆--Ry(π/2)--Rx(π)--◆--Ry(π/2)--Rx(π)--◆--Ry(π/2)--Rx(π)--
            |    ≡                           |                  |                  |
    q[1]  --x--      q[1]  ------------------◆--Ry(π/2)--Rx(π)--◆--Ry(π/2)--Rx(π)--◆------------------
    """

    def __init__(self) -> None:
        super().__init__()
        self.original = SwapGate.name.lower()
        self.basis = [CZGate.name.lower(), RXGate.name.lower(), RYGate.name.lower()]
        self.global_phase = pi

    def run(self, op: CircuitInstruction) -> List[CircuitInstruction]:
        rule = []
        if op.name == 'swap':
            if isinstance(op.pos, list):
                pos = op.pos[0]
            else:
                pos = op.pos
            rule.append(RYGate(pi / 2, pos[0]))
            rule.append(RXGate(pi, pos[0]))
            rule.append(CZGate(op.pos[0], op.pos[1]))
            rule.append(RYGate(pi / 2, pos[0]))
            rule.append(RXGate(pi, pos[0]))
            rule.append(RYGate(pi / 2, pos[1]))
            rule.append(RXGate(pi, pos[1]))
            rule.append(CZGate(op.pos[0], op.pos[1]))
            rule.append(RYGate(pi / 2, pos[0]))
            rule.append(RXGate(pi, pos[0]))
            rule.append(RYGate(pi / 2, pos[1]))
            rule.append(RXGate(pi, pos[1]))
            rule.append(CZGate(op.pos[0], op.pos[1]))
            rule.append(RYGate(pi / 2, pos[0]))
            rule.append(RXGate(pi, pos[0]))
        else:
            rule.append(op)
        self.rule = rule
        return rule
