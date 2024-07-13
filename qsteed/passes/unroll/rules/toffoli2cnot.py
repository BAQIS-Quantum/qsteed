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
from quafu.elements.element_gates import ToffoliGate, CXGate, HGate, TGate, TdgGate

from qsteed.passes.basepass import UnrollPass


class ToffoliToCNOT(UnrollPass):
    """The ToffoliToCNOT pass.
    convert Toffoli(CCX) gate to {CNOT, H, Tdg, T}.

    Toffoli(CCX) gate decomposition rule:
    q[0]  --*--      q[0]  -------------------*---------------------*----*-----T-----*--
            |                                 |                     |    |           |
    q[1]  --*--  ≡   q[1]  -------*-----------|---------*-----T-----|----+----Tdg----+--
            |                     |           |         |           |
    q[2]  --X--      q[2]  --H----+----Tdg----+----T----+----Tdg----+----T-----H--------
    """

    def __init__(self) -> None:
        super().__init__()
        self.original = ToffoliGate(0, 1, 2).name.lower()
        self.basis = [CXGate.name.lower(), HGate.name.lower(), TGate.name.lower(), TdgGate.name.lower()]

    def run(self, op: Instruction) -> List[Instruction]:
        rule = []
        if isinstance(op, ToffoliGate):
            rule.append(HGate(op.pos[2]))
            rule.append(CXGate(op.pos[1], op.pos[2]))
            rule.append(TdgGate(op.pos[2]))
            rule.append(CXGate(op.pos[0], op.pos[2]))
            rule.append(TGate(op.pos[2]))
            rule.append(CXGate(op.pos[1], op.pos[2]))
            rule.append(TGate(op.pos[1]))
            rule.append(TdgGate(op.pos[2]))
            rule.append(CXGate(op.pos[0], op.pos[2]))
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(TGate(op.pos[2]))
            rule.append(TGate(op.pos[0]))
            rule.append(TdgGate(op.pos[1]))
            rule.append(HGate(op.pos[2]))
            rule.append(CXGate(op.pos[0], op.pos[1]))
        else:
            rule.append(op)
        self.rule = rule
        return rule


class ToffoliToCNOT8(UnrollPass):
    """The ToffoliToCNOT pass.
    convert Toffoli(CCX) gate to {CNOT, H, Tdg, T}.

    Toffoli(CCX) gate decomposition rule:
    q[0]  --*--      q[0]  -------T----*---------*---------*----------------*--------------
            |                          |         |         |                |           
    q[1]  --*--  ≡   q[1]  -------T----+----*----+----*----+----Tdg----*----+------*-------
            |                               |         |                |           |
    q[2]  --X--      q[2]  --H----T---------+----T----+---------Tdg----+----Tdg----+----H--
    """

    def __init__(self) -> None:
        super().__init__()
        self.original = ToffoliGate(0, 1, 2).name.lower()
        self.basis = [CXGate.name.lower(), HGate.name.lower(), TGate.name.lower(), TdgGate.name.lower()]

    def run(self, op: Instruction) -> List[Instruction]:
        rule = []
        if isinstance(op, ToffoliGate):
            rule.append(HGate(op.pos[2]))
            rule.append(TGate(op.pos[0]))
            rule.append(TGate(op.pos[1]))
            rule.append(TGate(op.pos[2]))
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(CXGate(op.pos[1], op.pos[2]))
            rule.append(TGate(op.pos[2]))
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(CXGate(op.pos[1], op.pos[2]))
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(TdgGate(op.pos[1]))
            rule.append(TdgGate(op.pos[2]))
            rule.append(CXGate(op.pos[1], op.pos[2]))
            rule.append(CXGate(op.pos[0], op.pos[1]))
            rule.append(TdgGate(op.pos[2]))
            rule.append(CXGate(op.pos[1], op.pos[2]))
            rule.append(HGate(op.pos[2]))
        else:
            rule.append(op)
        self.rule = rule
        return rule
