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

import numpy as np
from quafu.elements import Instruction
from quafu.elements.element_gates import ControlledU, CXGate, RZGate, RYGate
from scipy.linalg import sqrtm

from qsteed.passes.basepass import UnrollPass
from qsteed.passes.decomposition.ZYZ_decompose import zyz_decomposition


class ControlledUToCNOT(UnrollPass):
    '''The ControllUToCNOT
    q[0]  --*--      q[0]  ---------------------------------+----*-----+----*----+-----*----+----*--------
            |                                               |    |     |    |    |     |    |    |
    q[1]  --*--  ≡   q[1]  -------------+----*----+----*----*----|-----|----|----*-----|----|----|--------
            |                           |    |    |    |         |     |    |          |    |    |
    q[2]  --*--      q[2]  -------*-----*----|----*----|---------|-----*----|----------|----*----|--------
            |                     |          |         |         |          |          |         |  
    q[3]  --X--      q[3]  -------V----------V---------V---------V----------V----------V---------V------- 


    q[0]  --*--      q[0]   --------------*--------------------   
            |    ≡                        |
    q[1]  --V--      q[1]   --RZ(gamma)---+------RY(beta)------


    q[0]  --*--     q[0]    -------*---------*----*---------
            |                      |         |    |
    q[1]  --*-- ≡   q[1]    --*----+----*----+----|---------
            |                 |         |         |
    q[2]  --V--     q[2]    --V---------V*--------V---------
        '''

    def __init__(self) -> None:
        super().__init__()
        self.original = ControlledU.name.lower()
        self.basis = [CXGate.name.lower(), RZGate.name.lower(), RYGate.name.lower()]

    def run(self, op: Instruction) -> List[Instruction]:
        rule = []
        if isinstance(op, ControlledU):
            control_bits = op.ctrls
            target_bit = op.targs[0]
            U = op.targ_gate
            if len(control_bits) == 1:
                gamma, beta, alpha, global_phase = zyz_decomposition(U)
                rule.append(RZGate(target_bit, gamma))
                rule.append(CXGate(control_bits[0], target_bit))
                rule.append(RYGate(target_bit, beta))
            elif len(control_bits) == 2:
                V = self.decompose_U(U)
                gamma1, beta1, alpha1, global_phase1 = zyz_decomposition(V)
                gamma2, beta2, alpha2, global_phase2 = zyz_decomposition(np.conj(V).T)
                rule.append(RZGate(target_bit, gamma1))
                rule.append(CXGate(control_bits[1], target_bit))
                rule.append(RYGate(target_bit, beta1))
                rule.append(CXGate(control_bits[0], control_bits[1]))
                rule.append(RZGate(target_bit, gamma2))
                rule.append(CXGate(control_bits[1], target_bit))
                rule.append(RYGate(target_bit, beta2))
                rule.append(CXGate(control_bits[0], control_bits[1]))
                rule.append(RZGate(target_bit, gamma1))
                rule.append(CXGate(control_bits[0], target_bit))
                rule.append(RYGate(target_bit, beta1))
            else:
                graycode = self.build_gray_code(len(control_bits))
                last_code = graycode[0, :]
                decompose_times = len(control_bits - 1)
                V = U
                for _ in decompose_times:
                    V = self.decompose_U(U)
                gamma1, beta1, alpha1, global_phase1 = zyz_decomposition(V)

                gamma2, beta2, alpha2, global_phase2 = zyz_decomposition(np.conj(V).T)
                for i in range(1, len(graycode)):
                    code = graycode[i, :]
                    ones_bits = np.where(code == 1)[0]
                    set_idx = ones_bits[0]
                    diff_idx = np.where(code != last_code)[0][0]
                    if diff_idx != set_idx:
                        rule.append(CXGate(control_bits[diff_idx], control_bits[set_idx]))
                    elif len(ones_bits) >= 2:
                        next_idx = ones_bits[1]
                        rule.append(CXGate(control_bits[next_idx], control_bits[set_idx]))
                    if np.sum(code) % 2 == 0:
                        rule.append(RZGate(target_bit, gamma2))
                        rule.append(CXGate(control_bits[set_idx], target_bit))
                        rule.append(RYGate(target_bit, beta2))
                    else:
                        rule.append(RZGate(target_bit, gamma1))
                        rule.append(CXGate(control_bits[set_idx], target_bit))
                        rule.append(RYGate(target_bit, beta1))
                    last_code = code

        else:
            rule.append(op)
        self.rule = rule
        return rule

    # build 2^n gray code based on the control bits
    def build_gray_code(self, n):
        code = np.full((2 ** n, n), 0)
        block = np.repeat([0, 1], 2 ** (n - 1))
        code[:, 0] = block
        for i in range(1, n):
            block = np.repeat([0, 1, 1, 0], 2 ** (n - 1 - i))
            print(block)
            code[:, i] = np.tile(block, 2 ** (i - 1))
        return code

    # decompose U to V, where U = V^2
    def decompose_U(self, U):
        V = sqrtm(U)
        return V
