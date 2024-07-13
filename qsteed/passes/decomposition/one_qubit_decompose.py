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

import cmath
import math

import numpy as np

from qsteed.passes.decomposition.utils.matrix_utils import get_global_phase


class OneQubitDecompose:
    def __init__(self, method: str = 'ZYZ'):
        self.method = method

    def zyz_decomposition(self, unitary):
        """ZYZ decomposition of arbitrary single-qubit gate (unitary).
        unitary = e^{i*global_phase} @ Rz(gamma) @ Ry(beta) @ Rz(alpha)

        Args:
            unitary (np.array): arbitrary unitary
        Returns:
            global_phase: the global phase of arbitrary unitary
            gamma, beta, alpha: rotation angle
        """
        if unitary.shape[0] == 2:
            global_phase, special_unitary = get_global_phase(unitary)
            beta = 2 * math.atan2(abs(special_unitary[1, 0]), abs(special_unitary[0, 0]))
            t1 = cmath.phase(special_unitary[1, 1])
            t2 = cmath.phase(special_unitary[1, 0])
            alpha = t1 + t2
            gamma = t1 - t2
        else:
            raise Exception("ZYZ decomposition only applies to single-qubit gate.")
        return gamma, beta, alpha, global_phase

    def zxz_decomposition(self, unitary):
        """ZXZ decomposition of arbitrary single-qubit gate (unitary).
        unitary = e^{i*global_phase} @ Rz(gamma) @ Rx(beta) @ Rz(alpha)

        Rx(ϴ) = Rz(-π/2) @ Ry(ϴ) @ Rz(π/2)
         ====>
        Rz(gamma) @ Rx(beta) @ Rz(alpha) = Rz(gamma-π/2) @ Ry(beta) @ Rz(alpha+π/2)

        Args:
            unitary (np.array): arbitrary unitary
        Returns:
            global_phase: the global phase of arbitrary unitary
            gamma, beta, alpha: rotation angle
        """
        if unitary.shape[0] == 2:
            gamma, beta, alpha, global_phase = self.zyz_decomposition(unitary)
        else:
            raise Exception("ZXZ decomposition only applies to single-qubit gate.")
        return gamma - np.pi / 2, beta, alpha + np.pi / 2, global_phase

    def xyx_decomposition(self, unitary):
        """XYX decomposition of arbitrary single-qubit gate (unitary).
        unitary = e^{i*global_phase} @ Rx(gamma) @ Ry(beta) @ Rx(alpha)

        H @ Rx(ϴ) @ H = Rz(ϴ)
        H @ Rz(ϴ) @ H = Rx(ϴ)
        H @ Ry(ϴ) @ H = Ry(-ϴ)
        H @ H = I
         ====>
        Rx(gamma) @ Ry(beta) @ Rx(alpha) = H @ Rz(gamma) @ Ry(-beta) @ Rz(alpha) @ H

        Args:
            unitary (np.array): arbitrary unitary
        Returns:
            global_phase: the global phase of arbitrary unitary
            gamma, beta, alpha: rotation angle
        """
        if unitary.shape[0] == 2:
            hadamard = np.array([[1, 1], [1, -1]]) / np.sqrt(2)
            zyz_form = hadamard @ unitary @ hadamard
            gamma, beta, alpha, global_phase = self.zyz_decomposition(zyz_form)
        else:
            raise Exception("XYX decomposition only applies to single-qubit gate.")
        return gamma, -beta, alpha, global_phase

    def xzx_decomposition(self, unitary):
        """XZX decomposition of arbitrary single-qubit gate (unitary).
        unitary = e^{i*global_phase} @ Rx(gamma) @ Rz(beta) @ Rx(alpha)

        H @ Rx(ϴ) @ H = Rz(ϴ)
        H @ Rz(ϴ) @ H = Rx(ϴ)
        H @ H = I
         ====>
        H @ Rx(gamma) @ Rz(beta) @ Rx(alpha) @ H = Rz(gamma) @ Rx(beta) @ Rz(alpha)

        Args:
            unitary (np.array): arbitrary unitary
        Returns:
            global_phase: the global phase of arbitrary unitary
            gamma, beta, alpha: rotation angle
        """
        if unitary.shape[0] == 2:
            hadamard = np.array([[1, 1], [1, -1]]) / np.sqrt(2)
            zxz_form = hadamard @ unitary @ hadamard
            gamma, beta, alpha, global_phase = self.zxz_decomposition(zxz_form)
        else:
            raise Exception("XZX decomposition only applies to single-qubit gate.")
        return gamma, beta, alpha, global_phase

    def run(self, unitary):
        if self.method == 'ZYZ':
            return self.zyz_decomposition(unitary)
        elif self.method == 'ZXZ':
            return self.zxz_decomposition(unitary)
        elif self.method == 'XYX':
            return self.xyx_decomposition(unitary)
        elif self.method == 'XZX':
            return self.xzx_decomposition(unitary)
        else:
            raise ValueError("The selected decomposition method can only be: ZYZ, ZXZ, XYX, XZX")
