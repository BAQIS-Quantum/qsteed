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
    CP as CPGate, 
    CX as CXGate, 
    H as HGate
)

from qsteed.passes.basepass import UnrollPass


class CNOTToCP(UnrollPass):
    """The CNOTToCP pass.

    convert CNOT gate to {RX, RY, RZ, CP}.

    CNOT gate decomposition rule:
    q[0]  --*--     q[0] --------*---------
            |    ≡               |
    q[1]  --+--     q[1] --H----P(π)----H--
    """

    def __init__(self) -> None:
        super().__init__()
        self.original = CXGate.name.lower()
        self.basis = [HGate.name.lower(), CPGate.name.lower()]
        # self.basis = [RYGate.name.lower(), RZGate.name.lower(), CPGate.name.lower()]
        # self.global_phase = pi

    def run(self, op: CircuitInstruction) -> List[CircuitInstruction]:
        rule = []
        if op.name in ['cx', 'cnot']:
            # rule.append(RYGate(-pi / 2, op.pos[1]))
            # rule.append(RZGate(-pi, op.pos[1]))
            # rule.append(CPGate(pi, op.pos[0], op.pos[1]))
            # rule.append(RYGate(-pi / 2, op.pos[1]))
            # rule.append(RZGate(-pi, op.pos[1]))

            rule.append(HGate(op.pos[1]))
            rule.append(CPGate(pi, op.pos[0], op.pos[1]))
            rule.append(HGate(op.pos[1]))
        else:
            rule.append(op)
        self.rule = rule
        return rule
