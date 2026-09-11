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
from quafu.elements.element_gates import RYGate, RZGate, SXGate, SXdgGate

from qsteed.passes.basepass import UnrollPass


class RZToSXRY(UnrollPass):
    """Convert an RZ gate to SXdg, RY(-theta), SX."""

    def __init__(self) -> None:
        super().__init__()
        self.original = RZGate(0, 0).name.lower()
        self.basis = [RYGate(0, 0).name.lower(), SXGate.name.lower(), SXdgGate.name.lower()]

    def run(self, op: Instruction) -> List[Instruction]:
        rule = []
        if isinstance(op, RZGate):
            pos = op.pos[0] if isinstance(op.pos, list) else op.pos
            paras = op.paras[0] if isinstance(op.paras, list) else op.paras
            rule.append(SXdgGate(pos))
            rule.append(RYGate(pos, -paras))
            rule.append(SXGate(pos))
        else:
            rule.append(op)
        self.rule = rule
        return rule
