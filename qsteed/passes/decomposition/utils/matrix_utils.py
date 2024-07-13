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

import numpy as np
from quafu.elements.matrices import IdMatrix


def split_matrix(matrix: np.ndarray):
    """
    Evenly split a matrix into 4 sub-matrices.
    """
    top, bottom = np.vsplit(matrix, 2)
    t_left, t_right = np.hsplit(top, 2)
    b_left, b_right = np.hsplit(bottom, 2)
    return t_left, t_right, b_left, b_right


def combine_matrix(topLeft, topRight, bottomLeft, bottomRight):
    top = np.hstack((topLeft, topRight))
    bottom = np.hstack((bottomLeft, bottomRight))
    tot = np.vstack((top, bottom))
    return tot


def stack_matrices(t_left, t_right, b_left, b_right):
    """
    Stack 4 sub-matrices into a matrix.
    """
    top = np.hstack((t_left, t_right))
    bottom = np.hstack((b_left, b_right))
    mat = np.vstack((top, bottom))
    return mat


def is_zero(a):
    return not np.any(np.absolute(a) > 1e-8)


def is_approx(a, b, thres=1e-6):
    # TODO: seems there are some very small elements that cannot be compared correctly
    # if not np.allclose(a, b, rtol=thres, atol=thres):
    #    print(np.sum(a-b))
    return np.allclose(a, b, rtol=thres, atol=thres)


def is_unitary(matrix):
    mat_dg = np.conjugate(matrix).T
    id_mat = np.eye(matrix.shape[0])
    return is_approx(mat_dg @ matrix, id_mat) and is_approx(matrix @ mat_dg, id_mat)


def is_hermitian(matrix):
    tmp = np.conjuate(matrix).T
    return is_approx(tmp, matrix)


def is_diagonal(matrix: np.ndarray):
    diag = np.diag(matrix)
    diag_mat = np.diag(diag)
    return is_approx(matrix, diag_mat)


def is_kron_with_id2(matrix):
    """
    Check if the matrix is a Kronecker product of a matrix and identity matrix.
    """
    nsize = matrix.shape[0]
    a_cond = is_zero(matrix[0:nsize:2, 1:nsize:2])
    b_cond = is_zero(matrix[1:nsize:2, 0:nsize:2])
    c_cond = is_approx(matrix[0, :-1], matrix[1, 1:])
    d_cond = is_approx(matrix[-2, :-1], matrix[-1, 1:])

    return a_cond and b_cond and c_cond and d_cond


def multi_kron(op1, op2, ind1, ind2, nspin):
    tmp = 1
    for i in range(nspin):
        if i == ind1:
            tmp = np.kron(tmp, op1)
        elif i == ind2:
            tmp = np.kron(tmp, op2)
        else:
            tmp = np.kron(tmp, IdMatrix)
    return tmp


def Int2Bas(n, b, nbit):
    if (n == 0): return [0] * nbit
    x = []
    while (n):
        x.append(int(n % b))
        n //= b
    return [0] * (nbit - len(x)) + x[::-1]


def Bas2Int(x, b):
    """
    b: base
    -type: int
    x: the b base bitstring numbers
    -type: list

    example: x = [1, 0, 1], b = 2, return 5
    """
    nbit = len(x)
    z = [b ** (nbit - 1 - i) for i in range(nbit)]
    return np.dot(z, x)


def general_CNOT(nqubit, control_q, target_q):
    CNOT_matrix = np.zeros((2 ** nqubit, 2 ** nqubit))
    for i in range(2 ** nqubit):
        i_bin = Int2Bas(i, 2, nqubit)
        # print("i bin is {}:".format(i_bin))
        if i_bin[control_q] == 0:
            CNOT_matrix[i, i] = 1
        else:
            j_bin = i_bin.copy()
            j_bin[target_q] = (j_bin[target_q] + 1) % 2
            # print("j bin is {}:".format(j_bin))
            j = Bas2Int(j_bin, 2)
            CNOT_matrix[i, j] = 1

    return CNOT_matrix


def general_kron(op, ind, nqubit):
    tmp = 1
    for i in range(nqubit):
        if i == ind:
            tmp = np.kron(tmp, op)
        else:
            tmp = np.kron(tmp, IdMatrix)
    return tmp


def get_global_phase(unitary):
    """ Get the global phase of arbitrary unitary, and get the special unitary.

    Args:
        unitary (np.array): arbitrary unitary
    Returns:
        global_phase: the global phase of arbitrary unitary
        special_unitary (np.array)
    """
    coefficient = np.linalg.det(unitary) ** (-0.5)
    global_phase = -cmath.phase(coefficient)
    special_unitary = coefficient * unitary
    return global_phase, special_unitary


def matrix_distance_squared(u1, u2):
    """Used to compare the distance of two matrices. The global phase is ignored.

    Args:
        u1 (np.array): A unitary matrix in the form of a numpy ndarray.
        u2 (np.array): Another unitary matrix.

    Returns:
        Float : A single value between 0 and 1. A value close to 0 indicates that u1 and u2 are the same unitary.
    """
    return np.abs(1 - np.abs(np.sum(np.multiply(u1, np.conj(u2)))) / u1.shape[0])
