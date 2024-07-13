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
from quafu.elements.element_gates import U3Gate
from quafu.elements.element_gates.rotation import RXGate, RYGate, RZGate

from qsteed.passes.basepass import UnrollPass
from qsteed.passes.decomposition.unitary_decompose import UnitaryDecompose


class U3Decompose(UnrollPass):
    """The U3Decompose pass.
    convert U3 gate to {Rx, RY, RZ}.

    U3 gate decomposition rule:
    U3 decomposition method: ZYZ or ZXZ or XYX or XZX
    """

    def __init__(self, one_qubit_decompose: str = 'ZXZ') -> None:
        super().__init__()
        self.original = U3Gate(0, 0, 0, 0).name.lower()
        self.basis = [RXGate(0, 0).name.lower(), RYGate(0, 0).name.lower(), RZGate(0, 0).name.lower()]
        self.one_qubit_decompose = one_qubit_decompose

    def run(self, op: Instruction) -> List[Instruction]:
        rule = []
        if isinstance(op, U3Gate):
            if isinstance(op.pos, list):
                pos = op.pos[0]
            else:
                pos = op.pos

            uc = UnitaryDecompose(op.matrix, [0], one_qubit_decompose=self.one_qubit_decompose)
            uc.decompose()
            gates = uc.quafuQC.gates

            for g in gates:
                g.pos = [pos]
                rule.append(g)
        else:
            rule.append(op)
        self.rule = rule
        return rule
