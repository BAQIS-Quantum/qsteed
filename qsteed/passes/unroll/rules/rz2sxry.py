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
    RX as RXGate,
    RY as RYGate,
    RZ as RZGate,
    SX as SXGate,
    SXdg as SXdgGate,
)

from qsteed.passes.basepass import UnrollPass


class RZToSXRY(UnrollPass):
    """The RZToSXRY pass.
    convert RZ gate to {RY, SX}.

    H gate decomposition rule:
    q[0] ----RZ(theta)---- ≡ q[0] ----√X---RY(-theta)---√Xdg----
    """

    def __init__(self) -> None:
        super().__init__()
        self.original = RZGate.name.lower()
        self.basis = [RYGate.name.lower(), SXGate.name.lower(), SXdgGate.name.lower()]

    def run(self, op: CircuitInstruction) -> List[CircuitInstruction]:
        rule = []
        if op.name == 'rz':
            if isinstance(op.pos, list):
                pos = op.pos[0]
            else:
                pos = op.pos
            if isinstance(op.paras, list):
                paras = op.paras[0]
            else:
                paras = op.paras
            rule.append(SXGate(pos))
            rule.append(RYGate(-paras, pos))
            rule.append(SXdgGate(pos))
        else:
            rule.append(op)
        self.rule = rule
        return rule
