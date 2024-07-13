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

from quafu.elements import Instruction
from quafu.elements.element_gates import CZGate, CXGate, HGate

from qsteed.passes.basepass import UnrollPass


class CNOTToCZ(UnrollPass):
    """The CNOTToCZ pass.

    convert CNOT gate to {H, CZ}.

    CNOT gate decomposition rule:
    q[0]  --*--     q[0] -------*-------
            |    ≡              |
    q[1]  --+--     q[1] --H----Z----H--
    """

    def __init__(self) -> None:
        super().__init__()
        self.original = CXGate.name.lower()
        self.basis = [CZGate.name.lower(), HGate.name.lower()]

    def run(self, op: Instruction) -> List[Instruction]:
        rule = []
        if isinstance(op, CXGate):
            rule.append(HGate(op.pos[1]))
            rule.append(CZGate(op.pos[0], op.pos[1]))
            rule.append(HGate(op.pos[1]))
        else:
            rule.append(op)
        self.rule = rule
        return rule
