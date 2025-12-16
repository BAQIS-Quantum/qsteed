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
    X as XGate,
    RX as RXGate,
)

from qsteed.passes.basepass import UnrollPass


class XToRX(UnrollPass):
    """The XToRX pass.
    convert X gate to {RX}.

    X gate decomposition rule:
    q[0] ----X---- ≡   U3(π,0,π)  ≡  q[0] ----RX(π)----  * global_phase(π/2)
    """

    def __init__(self) -> None:
        super().__init__()
        self.original = XGate.name.lower()
        self.basis = [RXGate.name.lower()]
        self.global_phase = pi / 2
        # qc = QuantumCircuit(1)
        # qc.rx(0, pi)
        # self.circuit = qc

    def run(self, op: CircuitInstruction) -> List[CircuitInstruction]:
        rule = []
        if op.name == 'x':
            if isinstance(op.pos, list):
                pos = op.pos[0]
            else:
                pos = op.pos
            rule.append(RXGate(pi, pos))
        else:
            rule.append(op)
        self.rule = rule
        return rule
