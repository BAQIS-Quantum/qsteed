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

import math

import numpy as np
from quafu import QuantumCircuit
from quafu.elements.matrices import ry_mat, rz_mat, CXMatrix, rx_mat

from qsteed.passes.decomposition.CSD_decompose import fatCSD
from qsteed.passes.decomposition.one_qubit_decompose import OneQubitDecompose
from qsteed.passes.decomposition.utils import decompose_utils as du
from qsteed.passes.decomposition.utils import matrix_utils as mu


class UnitaryDecompose(object):
    def __init__(self, array, qubits, endian: str = None, one_qubit_decompose: str = 'XZX'):
        """
        Args:
            array (np.array): arbitrary unitary
            qubits (list): example [0,1,2,3]
            endian (str): 'little-endian': U_qn ⊗ ... ⊗ U_q1 ⊗ U_q0, where q0<q1< ... <qn
                          'big-endian': U_q0 ⊗ U_q1 ⊗ ... ⊗ U_qn, where q0<q1< ... <qn
            one_qubit_decompose (str): one-qubit decomposition method: 'ZYZ', 'ZXZ', 'XYX', 'XZX'
        """
        if endian is None or endian == 'big-endian':
            # big-endian mode: U_q0 ⊗ U_q1 ⊗ ... ⊗ U_qn, where q0<q1< ... <qn
            self.endian = 'big-endian'
        elif endian == 'little-endian':
            self.endian = 'little-endian'
            # little-endian mode: U_qn ⊗ ... ⊗ U_q1 ⊗ U_q0, where q0<q1< ... <qn
            # If you want to use little-endian mode, use this command to invert qubits
            qubits = qubits[::-1]
        else:
            self.endian = 'Warning: endian can only be big-endian or little-endian'
            print('Warning: endian can only be big-endian or little-endian')
        self.array = array
        self.nqubit = len(qubits)
        self.qubits = qubits
        self.one_qubit_decompose = one_qubit_decompose

        self.Mk_table = du.genMk_table(self.nqubit)  # initialize the general M^k lookup table
        self.gates_list = []
        self.quafuQC = QuantumCircuit(self.nqubit)
        self.global_phase = 0

    def decompose(self):
        _matrix = self.array
        self._decompose_matrix(_matrix, self.qubits)
        self.quafuQC.measure(self.qubits, self.qubits)
        # print("Done decomposing")

    def _decompose_matrix(self, _matrix, qubits):
        # qubits = qubits[::-1]
        # tests the matrix is unitary
        assert mu.is_unitary(_matrix)
        num_qubit = len(qubits)
        assert int(np.log2(_matrix.shape[0])) == num_qubit

        if num_qubit == 1:
            # self.gates_list.append((_matrix, qubits, 'U'))
            one_qubit_decomposer = OneQubitDecompose(method=self.one_qubit_decompose)
            gamma, beta, alpha, global_phase = one_qubit_decomposer.run(_matrix)
            self.global_phase += global_phase
            self.one_qubit_circuit(gamma, beta, alpha, qubits)
            # if self.one_qubit_decompose == 'ZYZ':
            #     # # ZYZ decomposition for single-qubit gate
            #     # gamma, beta, alpha, global_phase = zyz_decomposition(_matrix)
            #     # gamma, beta, alpha, global_phase = one_qubit_decomposer.run(_matrix)
            #     # self.global_phase += global_phase
            #     self.gates_list.append((rz_mat(gamma), qubits, 'RZ', gamma))
            #     self.gates_list.append((ry_mat(beta), qubits, 'RY', beta))
            #     self.gates_list.append((rz_mat(alpha), qubits, 'RZ', alpha))
            #     self.quafuQC.rz(qubits[0], gamma)
            #     self.quafuQC.ry(qubits[0], beta)
            #     self.quafuQC.rz(qubits[0], alpha)

            # # # ZXZ decomposition for single-qubit gate
            # gamma, beta, alpha, global_phase = zxz_decomposition(_matrix)
            # self.global_phase += global_phase
            # self.gates_list.append((rz_mat(gamma), qubits, 'RZ', gamma))
            # self.gates_list.append((rx_mat(beta), qubits, 'RX', beta))
            # self.gates_list.append((rz_mat(alpha), qubits, 'RZ', alpha))
            # self.quafuQC.rz(qubits[0], gamma)
            # self.quafuQC.rx(qubits[0], beta)
            # self.quafuQC.rz(qubits[0], alpha)

            # # XYX decomposition for single-qubit gate
            # gamma, beta, alpha, global_phase = xyx_decomposition(_matrix)
            # self.global_phase += global_phase
            # self.gates_list.append((rx_mat(gamma), qubits, 'RX', gamma))
            # self.gates_list.append((ry_mat(beta), qubits, 'RY', beta))
            # self.gates_list.append((rx_mat(alpha), qubits, 'RX', alpha))
            # self.quafuQC.rx(qubits[0], gamma)
            # self.quafuQC.ry(qubits[0], beta)
            # self.quafuQC.rx(qubits[0], alpha)

            # # XZX decomposition for single-qubit gate
            # gamma, beta, alpha, global_phase = xzx_decomposition(_matrix)
            # self.global_phase += global_phase
            # self.gates_list.append((rx_mat(gamma), qubits, 'RX', gamma))
            # self.gates_list.append((rz_mat(beta), qubits, 'RZ', beta))
            # self.gates_list.append((rx_mat(alpha), qubits, 'RX', alpha))
            # self.quafuQC.rx(qubits[0], gamma)
            # self.quafuQC.rz(qubits[0], beta)
            # self.quafuQC.rx(qubits[0], alpha)
        else:
            U00, U01, U10, U11 = mu.split_matrix(_matrix)

            # if bottomLeftCorner(n)==0 and topRightCorner(n)==0:
            if mu.is_zero(U01) and mu.is_zero(U10):
                if mu.is_approx(U00, U11):
                    # print('Optimization: Unitaries are equal, skip one step in the recursion for unitaries of size')
                    self._decompose_matrix(U00, qubits[1:])
                else:
                    # print("Optimization: q2 is zero, only demultiplexing will be performed.")
                    V, D, W = du.demultiplexing(U00, U11)

                    self._decompose_matrix(W, qubits[1:])
                    self.multi_controlled_z(D, qubits[1:], qubits[0])
                    self._decompose_matrix(V, qubits[1:])
            # Check to see if it the kronecker product of a bigger matrix and the identity matrix.
            # By checking if the first row is equal to the second row one over, and if the last two rows are equal
            # Which means the last qubit is not affected by this gate
            elif mu.is_kron_with_id2(_matrix):
                # print("The last qubits not affect.")
                nsize = len(_matrix)
                self._decompose_matrix(_matrix[0:nsize:2, 0:nsize:2], qubits[:-1])

            else:
                # print("CSD decomposition.")
                L0, L1, R0, R1, c, s = fatCSD(_matrix)
                V, D, W = du.demultiplexing(R0, R1)

                self._decompose_matrix(W, qubits[1:])
                self.multi_controlled_z(D, qubits[1:], qubits[0])
                self._decompose_matrix(V, qubits[1:])

                self.multi_controlled_y(s, qubits[1:], qubits[0])

                V, D, W = du.demultiplexing(L0, L1)
                self._decompose_matrix(W, qubits[1:])
                self.multi_controlled_z(D, qubits[1:], qubits[0])
                self._decompose_matrix(V, qubits[1:])

    def multi_controlled_z(self, D, qubits, target_qubit):
        assert len(qubits) == int(math.log(D.shape[0], 2))
        num_qubit = len(qubits)
        # print("The size of D matrix {}".format(len(qubits)))
        # print(qubits)

        # alphas = -2*1j*np.log(np.diag(D))
        alphas = 2 * 1j * np.log(np.diag(D))
        Mk = self.Mk_table[num_qubit - 1]
        thetas = np.linalg.solve(Mk, alphas)
        # print('thetas',thetas)
        thetas = thetas.real
        assert len(thetas) == 2 ** num_qubit

        index = du.get_multi_control_index(num_qubit)
        assert len(index) == len(thetas)

        for i in range(len(index)):
            control_qubit = qubits[index[i]]
            self.gates_list.append((rz_mat(thetas[i]), [target_qubit], 'RZ', thetas[i]))
            self.quafuQC.rz(target_qubit, thetas[i])
            self.gates_list.append((CXMatrix, [control_qubit, target_qubit], 'CX'))
            self.quafuQC.cnot(control_qubit, target_qubit)

    def multi_controlled_y(self, ss, qubits, target_qubit):
        assert len(qubits) == int(math.log(ss.shape[0], 2))
        num_qubit = len(qubits)

        alphas = -2 * np.arcsin(np.diag(ss))
        Mk = self.Mk_table[num_qubit - 1]
        thetas = np.linalg.solve(Mk, alphas)
        # print(thetas)
        thetas = thetas.real
        assert len(thetas) == 2 ** num_qubit

        index = du.get_multi_control_index(num_qubit)
        assert len(index) == len(thetas)

        for i in range(len(index)):
            control_qubit = qubits[index[i]]
            self.gates_list.append((ry_mat(thetas[i]), [target_qubit], 'RY', thetas[i]))
            self.quafuQC.ry(target_qubit, thetas[i])
            self.gates_list.append((CXMatrix, [control_qubit, target_qubit], 'CX'))
            self.quafuQC.cnot(control_qubit, target_qubit)

    def one_qubit_circuit(self, gamma, beta, alpha, qubits):
        if self.one_qubit_decompose == 'ZYZ':
            # ZYZ decomposition for single-qubit gate
            self.gates_list.append((rz_mat(gamma), qubits, 'RZ', gamma))
            self.gates_list.append((ry_mat(beta), qubits, 'RY', beta))
            self.gates_list.append((rz_mat(alpha), qubits, 'RZ', alpha))
            self.quafuQC.rz(qubits[0], gamma)
            self.quafuQC.ry(qubits[0], beta)
            self.quafuQC.rz(qubits[0], alpha)
        elif self.one_qubit_decompose == 'ZXZ':
            # ZXZ decomposition for single-qubit gate
            self.gates_list.append((rz_mat(gamma), qubits, 'RZ', gamma))
            self.gates_list.append((rx_mat(beta), qubits, 'RX', beta))
            self.gates_list.append((rz_mat(alpha), qubits, 'RZ', alpha))
            self.quafuQC.rz(qubits[0], gamma)
            self.quafuQC.rx(qubits[0], beta)
            self.quafuQC.rz(qubits[0], alpha)
        elif self.one_qubit_decompose == 'XYX':
            # XYX decomposition for single-qubit gate
            self.gates_list.append((rx_mat(gamma), qubits, 'RX', gamma))
            self.gates_list.append((ry_mat(beta), qubits, 'RY', beta))
            self.gates_list.append((rx_mat(alpha), qubits, 'RX', alpha))
            self.quafuQC.rx(qubits[0], gamma)
            self.quafuQC.ry(qubits[0], beta)
            self.quafuQC.rx(qubits[0], alpha)
        elif self.one_qubit_decompose == 'XZX':
            # XZX decomposition for single-qubit gate
            self.gates_list.append((rx_mat(gamma), qubits, 'RX', gamma))
            self.gates_list.append((rz_mat(beta), qubits, 'RZ', beta))
            self.gates_list.append((rx_mat(alpha), qubits, 'RX', alpha))
            self.quafuQC.rx(qubits[0], gamma)
            self.quafuQC.rz(qubits[0], beta)
            self.quafuQC.rx(qubits[0], alpha)
        else:
            raise ValueError("The selected decomposition method can only be: ZYZ, ZXZ, XYX, XZX")
