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
from quafu.elements.element_gates import WGate, RYGate, RZGate

from qsteed.passes.basepass import UnrollPass


class WToRYRZ(UnrollPass):
    """The WToRYRZ pass.
    convert W gate to {RY, RZ}.

    W gate decomposition rule:
    q[0] ----W---- ≡ U3(π,-π/4,π/4) * global_phase(π/2)
                   ≡ q[0] ----RZ(π/2)---RY(π)---- * global_phase(π/2)
    """

    def __init__(self) -> None:
        super().__init__()
        self.original = WGate.name.lower()
        self.basis = [RYGate(0, 0).name.lower(), RZGate(0, 0).name.lower()]
        self.global_phase = pi / 2
        # qc = QuantumCircuit(1)
        # qc.rz(0, pi / 2)
        # qc.ry(0, pi)
        # self.circuit = qc

    def run(self, op: Instruction) -> List[Instruction]:
        rule = []
        if isinstance(op, WGate):
            if isinstance(op.pos, list):
                pos = op.pos[0]
            else:
                pos = op.pos
            rule.append(RZGate(pos, pi / 2))
            rule.append(RYGate(pos, pi))
        else:
            rule.append(op)
        self.rule = rule
        return rule
