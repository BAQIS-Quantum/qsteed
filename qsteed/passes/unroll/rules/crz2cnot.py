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

from qsteed.qsteedcpp import (
    CircuitInstruction,
    CRZ as CRZGate,
    CX as CXGate,
    RZ as RZGate,
)

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
        self.original = CRZGate.name.lower()
        self.basis = [CXGate.name.lower(), RZGate.name.lower()]

    def run(self, op: CircuitInstruction) -> List[CircuitInstruction]:
        rule = []
        if op.name == 'crz':
            theta = op.paras[0]  # Assuming CRZGate takes one parameter θ

            rule.append(RZGate(theta / 2, op.pos[1]))
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(RZGate(-theta / 2, op.pos[1]))
            rule.append(CXGate(op.pos[0], op.pos[1]))
        else:
            rule.append(op)
        self.rule = rule
        return rule
